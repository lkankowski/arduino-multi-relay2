#ifndef UNIT_TEST
#ifdef ARDUINO

#include <ArduinoAbstract.h>
#include <EepromAbstract.h>
#include <RelayService.h>
#include <ButtonService.h>
#include <MySensorsWrapper.h>
#define MY_GATEWAY_SERIAL
#include <MySensors.h>

using namespace lkankowski;

// Configuration in separate file
#include <Configuration.h>
#include "config.h"

#ifdef USE_EXPANDER
  #if defined(EXPANDER_PCF8574)
    PCF8574 gExpander[sizeof(gExpanderAddresses)];
  #elif defined(EXPANDER_MCP23017)
    Adafruit_MCP23017 gExpander[sizeof(gExpanderAddresses)];
  #endif
  PinCreator gPinCreator(gExpander, gExpanderAddresses, sizeof(gExpanderAddresses));
#else
  PinCreator gPinCreator;
#endif

int gTurnOffDependentsCounter = 2000;

#ifdef DEBUG_STATS
  bool debugStatsOn = false;
  int loopCounter = 0;
  unsigned long loopInterval = 0;
  unsigned long loopCumulativeMillis = 0;
#endif

MyMessage myMessage; // MySensors - Sending Data
#if defined(DEBUG_COMMUNICATION) || defined(DEBUG_STATS)
  MyMessage debugMessage(255, V_TEXT);
#endif

RelayConfigRef gRelayConfigRef = {gRelayConfig, sizeof(gRelayConfig) / sizeof(RelayConfigDef)};
ButtonConfigRef gButtonConfigRef = {gButtonConfig, sizeof(gButtonConfig) / sizeof(ButtonConfigDef)};
Configuration gConfiguration(gRelayConfigRef, gButtonConfigRef
  #ifdef USE_EXPANDER
    , gExpanderAddresses, sizeof(gExpanderAddresses)
  #endif
);

Eeprom gEeprom;
MySensorsWrapper gMySensorsWrapper(gConfiguration);
RelayService gRelayService(gConfiguration, gEeprom, gMySensorsWrapper);
ButtonService gButtonService(gConfiguration, BUTTON_DEBOUNCE_INTERVAL, gRelayService);

void(* resetFunc) (void) = 0; //declare reset function at address 0


// MySensors - This will execute before MySensors starts up
void before()
{
  Serial.begin(115200);

  #ifdef DEBUG_STARTUP
    Serial << F("# Debug startup - common config: MONO_STABLE_TRIGGER=") << MONO_STABLE_TRIGGER
           << F(", RELAY_IMPULSE_INTERVAL=") << RELAY_IMPULSE_INTERVAL
           << F(", BUTTON_DEBOUNCE_INTERVAL=") << BUTTON_DEBOUNCE_INTERVAL
           << F(", BUTTON_DOUBLE_CLICK_INTERVAL=") << BUTTON_DOUBLE_CLICK_INTERVAL
           << F(", BUTTON_LONG_PRESS_INTERVAL=") << BUTTON_LONG_PRESS_INTERVAL
           << F(", MULTI_RELAY_VERSION=") << F(xstr(SKETCH_VERSION)) << "\n";

    #ifdef USE_EXPANDER
      Serial << F("# Debug startup - expander config\n");
      for(size_t i = 0; i < sizeof(gExpanderAddresses); i++) {
        Serial << gExpanderAddresses[i] << ",";
      }
      Serial << "\n";
    #endif

    Serial << F("# Debug startup - relay config\n");
    for (size_t relayNum = 0; relayNum < gRelayConfigRef.size; relayNum++) {
      Serial << F("# > ") << gConfiguration.getRelaySensorId(relayNum) << ";"
             << gConfiguration.getRelayPin(relayNum) << ";"
             << gConfiguration.getRelayOptions(relayNum) << ";"
             << gConfiguration.getRelayDescription(relayNum) << "\n";
    }
    Serial << F("# Debug startup - button config\n");
    for (size_t buttonNum = 0; buttonNum < gButtonConfigRef.size; buttonNum++) {
      Serial << F("# > ") << gConfiguration.getButtonPin(buttonNum) << ";"
             << gConfiguration.getButtonType(buttonNum) << ";"
             << gConfiguration.getButtonClickAction(buttonNum) << ";"
             << gConfiguration.getButtonLongClickAction(buttonNum) << ";"
             << gConfiguration.getButtonDoubleClickAction(buttonNum) << ";"
             << gConfiguration.getButtonDescription(buttonNum) << "\n";
    }
    Serial << F("# Debug startup - EEPROM (first value is version, relay state starts at ") << RELAY_STATE_STORAGE
           << F(")\n# > ");
    for (size_t relayNum = 0; relayNum < gRelayConfigRef.size+1; relayNum++) {
      Serial << gEeprom.read(relayNum) << ",";
    }
    Serial << F("\n# Debug startup - buttons pin state\n# > ");
    for (size_t buttonNum = 0; buttonNum < gButtonConfigRef.size; buttonNum++) {
      pinMode(gConfiguration.getButtonPin(buttonNum), INPUT_PULLUP);
    }
    delay(200);
    for (size_t buttonNum = 0; buttonNum < gButtonConfigRef.size; buttonNum++) {
      Serial << digitalRead(gConfiguration.getButtonPin(buttonNum)) << ",";
    }
    Serial << "\n";
  #endif

  if (PinCreator::instance() == nullptr) {
    Serial << F("PinCreator instance is NULL\n");
    haltSystem();
  }

  // if version has changed, reset state of all relays
  bool versionChangeResetState = (CONFIG_VERSION == gEeprom.read(0) ) ? false : true;
  
  #ifdef USE_EXPANDER
    gPinCreator.initExpanders();
  #endif

  // initialize relays
  gRelayService.setImpulseInterval(RELAY_IMPULSE_INTERVAL);
  gRelayService.initialize(versionChangeResetState);
  
  // Setup buttons
  ButtonInterface::setEventIntervals(BUTTON_DOUBLE_CLICK_INTERVAL, BUTTON_LONG_PRESS_INTERVAL);
  MonoStableButton::clickTriggerWhenPressed(true);
  gButtonService.attachPins();

  if (versionChangeResetState) {
    // version has changed, so store new version in eeporom
    gEeprom.write(0, CONFIG_VERSION);
  }
}; // before()


// executed AFTER mysensors has been initialised
void setup()
{
  // Send state to MySensor Gateway
  for (size_t relayNum = 0; relayNum < gConfiguration.getRelaysCount(); relayNum++) {
    gMySensorsWrapper.notify(relayNum, gRelayService.getState(relayNum));
  }
};


void loop()
{
  unsigned long loopStartMillis = millis();

  #ifdef DEBUG_STATS
    if (loopCounter == 0) {
      loopCumulativeMillis = 0;
      loopInterval = loopStartMillis;
    }
  #endif

  gRelayService.processImpulse(loopStartMillis);

  gButtonService.checkEventsAndDoActions(loopStartMillis);

  #ifdef DEBUG_STATS
    if (debugStatsOn) {
      unsigned long loopIntervalCurrent = millis();
      loopCumulativeMillis += loopIntervalCurrent - loopStartMillis;
      loopCounter++;
      if ( loopCounter > DEBUG_STATS) {
        Serial << F("# ") << DEBUG_STATS << F(" loop stats for ") << gConfiguration.getButtonsCount()
               << F(" buttons: (end-start)=") << (loopIntervalCurrent - loopInterval)
               << F("ms, cumulative_loop_millis=") << loopCumulativeMillis << F("ms\n");
        loopCounter = 0;
        debugStatsOn = false;
      }
    }
  #endif
  
  if (--gTurnOffDependentsCounter <= 0) {
    gRelayService.turnOffDependent(loopStartMillis);
    gTurnOffDependentsCounter = 500;
  }
};


// MySensors - Presentation - Your sensor must first present itself to the controller.
// Executed after "before()" and before "setup()"
void presentation()
{
  sendSketchInfo((reinterpret_cast<const __FlashStringHelper *>(MULTI_RELAY_DESCRIPTION)),
                 F(xstr(SKETCH_VERSION)));
  
  // Register every relay as separate sensor
  gMySensorsWrapper.present();
};


// MySensors - Handling incoming messages
// Nodes that expects incoming data, must implement the receive() function to handle the incoming messages.
void receive(const MyMessage &message)
{
  #ifdef DEBUG_COMMUNICATION
    Serial << F("# Incoming message: sensorId=") << message.getSensor()
           << F(", command=") << message.getCommand() << F(", ack=") << message.isAck()
           << F(", echo=") << message.getRequestEcho() << F(", type=") << message.getType()
           << F(", payload=") << message.getString() << "\n";
  #endif
  if (message.getCommand() == C_SET) {
    if (message.getType() == V_STATUS) {
      int relayNum = gConfiguration.getRelayNum(message.getSensor());
      if (relayNum == -1) return;
      gRelayService.changeRelayState(relayNum, message.getBool(), millis());
      if (! message.getRequestEcho()) {
        gMySensorsWrapper.notifyWithId(relayNum, message.getBool(), message.getSensor()); // support for OPTIMISTIC=FALSE (Home Asistant)
      }
    } else if (message.getType() == V_VAR1) {
      int debugCommand = message.getInt();
      if (debugCommand == 1) { // show stats
        #ifdef DEBUG_STATS
          debugStatsOn = true;
          loopCounter = 0;
          send(debugMessage.set(F("toogle debug stats")));
          gTurnOffDependentsCounter = DEBUG_STATS+2; // TODO: temporary
        #endif
      } else if (debugCommand == 2) { // dump relays state
        for (size_t relayNum = 0; relayNum < gConfiguration.getRelaysCount(); relayNum++) {
          gRelayService.printDebug(relayNum);
        }
      } else if (debugCommand == 3) { // dump buttons state
        for (size_t buttonNum = 0; buttonNum < gConfiguration.getButtonsCount(); buttonNum++) {
          Serial << F("# Button ") << buttonNum << F(": ");
          gButtonService.printDebug(buttonNum);
        }
      } else if (debugCommand == 4) { // dump EEPROM
        Serial << F("# Dump EEPROM: ");
        for (int eepromIdx = 0; eepromIdx < gEeprom.length(); eepromIdx++) {
          Serial << gEeprom.dump(eepromIdx) << ",";
        }
        Serial << "\n";
      } else if (debugCommand == 5) { // clear EEPROM & reset
        gEeprom.clean();
        resetFunc();
      } else if (debugCommand == 6) { // reset
        resetFunc();
      } else if (debugCommand == 7) { // check relays state & eeprom
        // debug feature
        for (size_t relayNum = 0; relayNum < gConfiguration.getRelaysCount(); relayNum++) {
          if ((gConfiguration.getRelayPin(relayNum) >= 0) &&
              (gRelayService.getState(relayNum) != (ArduinoPin::digitalRead(gConfiguration.getRelayPin(relayNum)) ==
                                                    (gConfiguration.getRelayOptions(relayNum) & RELAY_TRIGGER_HIGH))))
          {
            Serial << F("# Error state relay ") << gConfiguration.getRelaySensorId(relayNum)
                   << F(": state=") << gRelayService.getState(relayNum)
                   << F(", pin_state=") << ArduinoPin::digitalRead(gConfiguration.getRelayPin(relayNum)) << "\n";
          }
          if (((gConfiguration.getRelayOptions(relayNum) & RELAY_STARTUP_MASK) == 0) &&
              (gRelayService.getState(relayNum) != (gEeprom.read(RELAY_STATE_STORAGE + relayNum) == 1)))
          {
            Serial << F("# Error eeprom relay ") << gConfiguration.getRelaySensorId(relayNum)
                   << F(": state=") << gRelayService.getState(relayNum)
                   << F(", eeprom=") << gEeprom.read(RELAY_STATE_STORAGE + relayNum) << "\n";
          }
        }
      }
    }
  }
};

#else

#include <ArduinoAbstract.h>
#include <EepromAbstract.h>
#include <RelayService.h>
#include <ButtonService.h>
#include <iostream>

using namespace lkankowski;

#include <Configuration.h>
#include "config.h"

RelayConfigRef gRelayConfigRef = {gRelayConfig, sizeof(gRelayConfig) / sizeof(RelayConfigDef)};
ButtonConfigRef gButtonConfigRef = {gButtonConfig, sizeof(gButtonConfig) / sizeof(ButtonConfigDef)};
Configuration gConfiguration(gRelayConfigRef, gButtonConfigRef);

// Eeprom gEeprom;
// RelayService gRelayService(gConfiguration, gEeprom);
// ButtonService gButtonService(gConfiguration, BUTTON_DEBOUNCE_INTERVAL);

int main( int argc, char **argv) {

  std::cout << "hello" << std::endl;

};
#endif
#endif
