#pragma once

#include <ArduinoAbstract.h>

#if defined(EXPANDER_PCF8574)
  #include "PCF8574.h"
  #define USE_EXPANDER
 #elif defined(EXPANDER_MCP23017)
  #include "Adafruit_MCP23017.h"
  #define USE_EXPANDER
#endif

namespace lkankowski {


class Relay {

  public:
    Relay();
    void initialize(int, const char *);
    void attachPin(int);
    void setTriggerMode(uint8_t mode) { _triggerState = mode; };
    bool changeState(bool);
    bool getState() { return(_state); };
    int getSensorId() { return(_sensorId); }; //TODO: refactor: move to RelayService
    const char * getDescription() { return(_description); };
    #if defined(EXPANDER_PCF8574)
      static void expanderInit(PCF8574 * exp) { Relay::_expander = exp; };
    #elif defined(EXPANDER_MCP23017)
      static void expanderInit(Adafruit_MCP23017 * exp) { _expander = exp; };
    #endif

  private:
    int _pin;
    Pin _pinObj;
    bool _state; // true/false = ON/OFF
    int _sensorId;
    const char * _description;
    uint8_t _triggerState;
    #if defined(EXPANDER_PCF8574)
      static PCF8574 * _expander;
    #elif defined(EXPANDER_MCP23017)
      static Adafruit_MCP23017 * _expander;
    #endif
};

} //namespace
