#include <Arduino.h>
#include <assert.h>
#include <EEPROM.h>
#include <RelayService.h>
#include <Relay.h>
#include <Button.h>
#define MY_GATEWAY_SERIAL
#include <MySensors.h>

using namespace lkankowski;

#define xstr(a) str(a)
#define str(a) #a
const char * MULTI_RELAY_VERSION = xstr(SKETCH_VERSION);

#if defined(EXPANDER_PCF8574) || defined(EXPANDER_MCP23017)
  #if defined(EXPANDER_PCF8574)
    #include "PCF8574.h"
    #define EXPANDER_PINS 8
  #elif defined(EXPANDER_MCP23017)
    #include "Adafruit_MCP23017.h"
    #define EXPANDER_PINS 16
  #endif
  #define USE_EXPANDER
  #include <Wire.h>    // Required for I2C communication
  #define E(expanderNo, ExpanderPin) (((expanderNo+1)<<8) | (ExpanderPin))
#endif

// Configuration in separate file
#include "config.h"

#ifdef USE_EXPANDER
  const int gNumberOfExpanders = sizeof(expanderAddresses);
  #if defined(PCF8574_H)
    PCF8574 gExpander[gNumberOfExpanders];
  #elif defined(_Adafruit_MCP23017_H_)
    Adafruit_MCP23017 gExpander[gNumberOfExpanders];
  #endif
#endif

int gTurnOffDependentsCounter = 2000;

#ifdef DEBUG_STATS
  bool debugStatsOn = false;
  int loopCounter = 0;
  unsigned long loopInterval = 0;
  unsigned long loopCumulativeMillis = 0;
#endif
#ifdef DEBUG_STARTUP
  unsigned long debugCounter = 0;
#endif
  
MyMessage myMessage; // MySensors - Sending Data
#if defined(DEBUG_COMMUNICATION) || defined(DEBUG_STATS)
  MyMessage debugMessage(255, V_TEXT);
#endif

#include <common.h>

Relay gRelay[gNumberOfRelays];
RelayService gRelayService(gNumberOfRelays, gRelay, gRelayConfig);
lkankowski::Button gButton[gNumberOfButtons];

void(* resetFunc) (void) = 0; //declare reset function at address 0


// MySensors - This will execute before MySensors starts up
void before() {
  Serial.begin(115200);

  #ifdef DEBUG_STARTUP
    Serial.println(String("# ")+(debugCounter++)+" Debug startup - common config: MONO_STABLE_TRIGGER="+MONO_STABLE_TRIGGER
                   +", RELAY_IMPULSE_INTERVAL="+RELAY_IMPULSE_INTERVAL+", BUTTON_DEBOUNCE_INTERVAL="+BUTTON_DEBOUNCE_INTERVAL
                   +", BUTTON_DOUBLE_CLICK_INTERVAL="+BUTTON_DOUBLE_CLICK_INTERVAL+", BUTTON_LONG_PRESS_INTERVAL="+BUTTON_LONG_PRESS_INTERVAL
                   +", MULTI_RELAY_VERSION="+MULTI_RELAY_VERSION);

    #ifdef USE_EXPANDER
      Serial.println(String("# ")+(debugCounter++)+" Debug startup - expander config");
      for(int i = 0; i < gNumberOfExpanders; i++) {
        Serial.print(expanderAddresses[i]);
        Serial.print(",");
      }
      Serial.println();
    #endif

    Serial.println(String("# ")+(debugCounter++)+" Debug startup - relay config");
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      Serial.println(String("# ")+(debugCounter++)+" > "+gRelayConfig[relayNum].sensorId+";"+gRelayConfig[relayNum].relayPin+";"
                     +gRelayConfig[relayNum].relayOptions+";"+gRelayConfig[relayNum].relayDescription);
    }
    Serial.println(String("# ")+(debugCounter++)+" Debug startup - button config");
    for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
      Serial.println(String("# ")+(debugCounter++)+" > "+gButtonConfig[buttonNum].buttonPin+";"+gButtonConfig[buttonNum].buttonType+";"
                     +gButtonConfig[buttonNum].clickRelayId+";"+gButtonConfig[buttonNum].longClickRelayId+";"
                     +gButtonConfig[buttonNum].doubleClickRelayId+";"+gButtonConfig[buttonNum].buttonDescription);
    }
    Serial.println(String("# ")+(debugCounter++)+" Debug startup - EEPROM (first value is version, relay state starts at "+RELAY_STATE_STORAGE+")");
    Serial.print(String("# ")+(debugCounter++)+" ");
    for (int relayNum = 0; relayNum < gNumberOfRelays+1; relayNum++) {
      Serial.print(EEPROM.read(relayNum));
      Serial.print(",");
    }
    Serial.println();
    Serial.println(String("# ")+(debugCounter++)+" Debug startup - buttons pin state");
    Serial.print(String("# ")+(debugCounter++)+" ");
    for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
      pinMode(gButtonConfig[buttonNum].buttonPin, INPUT_PULLUP);
    }
    delay(200);
    for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
      Serial.print(digitalRead(gButtonConfig[buttonNum].buttonPin));
      Serial.print(",");
    }
    Serial.println();
  #endif

  // validate config
  #ifdef USE_EXPANDER
    //TODO: check if I2C pins are not used
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      int pin = gRelayConfig[relayNum].relayPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)) {
          Serial.println(String("Configuration failed - expander no or number of pins out of range for relay: ") + relayNum);
          delay(1000);
          assert(0);
        }
      }
    }
  #endif
  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
    #ifdef USE_EXPANDER
      int pin = gButtonConfig[buttonNum].buttonPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)) {
          Serial.println(String("Configuration failed - expander no or number of pins out of range for button: ") + buttonNum);
          delay(1000);
          assert(0);
        }
      }
    #endif
    const char * failAction[] = {"OK", "click", "long-press", "double-click"};
    int fail = 0;
    if ((gButtonConfig[buttonNum].clickRelayId != -1) && (gRelayService.getRelayNum(gButtonConfig[buttonNum].clickRelayId) == -1)) fail = 1;
    if ((gButtonConfig[buttonNum].longClickRelayId != -1) && (gRelayService.getRelayNum(gButtonConfig[buttonNum].longClickRelayId) == -1)) fail = 2;
    if ((gButtonConfig[buttonNum].doubleClickRelayId != -1) && (gRelayService.getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId) == -1)) fail = 3;
    if (fail) {
        Serial.println(String("Configuration failed - invalid '") + failAction[fail] + " relay ID' for button: " + buttonNum);
        delay(1000);
        assert(0);
    }
    // TODO: validate if pin is correct to the current board
  }
  
  // if version has changed, reset state of all relays
  bool versionChangeResetState = (CONFIG_VERSION == EEPROM.read(0) ) ? false : true;
  
  #ifdef USE_EXPANDER
    /* Start I2C bus and PCF8574 instance */
    for(int i = 0; i < gNumberOfExpanders; i++) {
      gExpander[i].begin(expanderAddresses[i]);
    }

    Relay::expanderInit(gExpander);
    lkankowski::Button::expanderInit(gExpander);
  #endif

  // initialize relays
  gRelayService.setImpulseInterval(RELAY_IMPULSE_INTERVAL);
  gRelayService.initialize(versionChangeResetState);
  
  if (versionChangeResetState) {
    // version has changed, so store new version in eeporom
    EEPROM.write(0, CONFIG_VERSION);
  }
}; // before()


// executed AFTER mysensors has been initialised
void setup() {
  // Send state to MySensor Gateway
  myMessage.setType(V_STATUS);
  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
    myMessage.setSensor(gRelay[relayNum].getSensorId());
    send(myMessage.set(gRelay[relayNum].getState())); // send current state
  }

  // Setup buttons
  lkankowski::Button::Button::setEventIntervals(BUTTON_DOUBLE_CLICK_INTERVAL, BUTTON_LONG_PRESS_INTERVAL);
  lkankowski::Button::Button::setMonoStableTrigger(MONO_STABLE_TRIGGER);

  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
    
    gButton[buttonNum].initialize(gButtonConfig[buttonNum].buttonType, gButtonConfig[buttonNum].buttonDescription);
    gButton[buttonNum].setAction(gRelayService.getRelayNum(gButtonConfig[buttonNum].clickRelayId),
                                 gRelayService.getRelayNum(gButtonConfig[buttonNum].longClickRelayId),
                                 gRelayService.getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
    gButton[buttonNum].setDebounceInterval(BUTTON_DEBOUNCE_INTERVAL);
    gButton[buttonNum].attachPin(gButtonConfig[buttonNum].buttonPin);
  }
};

void loop() {
  #ifdef DEBUG_STATS
    unsigned long loopStartMillis = millis();
    if (loopCounter == 0) {
      loopCumulativeMillis = 0;
      loopInterval = loopStartMillis;
    }
  #endif

  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
    
    int relayNum = gButton[buttonNum].updateAndGetRelayNum();
    if (relayNum > -1) {
      // mono/bi-stable button toggles the relay, ding-dong/reed-switch switch to exact state
      bool relayState = gButton[buttonNum].getRelayState(gRelay[relayNum].getState());

      #ifdef IGNORE_BUTTONS_START_MS
        if (millis() > IGNORE_BUTTONS_START_MS) {
      #endif
          if (gRelayService.changeState(relayNum, relayState)) {
            myMessage.setSensor(gRelay[relayNum].getSensorId());
            send(myMessage.set(relayState));
          }
      #ifdef IGNORE_BUTTONS_START_MS
        }
      #endif
    }
  }
  
  if (gRelayService.isImpulsePending()) {
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      if (gRelayService.impulseProcess(relayNum)) {
        myMessage.setSensor(gRelay[relayNum].getSensorId());
        send(myMessage.set(0));
      }
    }
  }

  if (--gTurnOffDependentsCounter <= 0) {
    gRelayService.turnOffDependent();
    gTurnOffDependentsCounter = 500;
  }

  #ifdef DEBUG_STATS
    if (debugStatsOn) {
      unsigned long loopIntervalCurrent = millis();
      loopCumulativeMillis += loopIntervalCurrent - loopStartMillis;
      loopCounter++;
      if ( loopCounter > DEBUG_STATS) {
        Serial.println(String("# ") + DEBUG_STATS + " loop stats: (end-start)=" + (loopIntervalCurrent - loopInterval)
                       + "ms, cumulative_loop_millis=" + loopCumulativeMillis + "ms");
        loopCounter = 0;
        debugStatsOn = false;
      }
    }
  #endif
};


// MySensors - Presentation - Your sensor must first present itself to the controller.
// Executed after "before()" and before "setup()"
void presentation() {
  sendSketchInfo(MULTI_RELAY_DESCRIPTION, MULTI_RELAY_VERSION );
  
  // Register every relay as separate sensor
  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
    present(gRelay[relayNum].getSensorId(), S_BINARY, gRelay[relayNum].getDescription());
  }
};


// MySensors - Handling incoming messages
// Nodes that expects incoming data, must implement the receive() function to handle the incoming messages.
void receive(const MyMessage &message) {

  #ifdef DEBUG_COMMUNICATION
    Serial.println(String("# Incoming message: sensorId=") + message.getSensor() + ", command=" + message.getCommand()
                 + ", ack=" + message.isAck() + ", echo=" + message.isEcho() + ", type=" + message.getType()
                 + ", payload=" + message.getString());
  #endif
  if (message.getCommand() == C_SET) {
    if (message.getType() == V_STATUS) {
      int relayNum = gRelayService.getRelayNum(message.getSensor());
      if (relayNum == -1) return;
      gRelayService.changeState(relayNum, message.getBool());
      myMessage.setSensor(message.getSensor());
      send(myMessage.set(message.getBool())); // support for OPTIMISTIC=FALSE (Home Asistant)
    #ifdef DEBUG_STATS
    } else if (message.getType() == V_VAR1) {
      int debugCommand = message.getInt();
      if (debugCommand == 1) {
        debugStatsOn = message.getBool();
        loopCounter = 0;
        send(debugMessage.set("toogle debug stats"));
      } else if (debugCommand == 2) {
        for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
          Serial.println(String("# Sensor ") + gRelay[relayNum].getSensorId() + " state="
                         + gRelay[relayNum].getState() + "; " + gRelay[relayNum].getDescription());
        }
      } else if (debugCommand == 3) {
        for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
          Serial.println(String("# Button ") + buttonNum + ": " + gButton[buttonNum].toString());
        }
      } else if (debugCommand == 4) { // dump EEPROM
        Serial.print("# Dump EEPROM: ");
        for (int relayNum = 0; relayNum < gNumberOfRelays+RELAY_STATE_STORAGE; relayNum++) {
          Serial.print(EEPROM.read(relayNum));
          Serial.print(",");
        }
        Serial.println();
      } else if (debugCommand == 5) { // clear EEPROM & reset
        for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
          EEPROM.write(RELAY_STATE_STORAGE + relayNum, 0);
        }
        resetFunc();
      } else if (debugCommand == 6) { // reset
        resetFunc();
      }
    #endif
    }
  }
};
