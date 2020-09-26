#include <Arduino.h>
#include <assert.h>
#include <EEPROM.h>
#include <Relay.h>
#include <Button.h>
#define MY_GATEWAY_SERIAL
#include <MySensors.h>

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

const int gNumberOfRelays = sizeof(gRelayConfig) / sizeof(RelayConfigDef);
const int gNumberOfButtons = sizeof(gButtonConfig) / sizeof(ButtonConfigDef);
#ifdef USE_EXPANDER
  const int gNumberOfExpanders = sizeof(expanderAddresses);
  #if defined(PCF8574_H)
    PCF8574 expander[numberOfExpanders];
  #elif defined(_Adafruit_MCP23017_H_)
    Adafruit_MCP23017 gExpander[gNumberOfExpanders];
  #endif
#endif

#ifdef DEBUG_STATS
  bool debugStatsOn = true;
  int loopCounter = 0;
  unsigned long loopInterval = 0;
  unsigned long loopCumulativeMillis = 0;
#endif


MyMessage myMessage; // MySensors - Sending Data
Relay gRelay[gNumberOfRelays];
MyButton gButton[gNumberOfButtons];

int getRelayNum(int sensorId);


// MySensors - This will execute before MySensors starts up
void before() {
  Serial.begin(115200);

  // validate config
  #ifdef USE_EXPANDER
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      int pin = gRelayConfig[relayNum].relayPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)) {
          Serial.print("Configuration failed - expander no or number of pins out of range for relay: ");
          Serial.println(relayNum);
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
          Serial.print("Configuration failed - expander no or number of pins out of range for button: ");
          Serial.println(buttonNum);
          delay(1000);
          assert(0);
        }
      }
    #endif
    const char * failAction[] = {"OK", "click", "long-press", "double-click"};
    int fail = 0;
    if ((gButtonConfig[buttonNum].clickRelayId != -1) && (getRelayNum(gButtonConfig[buttonNum].clickRelayId) == -1)) fail = 1;
    if ((gButtonConfig[buttonNum].longClickRelayId != -1) && (getRelayNum(gButtonConfig[buttonNum].longClickRelayId) == -1)) fail = 2;
    if ((gButtonConfig[buttonNum].doubleClickRelayId != -1) && (getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId) == -1)) fail = 3;
    if (fail) {
        Serial.print("Configuration failed - invalid '");
        Serial.print(failAction[fail]);
        Serial.print(" relay ID' for button: ");
        Serial.println(buttonNum);
        delay(1000);
        assert(0);
    }
  }
  
  // if version has changed, reset state of all relays
  bool versionChangeResetState = (MULTI_RELAY_VERSION == EEPROM.read(0) ) ? false : true;
  
  #ifdef USE_EXPANDER
    /* Start I2C bus and PCF8574 instance */
    for(int i = 0; i < gNumberOfExpanders; i++) {
      gExpander[i].begin(expanderAddresses[i]);
    }

    Relay::expanderInit(gExpander);
    MyButton::expanderInit(gExpander);
  #endif

  // initialize relays
  Relay::setImpulseInterval(RELAY_IMPULSE_INTERVAL);

  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
    
    gRelay[relayNum].initialize(relayNum, gRelayConfig[relayNum].sensorId, gRelayConfig[relayNum].relayDescription);
    gRelay[relayNum].attachPin(gRelayConfig[relayNum].relayPin);
    gRelay[relayNum].setModeAndStartupState(gRelayConfig[relayNum].relayOptions, versionChangeResetState);
    gRelay[relayNum].start();
  }
  if (versionChangeResetState) {
    // version has changed, so store new version in eeporom
    EEPROM.write(0, MULTI_RELAY_VERSION);
  }
} // before()

// executed AFTER mysensors has been initialised
void setup() {
  // Send state to MySensor Gateway
  myMessage.setType(V_STATUS);
  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
    myMessage.setSensor(gRelay[relayNum].getSensorId());
    send(myMessage.set(gRelay[relayNum].getState())); // send current state
  }

  // Setup buttons
  MyButton::setEventIntervals(BUTTON_DOUBLE_CLICK_INTERVAL, BUTTON_LONG_PRESS_INTERVAL);
  MyButton::setMonoStableTrigger(MONO_STABLE_TRIGGER);

  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
    
    gButton[buttonNum].initialize(gButtonConfig[buttonNum].buttonType, gButtonConfig[buttonNum].buttonDescription);
    gButton[buttonNum].setAction(getRelayNum(gButtonConfig[buttonNum].clickRelayId),
                                 getRelayNum(gButtonConfig[buttonNum].longClickRelayId),
                                 getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
    gButton[buttonNum].setDebounceInterval(BUTTON_DEBOUNCE_INTERVAL);
    gButton[buttonNum].attachPin(gButtonConfig[buttonNum].buttonPin);
  }
}

void loop() {
  #ifdef DEBUG_STATS
    unsigned long loopStartMillis = millis();
  #endif

  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
    
    int relayNum = gButton[buttonNum].updateAndGetRelayNum();
    if (relayNum > -1) {
      // mono/bi-stable button toggles the relay, ding-dong/reed-switch switch to exact state
      bool relayState = gButton[buttonNum].getRelayState(gRelay[relayNum].getState());

      if (gRelay[relayNum].changeState(relayState)) {
        myMessage.setSensor(gRelay[relayNum].getSensorId());
        send(myMessage.set(relayState));
      }
    }
  }
  
  if (Relay::isImpulsePending()) {
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      if (gRelay[relayNum].impulseProcess()) {
        myMessage.setSensor(gRelay[relayNum].getSensorId());
        send(myMessage.set(0));
      }
    }
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
  sendSketchInfo("Multi Relay", "2.0");
  
  // Register every relay as separate sensor
  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
    // Register all sensors to gw (they will be created as child devices)
    // void present(uint8_t childSensorId, uint8_t sensorType, const char *description, bool ack);
    //   childSensorId - The unique child id you want to choose for the sensor connected to this Arduino. Range 0-254.
    //   sensorType - The sensor type you want to create.
    //   description An optional textual description of the attached sensor.
    //   ack - Set this to true if you want destination node to send ack back to this node. Default is not to request any ack.
    present(gRelay[relayNum].getSensorId(), S_BINARY, gRelay[relayNum].getDescription());
  }
}


// MySensors - Handling incoming messages
// Nodes that expects incoming data, such as an actuator or repeating nodes,
// must implement the receive() - function to handle the incoming messages.
// Do not sleep a node where you expect incoming data or you will lose messages.
void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.getCommand() == C_SET) {
    if (message.getType() == V_STATUS) {
      uint8_t isTurnedOn = message.getBool(); // 1 - true, 0 - false
      int relayNum = getRelayNum(message.sensor);
      if (relayNum == -1) return;
      gRelay[relayNum].changeState(isTurnedOn);
      myMessage.setSensor(gRelay[relayNum].getSensorId());
      send(myMessage.set(isTurnedOn)); // support for OPTIMISTIC=FALSE (Home Asistant)
      #ifdef MY_DEBUG
        // Write some debug info
        Serial.print("# Incoming change for sensor: " + relayNum);
        Serial.println(", New status: " + isTurnedOn);
      #endif
    #ifdef DEBUG_STATS
    } else if (message.getType() == V_VAR1) {
        debugStatsOn = message.getBool();
        Serial.print("# Debug is: ");
        Serial.println(debugStatsOn);
    #endif
    }
  }
}


int getRelayNum(int sensorId) {
  
  if (sensorId > -1) {
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      if (gRelayConfig[relayNum].sensorId == sensorId) return(relayNum);
    }
  }
  return(-1);
}
