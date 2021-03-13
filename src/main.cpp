#include <Arduino.h>
#include <assert.h>
#include <EEPROM.h>
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
lkankowski::Button gButton[gNumberOfButtons];

void(* resetFunc) (void) = 0; //declare reset function at address 0

FILE serial_stdout;

// MySensors - This will execute before MySensors starts up
void before() {
  Serial.begin(115200);

   // Set up redirect of stdout to serial
   fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
   stdout = &serial_stdout;
  #ifdef DEBUG_STARTUP
    printf_P(
      PSTR("# %lu Debug startup - common config: MONO_STABLE_TRIGGER=%i, RELAY_IMPULSE_INTERVAL=%lu, BUTTON_DEBOUNCE_INTERVAL=%lu, BUTTON_DOUBLE_CLICK_INTERVAL=%lu, BUTTON_LONG_PRESS_INTERVAL=%lu, MULTI_RELAY_VERSION=%s\n"),
      debugCounter++, MONO_STABLE_TRIGGER, RELAY_IMPULSE_INTERVAL, BUTTON_DEBOUNCE_INTERVAL, BUTTON_DOUBLE_CLICK_INTERVAL,
      BUTTON_LONG_PRESS_INTERVAL, MULTI_RELAY_VERSION
    );

    #ifdef USE_EXPANDER
      printf_P(PSTR("# %lu Debug startup - expander config\n"), debugCounter++);
      for(int i = 0; i < gNumberOfExpanders; i++) {
        printf("%i,",expanderAddresses[i]);
      }
      printf("\n");
    #endif

    printf_P(PSTR("# %lu Debug startup - relay config\n"), debugCounter++);
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      RelayConfigDef relayConfig = {};
      PROGMEM_readAnything(&gRelayConfig[relayNum], relayConfig);
      printf_P(
        PSTR("# %lu > %i;%i;%i;%s;\n"), debugCounter++, relayConfig.sensorId,
        relayConfig.relayPin, relayConfig.relayOptions, relayConfig.relayDescription
      );

    }
    printf_P(PSTR("# %lu Debug startup - button config\n"), debugCounter++);
    for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
      ButtonConfigDef buttonConfig = {};
      PROGMEM_readAnything(&gButtonConfig[buttonNum], buttonConfig);
      printf_P(
        PSTR("# %lu > %i;%i;%i;%i;%i;%s;\n"), debugCounter++, buttonConfig.buttonPin,
        buttonConfig.buttonType, buttonConfig.clickRelayId, buttonConfig.longClickRelayId,
        buttonConfig.doubleClickRelayId, buttonConfig.buttonDescription
      );
    }
    printf_P(PSTR("# %lu Debug startup - EEPROM (first value is version, relay state starts at %i\n"), debugCounter++, RELAY_STATE_STORAGE);
    printf_P(PSTR("# %lu > "), debugCounter++);
    for (int relayNum = 0; relayNum < gNumberOfRelays+1; relayNum++) {
      printf("%i,", EEPROM.read(relayNum));
    }
    printf("\n");
    printf_P(PSTR("# %lu Debug startup - buttons pin state\n"), debugCounter++);
    printf_P(PSTR("# %lu > "), debugCounter++);
    for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
      pinMode(gButtonConfig[buttonNum].buttonPin, INPUT_PULLUP);
    }
    delay(200);
    for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
      printf("%i,",digitalRead(gButtonConfig[buttonNum].buttonPin));
    }
    printf("\n");
  #endif

  // validate config
  #ifdef USE_EXPANDER
    //TODO: check if I2C pins are not used
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      RelayConfigDef relayConfig = {};
      PROGMEM_readAnything(&gRelayConfig[relayNum], relayConfig);

      int pin = relayConfig.relayPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)) {
          printf_P(PSTR("Configuration failed - expander no or number of pins out of range for relay: %i\n"), relayNum);
          delay(1000);
          assert(0);
        }
      }
    }
  #endif
  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
    ButtonConfigDef buttonConfig = {};
    PROGMEM_readAnything(&gButtonConfig[buttonNum], buttonConfig);

    #ifdef USE_EXPANDER
      int pin = buttonConfig.buttonPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)) {
          printf_P(PSTR("Configuration failed - expander no or number of pins out of range for button: %i\n"), buttonNum);
          delay(1000);
          assert(0);
        }
      }
    #endif
    const char * failAction[] = {"OK", "click", "long-press", "double-click"};
    int fail = 0;

    if ((buttonConfig.clickRelayId != -1) && (getRelayNum(buttonConfig.clickRelayId) == -1)) fail = 1;
    if ((buttonConfig.longClickRelayId != -1) && (getRelayNum(buttonConfig.longClickRelayId) == -1)) fail = 2;
    if ((buttonConfig.doubleClickRelayId != -1) && (getRelayNum(buttonConfig.doubleClickRelayId) == -1)) fail = 3;
    if (fail) {
        printf_P(PSTR("Configuration failed - invalid '%s relay ID' for button: %i\n"), failAction[fail], buttonNum);
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
  Relay::setImpulseInterval(RELAY_IMPULSE_INTERVAL);

  for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
    RelayConfigDef relayConfig = {};
    PROGMEM_readAnything(&gRelayConfig[relayNum], relayConfig);

    gRelay[relayNum].initialize(relayNum, relayConfig.sensorId, relayConfig.relayDescription);
    gRelay[relayNum].attachPin(relayConfig.relayPin);
    gRelay[relayNum].setModeAndStartupState(relayConfig.relayOptions, versionChangeResetState);
    gRelay[relayNum].start();
  }
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
    ButtonConfigDef buttonConfig = {};
    PROGMEM_readAnything(&gButtonConfig[buttonNum], buttonConfig);

    gButton[buttonNum].initialize(buttonConfig.buttonType, buttonConfig.buttonDescription);
    gButton[buttonNum].setAction(getRelayNum(buttonConfig.clickRelayId),
                                 getRelayNum(buttonConfig.longClickRelayId),
                                 getRelayNum(buttonConfig.doubleClickRelayId));
    gButton[buttonNum].setDebounceInterval(BUTTON_DEBOUNCE_INTERVAL);
    gButton[buttonNum].attachPin(buttonConfig.buttonPin);
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
          if (gRelay[relayNum].changeState(relayState)) {
            myMessage.setSensor(gRelay[relayNum].getSensorId());
            send(myMessage.set(relayState));
          }
      #ifdef IGNORE_BUTTONS_START_MS
        }
      #endif
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
    printf_P(
      PSTR("# Incoming message: sensorId=%i, command=%s, ack=%s, echo=%s, type=%s, payload=%s\n"),
      message.getSensor(), message.getCommand(), message.isAck(), message.isEcho(), message.getType(), message.getString()
    );
  #endif
  if (message.getCommand() == C_SET) {
    if (message.getType() == V_STATUS) {
      int relayNum = getRelayNum(message.getSensor());
      if (relayNum == -1) return;
      gRelay[relayNum].changeState(message.getBool());
      myMessage.setSensor(message.getSensor());
      send(myMessage.set(message.getBool())); // support for OPTIMISTIC=FALSE (Home Asistant)
    #ifdef DEBUG_STATS
    } else if (message.getType() == V_VAR1) {
      int debugCommand = message.getInt();
      if (debugCommand == 1) {
        debugStatsOn = message.getBool();
        loopCounter = 0;
        send(debugMessage.set(F("toogle debug stats")));
      } else if (debugCommand == 2) {
        for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
          printf_P(
            PSTR("# Sensor %i state=%i; %s\n"),
            gRelay[relayNum].getSensorId(),  gRelay[relayNum].getState(), gRelay[relayNum].getDescription()
          );
        }
      } else if (debugCommand == 3) {
        for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
          printf_P(PSTR("# Button %i: %s\n"), buttonNum, gButton[buttonNum]);
        }
      } else if (debugCommand == 4) { // dump EEPROM
        printf_P(PSTR("# Dump EEPROM: "));
        for (int relayNum = 0; relayNum < gNumberOfRelays+RELAY_STATE_STORAGE; relayNum++) {
          printf("%i,",EEPROM.read(relayNum));
        }
        printf("\n");
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
