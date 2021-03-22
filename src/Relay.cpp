#include <Relay.h>
#include <Arduino.h>

void saveState(const uint8_t pos, const uint8_t value);
uint8_t loadState(const uint8_t pos);

using namespace lkankowski;

#ifdef DEBUG_STARTUP
  extern unsigned long debugCounter;
#endif

// static variables initialisation
int Relay::_impulsePending = 0;
unsigned long Relay::_impulseInterval = 250;

#if defined(EXPANDER_PCF8574)
  PCF8574 * Relay::_expander = NULL;
#elif defined(EXPANDER_MCP23017)
  Adafruit_MCP23017 * Relay::_expander = NULL;
#endif


Relay::Relay()
    : _pin(0)
    , _state(false)
    , _eepromIndex(0)
    , _sensorId(0)
    , _description(NULL)
    , _hasStartupOverride(false)
    , _triggerState(0)
    , _isImpulse(false)
    , _impulseStartMillis(0)
{};


void Relay::initialize(int index, int sensorId, const char * description) {
    _eepromIndex = index;
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


void Relay::setModeAndStartupState(int mode, bool resetState) {

    _triggerState = mode & RELAY_TRIGGER_HIGH;

    if (mode & RELAY_IMPULSE) {
      _isImpulse = true;
      _state = false;
      _hasStartupOverride = true;
    } else if (mode & RELAY_STARTUP_ON) {
      _state = true;
      _hasStartupOverride = true;
    } else if (mode & RELAY_STARTUP_OFF) {
      _state = false;
      _hasStartupOverride = true;
    } else {
        // Set relay to last known state (using eeprom storage)
        _state = loadState(RELAY_STATE_STORAGE + _eepromIndex) == 1; // 1 - true, 0 - false
        if (resetState && _state) {
            saveState(RELAY_STATE_STORAGE + _eepromIndex, 0);
            _state = false;
      }
    }
};


bool Relay::changeState(bool state) {

    #ifdef DEBUG_STARTUP
      printf_P(
        PSTR("# %lu:%lu Relay::changeState: old_state=%d, new_state=%d, _hasStartupOverride=%d, _eepromIndex=%i, (uint8_t) state=%d, _isImpulse=%d\n"),
        debugCounter++, millis(), _state, state, _hasStartupOverride, _eepromIndex, (uint8_t) state, _isImpulse
      );
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

    if (! _hasStartupOverride && stateHasChanged) {
        saveState(RELAY_STATE_STORAGE + _eepromIndex, (uint8_t) state);
    }

    if (_isImpulse && stateHasChanged) {
      if (state) {
        _impulseStartMillis = millis();
        _impulsePending++;
      } else {
        _impulseStartMillis = 0;
        _impulsePending--;
      }
    }

    _state = state;

    return(stateHasChanged);
};


bool Relay::impulseProcess() {

  if (_isImpulse && _impulseStartMillis > 0) {
    unsigned long currentMillis = millis();

    // the "|| (currentMillis < myRelayImpulseStart[i])" is for "millis()" overflow protection
    if ((currentMillis > _impulseStartMillis+_impulseInterval) || (currentMillis < _impulseStartMillis)) {
      return(changeState(false));
    }
  }
  return(false);
};
