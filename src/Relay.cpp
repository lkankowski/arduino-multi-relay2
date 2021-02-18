#include <Relay.h>
#include <Arduino.h>

using namespace lkankowski;

#ifdef DEBUG_STARTUP
  extern unsigned long debugCounter;
#endif

// static variables initialisation

#if defined(EXPANDER_PCF8574)
  PCF8574 * Relay::_expander = NULL;
#elif defined(EXPANDER_MCP23017)
  Adafruit_MCP23017 * Relay::_expander = NULL;
#endif


Relay::Relay()
    : _pin(0)
    , _state(false)
    , _sensorId(0)
    , _description(NULL)
    , _triggerState(0)
{};


void Relay::initialize(int sensorId, const char * description) {
    _sensorId = sensorId;
    _description = description;
};


void Relay::attachPin(int pin) {
    _pin = pin;

    // Then set relay pins in output mode
    #ifdef USE_EXPANDER
      if ( pin & 0xff00 ) {
        // EXPANDER
        int expanderNo = (pin >> 8) - 1;
        int expanderPin = pin & 0xff;
        _expander[expanderNo].pinMode(expanderPin, OUTPUT);
      } else {
    #endif
        pinMode(pin, OUTPUT);
    #ifdef USE_EXPANDER
      }
    #endif
};


bool Relay::changeState(bool state) {

    #ifdef DEBUG_STARTUP
      Serial.println(String("# ")+(debugCounter++)+":"+millis()+" Relay::changeState: old_state="+_state+", new_state="+state
                     +", _hasStartupOverride="+_hasStartupOverride+", _eepromIndex="+_eepromIndex
                     +", (uint8_t) state="+((uint8_t) state)+", _isImpulse="+_isImpulse);
    #endif
    bool stateHasChanged = state != _state;
    uint8_t digitalOutState = state ? _triggerState : ! _triggerState;

    #ifdef USE_EXPANDER
      if ( _pin & 0xff00 ) {
        int expanderNo = (_pin >> 8) - 1;
        int expanderPin = _pin & 0xff;
        _expander[expanderNo].digitalWrite(expanderPin, digitalOutState);
      } else {
    #endif
        digitalWrite(_pin, digitalOutState);
    #ifdef USE_EXPANDER
      }
    #endif

    _state = state;

    return(stateHasChanged);
};

