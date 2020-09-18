#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}



// comment in Arduino IDE
#include <Arduino.h>

// Enable debug prints to serial monitor
#define MY_DEBUG
//#define DEBUG_STATS 1000

#define MY_GATEWAY_SERIAL

#include <MySensors.h>
#include <Bounce2.h>
//#include "PCF8574.h"
//#include "Adafruit_MCP23017.h"

#if defined(PCF8574_H) || defined(_Adafruit_MCP23017_H_)
  #define USE_EXPANDER
  #include <Wire.h>    // Required for I2C communication
  uint8_t expanderAddresses[] = {0x20};
  const int numberOfExpanders = sizeof(expanderAddresses);
  #if defined(PCF8574_H)
    PCF8574 expander[numberOfExpanders];
  #elif defined(_Adafruit_MCP23017_H_)
    Adafruit_MCP23017 expander[numberOfExpanders];
  #endif
  #define E(expanderNo, ExpanderPin) (((expanderNo+1)<<8) | (ExpanderPin))
#endif

#define MULTI_RELAY_VERSION 10
#define RELAY_STATE_STORAGE 1

const uint8_t RELAY_TRIGGER_LOW  = 0;
const uint8_t RELAY_TRIGGER_HIGH = 1;
const uint8_t RELAY_STARTUP_ON   = 2;
const uint8_t RELAY_STARTUP_OFF  = 4;
const uint8_t RELAY_IMPULSE      = 8;
const uint8_t RELAY_STARTUP_MASK = RELAY_STARTUP_ON | RELAY_STARTUP_OFF;

enum ButtonType {
  MONO_STABLE = 0,
  BI_STABLE = 1,
  DING_DONG = 2, // HIGH state immediatly after push, LOW state after release
  REED_SWITCH = 3 // magnetic sensor for door or window, LOW - closed, HIGH - opened
};

enum ButtonState {
  BTN_STATE_INITIAL,
  BTN_STATE_1ST_PRESS,
  BTN_STATE_1ST_RELEASE,
  BTN_STATE_2ND_PRESS,
  BTN_STATE_RELEASE_WAIT,
  BTN_STATE_1ST_CHANGE_BI
};

enum ButtonEvent {
  BUTTON_NO_EVENT = 0,
  BUTTON_CLICK = 0x01,
  BUTTON_DOUBLE_CLICK = 0x02,
  BUTTON_LONG_PRESS = 0x04,
  BUTTON_PRESSED = 0x10,
  BUTTON_CHANGED = 0x20,
  BUTTON_ACTION_MASK = 0x0f
};

typedef struct {
  int sensorId;
  int relayPin;
  uint8_t relayOptions;
  const char * relayDescription;
} RelayDef;

typedef struct {
  int buttonPin;
  ButtonType buttonType;
  int clickRelayId;
  int longClickRelayId;
  int doubleClickRelayId;
  const char * buttonDescription;
} ButtonDef;

// Configuration in separate file
#include "config.h"

const int numberOfRelays = sizeof(myRelays) / sizeof(RelayDef);
const int numberOfButtons = sizeof(myButtons) / sizeof(ButtonDef);

typedef struct {
  int clickRelayNum;
  int longClickRelayNum;
  int doubleClickRelayNum;
  int buttonState;
  unsigned long startStateMillis;
} ButtonToRelayIndexDef;

ButtonToRelayIndexDef buttonToRelayIndex[numberOfButtons];

uint8_t myRelayState[numberOfRelays]; // 0 - OFF, 1 - ON
unsigned long myRelayImpulseStart[numberOfRelays];
int impulsePending = 0;

#ifdef DEBUG_STATS
  bool debugStatsOn = false;
  int loopCounter = 0;
  unsigned long loopInterval = 0;
  unsigned long loopCumulativeMillis = 0;
#endif

// MySensors - Sending Data
// To send data you have to create a MyMessage container to hold the information.
MyMessage myMessage;

Bounce myPhysicalButton[numberOfButtons];

//Function Declaration
uint8_t loadRelayState(int relayNum, uint8_t forceEeprom = 0);
void saveRelayState(int relayNum, uint8_t state, uint8_t useEeprom);
void saveRelayState(int relayNum, uint8_t state);
void changeRelayState(int relayNum, uint8_t relayState);
int getButtonAction(int buttonNum);


// MySensors - This will execute before MySensors starts up
void before() {
  Serial.begin(115200);

  // TODO: validate config
  
  #ifdef USE_EXPANDER
    /* Start I2C bus and PCF8574 instance */
    for(int i = 0; i < numberOfExpanders; i++) {
      expander[i].begin(expanderAddresses[i]);
    }
  #endif

  // if version has changed, reset state of all relays
  int versionChangeResetState = (MULTI_RELAY_VERSION == loadState(0) ) ? 0 : 1;
  
  // initialize relays
  for (int i = 0; i < numberOfRelays; i++) {
    // Then set relay pins in output mode
    #ifdef USE_EXPANDER
      if ( myRelays[i].relayPin & 0xff00 ) {
        // EXPANDER
        int expanderNo = (myRelays[i].relayPin >> 8) - 1;
        int expanderPin = myRelays[i].relayPin & 0xff;
        expander[expanderNo].pinMode(expanderPin, OUTPUT);
      } else {
    #endif
        pinMode(myRelays[i].relayPin, OUTPUT);
    #ifdef USE_EXPANDER
      }
    #endif
    
    uint8_t isTurnedOn = 0;
    
    if (myRelays[i].relayOptions & RELAY_STARTUP_ON) {
      isTurnedOn = 1;
    } else if (myRelays[i].relayOptions & RELAY_STARTUP_OFF) {
    } else {
      // Set relay to last known state (using eeprom storage)
      isTurnedOn = loadRelayState(i, 1); // 1 - true, 0 - false
      if (versionChangeResetState && isTurnedOn) {
        saveRelayState(i, 0, 1);
        isTurnedOn = 0;
      }
    }

    changeRelayState(i, isTurnedOn);
    myRelayState[i] = isTurnedOn;
    myRelayImpulseStart[i] = 0;
  }
  if (versionChangeResetState) {
    // version has changed, so store new version in eeporom
    saveState(0, MULTI_RELAY_VERSION);
  }
} // before()

// executed AFTER mysensors has been initialised
void setup() {
  // Send state to MySensor Gateway
  myMessage.setType(V_STATUS);
  for(int i = 0; i < numberOfRelays; i++) {
 
    uint8_t relayState;
    if (myRelays[i].relayOptions & RELAY_STARTUP_ON) {
      relayState = 1;
    } else if (myRelays[i].relayOptions & RELAY_STARTUP_OFF) {
      relayState = 0;
    } else {
      relayState = loadRelayState(i);
    }
    myMessage.setSensor(myRelays[i].sensorId);
    send(myMessage.set(relayState)); // send current state
  }
  // Setup buttons
  for(int i = 0; i < numberOfButtons; i++) {
      // No Expander support for buttons (de-bouncing)
      myPhysicalButton[i].attach(myButtons[i].buttonPin, INPUT_PULLUP); // HIGH state when button is not pushed
      myPhysicalButton[i].interval(BUTTON_DEBOUNCE_INTERVAL);

      // make button to relay index
      buttonToRelayIndex[i].clickRelayNum = myButtons[i].clickRelayId > 0 ? getRelayNum(myButtons[i].clickRelayId) : -1;
      buttonToRelayIndex[i].longClickRelayNum = myButtons[i].longClickRelayId > 0 ? getRelayNum(myButtons[i].longClickRelayId) : -1;
      buttonToRelayIndex[i].doubleClickRelayNum = myButtons[i].doubleClickRelayId > 0 ? getRelayNum(myButtons[i].doubleClickRelayId) : -1;
      //TODO: initial state needed for REED_SWITCH and DING_DONG
      if ((myButtons[i].buttonType == DING_DONG) || (myButtons[i].buttonType == REED_SWITCH)) {
        myPhysicalButton[i].update();
        int buttonPinState = myPhysicalButton[i].read();
        int activeLevel = buttonPinState == (myButtons[i].buttonType == REED_SWITCH ? HIGH : LOW);
        if (activeLevel) {
          buttonToRelayIndex[i].buttonState = BTN_STATE_1ST_PRESS;
        } else {
          buttonToRelayIndex[i].buttonState = BTN_STATE_INITIAL;
        }
      } else {
        buttonToRelayIndex[i].buttonState = BTN_STATE_INITIAL;
      }
      buttonToRelayIndex[i].startStateMillis = 0;
  }
}

void loop() {
  #ifdef DEBUG_STATS
    unsigned long loopStartMillis = millis();
  #endif

  for(int i = 0; i < numberOfButtons; i++) {
    int buttonAction = getButtonAction(i);
    int relayNum;

    if ((myButtons[i].buttonType == DING_DONG) || (myButtons[i].buttonType == REED_SWITCH)) {
      relayNum = buttonToRelayIndex[i].clickRelayNum;
      #ifdef MY_DEBUG
        if (buttonAction & BUTTON_CHANGED) {
          Serial.print("# relay: ");
          Serial.print(relayNum);
          Serial.print(", myRelayState: ");
          Serial.print(myRelayState[relayNum]);
          Serial.print(", buttonAction: ");
          Serial.println(buttonAction);
        }
      #endif
      myMessage.setSensor(myRelays[relayNum].sensorId);
      if ((buttonAction & BUTTON_CHANGED) && ((buttonAction & BUTTON_PRESSED) > 0) && (myRelayState[relayNum] == 0)) { // button pressed or door/window opened
          changeRelayState(relayNum, 1);
          saveRelayState(relayNum, 1, false);
          Serial.println("# ON");
          //send(myMessage.set(1));
      } else if ((buttonAction & BUTTON_CHANGED) && ((buttonAction & BUTTON_PRESSED) == 0) && (myRelayState[relayNum] == 1)) { // button released or door/window closed
          changeRelayState(relayNum, 0);
          saveRelayState(relayNum, 0, false);
          Serial.println("# OFF");
          //send(myMessage.set(0));
      }
      continue;
    }
    if (buttonAction & BUTTON_CLICK) {
      relayNum = buttonToRelayIndex[i].clickRelayNum;
    } else if (buttonAction & BUTTON_DOUBLE_CLICK) {
      relayNum = buttonToRelayIndex[i].doubleClickRelayNum;
    } else if (buttonAction & BUTTON_LONG_PRESS) {
      relayNum = buttonToRelayIndex[i].longClickRelayNum;
    } else {
      continue;
    }
    #ifdef MY_DEBUG
      if (relayNum == -1) {
        Serial.print("# button action (-1): " + i);
        Serial.println(", buttonAction: " + buttonAction);
      }
    #endif
    myMessage.setSensor(myRelays[relayNum].sensorId);

    if (buttonAction & BUTTON_ACTION_MASK) {
      uint8_t isTurnedOn = ! loadRelayState(relayNum); // 1 - true, 0 - false
      changeRelayState(relayNum, isTurnedOn);
      send(myMessage.set(isTurnedOn));
      saveRelayState(relayNum, isTurnedOn);
      if ((myRelays[relayNum].relayOptions & RELAY_IMPULSE) && isTurnedOn) {
        myRelayImpulseStart[relayNum] = millis();
        impulsePending++;
      }
    }
  }
  
  if (impulsePending) {
    unsigned long currentMillis = millis();
    for(int i = 0; i < numberOfRelays; i++) {
      // Ignore multi-buttons
      if ((myRelays[i].relayOptions & RELAY_IMPULSE) && (myRelayImpulseStart[i] > 0)) {
        // the "|| (currentMillis < myRelayImpulseStart[i])" is for "millis()" overflow protection
        if ((currentMillis > myRelayImpulseStart[i]+RELAY_IMPULSE_INTERVAL) || (currentMillis < myRelayImpulseStart[i])) {
          if (myRelayState[i] == 1) {
            changeRelayState(i, 0);
            saveRelayState(i, 0);
          }
          myMessage.setSensor(myRelays[i].sensorId);
          send(myMessage.set(0)); // send current state
          myRelayImpulseStart[i] = 0;
          impulsePending--;
        }
      }
    }
    if (impulsePending < 0) impulsePending = 0;
  }

  #ifdef DEBUG_STATS
    if (debugStatsOn) {
      loopCumulativeMillis += millis() - loopStartMillis;
      loopCounter++;
      if ( loopCounter > DEBUG_STATS) {
        unsigned long loopIntervalCurrent = millis();
        Serial.print("# loop stats: 1000 loops=");
        Serial.print(loopIntervalCurrent - loopInterval);
        Serial.print("ms, cumulative_loop_millis=");
        Serial.print(loopCumulativeMillis);
        Serial.println("ms");
        loopInterval = loopIntervalCurrent;
        loopCumulativeMillis = 0;
        loopCounter = 0;
      }
    }
  #endif
}



// MySensors - Presentation
// Your sensor must first present itself to the controller.
// The presentation is a hint to allow controller prepare for the sensor data that eventually will come.
// Executed after "before()" and before "setup()" in: _begin (MySensorsCore.cpp) > gatewayTransportInit() > presentNode()
void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Multi Relay", "1.4");
  
  // Register every relay as separate sensor
  for (int i = 0; i < numberOfRelays; i++) {
    // Register all sensors to gw (they will be created as child devices)
    // void present(uint8_t childSensorId, uint8_t sensorType, const char *description, bool ack);
    //   childSensorId - The unique child id you want to choose for the sensor connected to this Arduino. Range 0-254.
    //   sensorType - The sensor type you want to create.
    //   description An optional textual description of the attached sensor.
    //   ack - Set this to true if you want destination node to send ack back to this node. Default is not to request any ack.
    present(myRelays[i].sensorId, S_BINARY, myRelays[i].relayDescription);
  }
}


// MySensors - Handling incoming messages
// Nodes that expects incoming data, such as an actuator or repeating nodes,
// must implement the receive() - function to handle the incoming messages.
// Do not sleep a node where you expect incoming data or you will lose messages.
void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type == V_STATUS) {
    uint8_t isTurnedOn = message.getBool(); // 1 - true, 0 - false
    int relayNum = getRelayNum(message.sensor);
    if (relayNum == -1) return;
    changeRelayState(relayNum, isTurnedOn);
    if ((myRelays[relayNum].relayOptions & RELAY_IMPULSE) && isTurnedOn) {
      myRelayImpulseStart[relayNum] = millis();
      impulsePending++;
    }
    // Store state in eeprom if changed
    if (loadRelayState(relayNum) != isTurnedOn) {
      saveRelayState(relayNum, isTurnedOn);
    }
    myMessage.setSensor(myRelays[relayNum].sensorId);
    send(myMessage.set(isTurnedOn)); // support for OPTIMISTIC=FALSE (Home Asistant)
    #ifdef MY_DEBUG
      // Write some debug info
      Serial.print("# Incoming change for sensor: " + relayNum);
      Serial.println(", New status: " + isTurnedOn);
    #endif
  #ifdef DEBUG_STATS
  } else if (message.type == V_VAR1) {
      debugStatsOn = message.getBool();
      Serial.print("# Debug is: ");
      Serial.println(debugStatsOn);
  #endif
  }
}

uint8_t loadRelayState(int relayNum, uint8_t forceEeprom) {
  uint8_t relayState;
  if (forceEeprom) {
    relayState = loadState(RELAY_STATE_STORAGE + relayNum);
  } else {
    relayState = myRelayState[relayNum];
  }
  #ifdef MY_DEBUG
    Serial.print("# loadRelayState: ");
    Serial.print(relayNum);
    if (forceEeprom) {
      Serial.print("(byte ");
      Serial.print(RELAY_STATE_STORAGE + relayNum);
      Serial.print(")");
    }
    Serial.print(" = ");
    Serial.println(relayState);
  #endif
  if (relayState > 1) relayState = 0;
  return(relayState);
}

void saveRelayState(int relayNum, uint8_t state, uint8_t useEeprom) {
  
  myRelayState[relayNum] = state;
  if (useEeprom) {
    saveState(RELAY_STATE_STORAGE + relayNum, state);
  }
}

void saveRelayState(int relayNum, uint8_t state) {
  uint8_t useEeprom = ((myRelays[relayNum].relayOptions & RELAY_STARTUP_MASK) == 0);
  saveRelayState(relayNum, state, useEeprom);
}

void changeRelayState(int relayNum, uint8_t relayState) {
  
  uint8_t relayTrigger = myRelays[relayNum].relayOptions & RELAY_TRIGGER_HIGH;
  uint8_t digitalOutState = relayState ? relayTrigger : ! relayTrigger;
  
  #ifdef USE_EXPANDER
    if ( myRelays[relayNum].relayPin & 0xff00 ) {
      int expanderNo = (myRelays[relayNum].relayPin >> 8) - 1;
      int expanderPin = myRelays[relayNum].relayPin & 0xff;
      expander[expanderNo].digitalWrite(expanderPin, digitalOutState);
    } else {
  #endif
    digitalWrite(myRelays[relayNum].relayPin, digitalOutState);
  #ifdef USE_EXPANDER
    }
  #endif
}

int getRelayNum(int sensorId) {
  for (int i = 0; i < numberOfRelays; i++) {
    if (myRelays[i].sensorId == sensorId) return(i);
  }
  return(-1);
}

int getButtonAction(int buttonNum) {

  int result = BUTTON_NO_EVENT;
  bool isPinChanged = myPhysicalButton[buttonNum].update();
  int buttonPinState = myPhysicalButton[buttonNum].read();
  int activeLevel = buttonPinState == (myButtons[buttonNum].buttonType == REED_SWITCH ? HIGH : LOW);

  bool hasLongClick = buttonToRelayIndex[buttonNum].longClickRelayNum != -1;
  bool hasDoubleClick = buttonToRelayIndex[buttonNum].doubleClickRelayNum != -1;


  #ifdef MY_DEBUG
    // Serial.print("# Button ");
    // Serial.print(buttonNum);
    // Serial.print(" changed to: ");
    // Serial.println(buttonPinState);
  #endif

  unsigned long now = millis();

  if (buttonToRelayIndex[buttonNum].buttonState == BTN_STATE_INITIAL) { // waiting for change
    if (isPinChanged) {
      buttonToRelayIndex[buttonNum].startStateMillis = now;
      if (myButtons[buttonNum].buttonType == BI_STABLE) {
        buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_1ST_CHANGE_BI;
      } else {
        buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_1ST_PRESS;
        result = BUTTON_PRESSED;
      }
    }
  
  // BI_STABLE buttons only state
  } else if (buttonToRelayIndex[buttonNum].buttonState == BTN_STATE_1ST_CHANGE_BI) { // waiting for next change
    // waiting for second change or timeout
    if (!hasDoubleClick || ((now - buttonToRelayIndex[buttonNum].startStateMillis) > BUTTON_DOUBLE_CLICK_INTERVAL)) {
      // this was only a single short click
      result = BUTTON_CLICK;
      buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_INITIAL;
    } else if (isPinChanged) {
      result = BUTTON_DOUBLE_CLICK;
      buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_INITIAL;
    }

  } else if (buttonToRelayIndex[buttonNum].buttonState == BTN_STATE_1ST_PRESS) { // waiting for 1st release

    if (!activeLevel) {
      if (!hasDoubleClick) {
        result = BUTTON_CLICK;
        buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_INITIAL;
      } else {
        result = BUTTON_NO_EVENT;
        buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_1ST_RELEASE;
      }

    } else { // (activeLevel == true)
      if ((!hasDoubleClick) && (!hasLongClick) && (buttonPinState == MONO_STABLE_TRIGGER)) { // no long/double-click action, do click (old behavior)
        result = BUTTON_CLICK | BUTTON_PRESSED;
        buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_RELEASE_WAIT;
      } else if (hasLongClick && ((now - buttonToRelayIndex[buttonNum].startStateMillis) > BUTTON_LONG_PRESS_INTERVAL)) {
        result = BUTTON_LONG_PRESS | BUTTON_PRESSED;
        buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_RELEASE_WAIT;
      } else {
        // Button was pressed down and timeout dind't occured - wait in this state
      }
    }

  } else if (buttonToRelayIndex[buttonNum].buttonState == BTN_STATE_1ST_RELEASE) {
    // waiting for press the second time or timeout
    if ((now - buttonToRelayIndex[buttonNum].startStateMillis) > BUTTON_DOUBLE_CLICK_INTERVAL) {
      // this was only a single short click
      result = BUTTON_CLICK;
      buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_INITIAL;

    } else if (activeLevel) {
      if (buttonPinState == MONO_STABLE_TRIGGER) {
        result = BUTTON_DOUBLE_CLICK | BUTTON_PRESSED;
        buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_RELEASE_WAIT;
      } else {
        result = BUTTON_PRESSED;
        buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_2ND_PRESS;
      }
    }

  } else if (buttonToRelayIndex[buttonNum].buttonState == BTN_STATE_2ND_PRESS) { // waiting for second release
    if (!activeLevel) {
      // this was a 2 click sequence.
      result = BUTTON_DOUBLE_CLICK;
      buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_INITIAL;
    }

  } else if (buttonToRelayIndex[buttonNum].buttonState == BTN_STATE_RELEASE_WAIT) { // waiting for release after long press
    if (!activeLevel) {
      buttonToRelayIndex[buttonNum].buttonState = BTN_STATE_INITIAL;
    }
  }

  if (isPinChanged) {
    result |= BUTTON_CHANGED;
  }
  return result;
};
