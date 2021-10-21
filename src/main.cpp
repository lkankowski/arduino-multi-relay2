#ifndef UNIT_TEST
#ifdef ARDUINO

#include <ArduinoAbstract.h>
#include <EepromAbstract.h>
#include <RelayService.h>
#include <ButtonService.h>
#define MY_GATEWAY_SERIAL
#include <MySensors.h>

using namespace lkankowski;

const char * MULTI_RELAY_VERSION = xstr(SKETCH_VERSION);

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

Vector<const RelayConfigDef> gRelayConfigRef(gRelayConfig, sizeof(gRelayConfig) / sizeof(RelayConfigDef));
Vector<const ButtonConfigDef> gButtonConfigRef(gButtonConfig, sizeof(gButtonConfig) / sizeof(ButtonConfigDef));
Configuration gConfiguration(gRelayConfigRef, gButtonConfigRef);

Eeprom gEeprom;
RelayService gRelayService(gConfiguration, gEeprom);
ButtonService gButtonService(gConfiguration, BUTTON_DEBOUNCE_INTERVAL);

FILE serial_stdout;
void(* resetFunc) (void) = 0; //declare reset function at address 0


// MySensors - This will execute before MySensors starts up
void before()
{
  Serial.begin(115200);

  // Set up redirect of stdout to serial
  fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &serial_stdout;

  #ifdef DEBUG_STARTUP
    printf_P(
      PSTR("# Debug startup - common config: MONO_STABLE_TRIGGER=%i, RELAY_IMPULSE_INTERVAL=%lu, BUTTON_DEBOUNCE_INTERVAL=%lu, BUTTON_DOUBLE_CLICK_INTERVAL=%lu, BUTTON_LONG_PRESS_INTERVAL=%lu, MULTI_RELAY_VERSION=%s\n"),
      MONO_STABLE_TRIGGER, RELAY_IMPULSE_INTERVAL, BUTTON_DEBOUNCE_INTERVAL, BUTTON_DOUBLE_CLICK_INTERVAL,
      BUTTON_LONG_PRESS_INTERVAL, MULTI_RELAY_VERSION
    );

    #ifdef USE_EXPANDER
      printf_P(PSTR("# Debug startup - expander config\n"));
      for(int i = 0; i < sizeof(gExpanderAddresses); i++) {
        printf("%i,",expanderAddresses[i]);
      }
      printf("\n");
    #endif

    printf_P(PSTR("# Debug startup - relay config\n"), );
    for (int relayNum = 0; relayNum < gRelayConfigRef.size(); relayNum++) {
      RelayConfigDef relayConfig = {};
      printf_P(
        PSTR("# > %i;%i;%i;%s;\n"), gConfiguration.getRelaySensorId(relayNum),
        gConfiguration.getRelayPin(relayNum), gConfiguration.getRelayOptions(relayNum), gConfiguration.getRelayDescription(relayNum)
      );
    }
    printf_P(PSTR("# Debug startup - button config\n"));
    for (int buttonNum = 0; buttonNum < gButtonConfigRef.size(); buttonNum++) {
      ButtonConfigDef buttonConfig = {};
      PROGMEM_readAnything(&gButtonConfig[buttonNum], buttonConfig);
      printf_P(
        PSTR("# > %i;%i;%i;%i;%i;%s;\n"), buttonConfig.buttonPin,
        buttonConfig.buttonType, buttonConfig.clickRelayId, buttonConfig.longClickRelayId,
        buttonConfig.doubleClickRelayId, buttonConfig.buttonDescription
      );
    }
    printf_P(PSTR("# Debug startup - EEPROM (first value is version, relay state starts at %i\n"), RELAY_STATE_STORAGE);
    printf_P(PSTR("# > "));
    for (int relayNum = 0; relayNum < gRelayConfigRef.size()+1; relayNum++) {
      printf("%i,", gEeprom.read(relayNum));
    }
    printf("\n");
    printf_P(PSTR("# Debug startup - buttons pin state\n"));
    printf_P(PSTR("# > "));
    for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
      pinMode(gConfiguration.getButtonPin(buttonNum), INPUT_PULLUP);
    }
    delay(200);
    for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
      printf("%i,",digitalRead(gConfiguration.getButtonPin(buttonNum)));
    }
    printf("\n");
  #endif

  if (PinCreator::instance() == nullptr) {
    Serial.println("PinCreator instance is NULL");
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

  // gButtonService.setup();
  for (size_t buttonNum = 0; buttonNum < gButtonConfigRef.size(); buttonNum++)
  {
    int clickActionRelayNum = gConfiguration.getRelayNum(gConfiguration.getButtonClickAction(buttonNum));
    gButtonService.setAction(buttonNum,
                            clickActionRelayNum,
                            gConfiguration.getRelayNum(gConfiguration.getButtonLongClickAction(buttonNum)),
                            gConfiguration.getRelayNum(gConfiguration.getButtonDoubleClickAction(buttonNum)));
    gButtonService.attachPin(buttonNum);
    if (((gConfiguration.getButtonType(buttonNum) & 0x0f) == REED_SWITCH) && (clickActionRelayNum > -1)) {
      gRelayService.reportAsSensor(clickActionRelayNum);
      gRelayService.changeState(clickActionRelayNum, gButtonService.getRelayState(buttonNum, false), millis());
    } else if (((gConfiguration.getButtonType(buttonNum) & 0x0f) == DING_DONG) && (clickActionRelayNum > -1)) {
      gRelayService.changeState(clickActionRelayNum, gButtonService.getRelayState(buttonNum, false), millis());
    }
  }

  if (versionChangeResetState) {
    // version has changed, so store new version in eeporom
    gEeprom.write(0, CONFIG_VERSION);
  }
}; // before()


// executed AFTER mysensors has been initialised
void setup()
{
  // Send state to MySensor Gateway
  for (size_t relayNum = 0; relayNum < gRelayConfigRef.size(); relayNum++) {
    myMessage.setSensor(gRelayService.getSensorId(relayNum));
    myMessage.setType(gRelayService.isSensor(relayNum) ? V_TRIPPED : V_STATUS);
    send(myMessage.set(gRelayService.getState(relayNum))); // send current state
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

  if (gRelayService.isImpulsePending()) {
    for (size_t relayNum = 0; relayNum < gRelayConfigRef.size(); relayNum++) {
      if (gRelayService.impulseProcess(relayNum, loopStartMillis)) {
        myMessage.setSensor(gRelayService.getSensorId(relayNum));
        send(myMessage.set(0));
      }
    }
  }

  for (size_t buttonNum = 0; buttonNum < gButtonConfigRef.size(); buttonNum++) {
    
    int relayNum = gButtonService.checkEvent(buttonNum, loopStartMillis);
    if (relayNum > -1) {
      // mono/bi-stable button toggles the relay, ding-dong/reed-switch switch to exact state
      bool relayState = gButtonService.getRelayState(buttonNum, gRelayService.getState(relayNum));

      #ifdef IGNORE_BUTTONS_START_MS
        if (loopStartMillis > IGNORE_BUTTONS_START_MS) {
      #endif
          if (gRelayService.changeState(relayNum, relayState, loopStartMillis)) {
            myMessage.setType(gRelayService.isSensor(relayNum) ? V_TRIPPED : V_STATUS);
            myMessage.setSensor(gRelayService.getSensorId(relayNum));
            send(myMessage.set(relayState));
          }
      #ifdef IGNORE_BUTTONS_START_MS
        }
      #endif
    }
  }

  #ifdef DEBUG_STATS
    if (debugStatsOn) {
      unsigned long loopIntervalCurrent = millis();
      loopCumulativeMillis += loopIntervalCurrent - loopStartMillis;
      loopCounter++;
      if ( loopCounter > DEBUG_STATS) {
        printf_P(
          PSTR("# %i loop stats: (end-start)=%lums, cumulative_loop_millis=%lums\n"),
          DEBUG_STATS, (loopIntervalCurrent - loopInterval), loopCumulativeMillis
        );
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
  sendSketchInfo(MULTI_RELAY_DESCRIPTION, MULTI_RELAY_VERSION );
  
  // Register every relay as separate sensor
  for (size_t relayNum = 0; relayNum < gRelayConfigRef.size(); relayNum++) {
    present(gRelayService.getSensorId(relayNum),
            gRelayService.isSensor(relayNum) ? S_DOOR : S_BINARY,
            gRelayService.getDescription(relayNum));
  }
};


// MySensors - Handling incoming messages
// Nodes that expects incoming data, must implement the receive() function to handle the incoming messages.
void receive(const MyMessage &message)
{
  #ifdef DEBUG_COMMUNICATION
    printf_P(
      PSTR("# Incoming message: sensorId=%i, command=%s, ack=%s, echo=%s, type=%s, payload=%s\n"),
      message.getSensor(), message.getCommand(), message.isAck(), message.getRequestEcho(), message.getType(), message.getString()
    );
  #endif
  if (message.getCommand() == C_SET) {
    if (message.getType() == V_STATUS) {
      int relayNum = gConfiguration.getRelayNum(message.getSensor());
      if (relayNum == -1) return;
      gRelayService.changeState(relayNum, message.getBool(), millis());
      if (! message.getRequestEcho()) {
        myMessage.setType(gRelayService.isSensor(relayNum) ? V_TRIPPED : V_STATUS);
        myMessage.setSensor(message.getSensor());
        send(myMessage.set(message.getBool())); // support for OPTIMISTIC=FALSE (Home Asistant)
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
        for (size_t relayNum = 0; relayNum < gRelayConfigRef.size(); relayNum++) {
          Serial.println(gRelayService.toString(relayNum));
        }
      } else if (debugCommand == 3) { // dump buttons state
        for (size_t buttonNum = 0; buttonNum < gButtonConfigRef.size(); buttonNum++) {
          Serial.println(String("# Button ") + buttonNum + ": " + gButtonService.toString(buttonNum));
        }
      } else if (debugCommand == 4) { // dump EEPROM
        Serial.print(F("# Dump EEPROM: "));
        for (int eepromIdx = 0; eepromIdx < gEeprom.length(); eepromIdx++) {
          Serial.print(gEeprom.dump(eepromIdx));
          Serial.print(",");
        }
        Serial.println();
      } else if (debugCommand == 5) { // clear EEPROM & reset
        gEeprom.clean();
        resetFunc();
      } else if (debugCommand == 6) { // reset
        resetFunc();
      } else if (debugCommand == 7) { // check relays state & eeprom
        // debug feature
        for (size_t relayNum = 0; relayNum < gRelayConfigRef.size(); relayNum++) {
          if ((gConfiguration.getRelayPin(relayNum) >= 0) &&
              (gRelayService.getState(relayNum) != (ArduinoPin::digitalRead(gConfiguration.getRelayPin(relayNum)) ==
                                                    (gConfiguration.getRelayOptions(relayNum) & RELAY_TRIGGER_HIGH))))
          {
            Serial.println(String("# Error state relay ") + gRelayService.getSensorId(relayNum) + ": state=" + gRelayService.getState(relayNum)
                          + ", pin_state=" + ArduinoPin::digitalRead(gConfiguration.getRelayPin(relayNum)));
          }
          if (((gConfiguration.getRelayOptions(relayNum) & RELAY_STARTUP_MASK) == 0) &&
              (gRelayService.getState(relayNum) != (gEeprom.read(RELAY_STATE_STORAGE + relayNum) == 1)))
          {
            Serial.println(String("# Error eeprom relay ") + gRelayService.getSensorId(relayNum) + ": state=" + gRelayService.getState(relayNum)
                          + ", eeprom=" + gEeprom.read(RELAY_STATE_STORAGE + relayNum));
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

Vector<const RelayConfigDef> gRelayConfigRef(gRelayConfig, sizeof(gRelayConfig) / sizeof(RelayConfigDef));
Vector<const ButtonConfigDef> gButtonConfigRef(gButtonConfig, sizeof(gButtonConfig) / sizeof(ButtonConfigDef));
Configuration gConfiguration(gRelayConfigRef, gButtonConfigRef);

// Eeprom gEeprom;
// RelayService gRelayService(gConfiguration, gEeprom);
// ButtonService gButtonService(gConfiguration, BUTTON_DEBOUNCE_INTERVAL);

int main( int argc, char **argv) {

  std::cout << "hello" << std::endl;

  // initialize relays
  // gRelayService.setImpulseInterval(RELAY_IMPULSE_INTERVAL);
  // gRelayService.initialize(false);

  // Setup buttons
  ButtonInterface::setEventIntervals(BUTTON_DOUBLE_CLICK_INTERVAL, BUTTON_LONG_PRESS_INTERVAL);
  MonoStableButton::clickTriggerWhenPressed(true);

  // Serial.println("# setup() - before button setup");
  // for (int buttonNum = 0; buttonNum < gButtonConfigRef.size(); buttonNum++)
  // {
  //   gButtonService.setAction(buttonNum,
  //                           gConfiguration.getRelayNum(gConfiguration.getButtonClickAction(buttonNum)),
  //                           gConfiguration.getRelayNum(gConfiguration.getButtonLongClickAction(buttonNum)),
  //                           gConfiguration.getRelayNum(gConfiguration.getButtonDoubleClickAction(buttonNum)));
  //   Serial.println("# setup() - setAction");
  //   gButtonService.attachPin(buttonNum);
  //   Serial.println("# setup() - attachPin");
  // }
  // Serial.println("# setup() - after button setup");
};
#endif
#endif
