#include <Relay.h>

using namespace lkankowski;

#ifdef DEBUG_STARTUP
  extern unsigned long debugCounter;
#endif


Relay::Relay(PinInterface& pin)
    : _pin(pin)
    , _state(false)
    , _sensorId(0)
    // , _description(NULL)
    , _triggerState(0)
{
};


Relay::~Relay()
{
  delete &_pin;
};


void Relay::initialize(int sensorId, const char * description)
{
  _sensorId = sensorId;
  _description = description;
};


void Relay::attachPin()
{
  _pin.pinMode(OUTPUT);
};


bool Relay::changeState(bool state)
{
  #ifdef DEBUG_STARTUP
    Serial.println(String("# ")+(debugCounter++)+":"+millis()+" Relay::changeState: old_state="+_state+", new_state="+state
    //               +", _hasStartupOverride="+_hasStartupOverride+", _eepromIndex="+_eepromIndex +", _isImpulse="+_isImpulse
                    +", (uint8_t) state="+((uint8_t) state));
  #endif
  bool stateHasChanged = state != _state;
  uint8_t digitalOutState = state ? _triggerState : ! _triggerState;

  _pin.digitalWrite(digitalOutState);

  _state = state;

  return(stateHasChanged);
};

