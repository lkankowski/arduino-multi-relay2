#ifndef Relay_h
#define Relay_h

#include <stdint.h>

#if defined(EXPANDER_PCF8574)
  #include "PCF8574.h"
  #define USE_EXPANDER
 #elif defined(EXPANDER_MCP23017)
  #include "Adafruit_MCP23017.h"
  #define USE_EXPANDER
#endif

namespace lkankowski {

const uint8_t RELAY_TRIGGER_LOW  = 0;
const uint8_t RELAY_TRIGGER_HIGH = 1;
const uint8_t RELAY_STARTUP_ON   = 2;
const uint8_t RELAY_STARTUP_OFF  = 4;
const uint8_t RELAY_IMPULSE      = 8;
const uint8_t RELAY_STARTUP_MASK = RELAY_STARTUP_ON | RELAY_STARTUP_OFF;

typedef struct {
  int sensorId;
  int relayPin;
  uint8_t relayOptions;
  const char * relayDescription;
} RelayConfigDef;

#define RELAY_STATE_STORAGE 1


class Relay {

  public:
    Relay();
    void initialize(int, int, const char *);
    static void setImpulseInterval(unsigned long impulseInterval) { _impulseInterval = impulseInterval; };
    void attachPin(int);
    void setModeAndStartupState(int, bool);
    void start() { changeState(_state); };
    bool changeState(bool);
    bool getState() { return(_state); };
    int getSensorId() { return(_sensorId); };
    static bool isImpulsePending() { return(_impulsePending > 0); };
    bool impulseProcess();
    void reportAsSensor() { _reportAsSensor = true; };
    bool isSensor() { return _reportAsSensor; };
    const char * getDescription() { return(_description); };
    #if defined(EXPANDER_PCF8574)
      static void expanderInit(PCF8574 * exp) { Relay::_expander = exp; };
    #elif defined(EXPANDER_MCP23017)
      static void expanderInit(Adafruit_MCP23017 * exp) { _expander = exp; };
    #endif

  private:
    int _pin;
    bool _state; // true/false = ON/OFF
    int _eepromIndex;
    int _sensorId;
    const char * _description;
    bool _hasStartupOverride;
    uint8_t _triggerState;
    bool _isImpulse;
    unsigned long _impulseStartMillis;
    static int _impulsePending;
    static unsigned long _impulseInterval;
    bool _reportAsSensor;
    #if defined(EXPANDER_PCF8574)
      static PCF8574 * _expander;
    #elif defined(EXPANDER_MCP23017)
      static Adafruit_MCP23017 * _expander;
    #endif
};

} //namespace

#endif
