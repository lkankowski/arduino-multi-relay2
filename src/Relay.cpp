#include <Relay.h>

using namespace lkankowski;


Relay::Relay(PinInterface * pin)
  : _pin(pin)
  , _state(false)
  , _triggerState(0)
{};


Relay::~Relay()
{};


void Relay::attachPin()
{
  _pin->pinMode(OUTPUT);
};


bool Relay::changeState(bool state)
{
  bool stateHasChanged = state != _state;
  uint8_t digitalOutState = state ? _triggerState : ! _triggerState;

  _pin->digitalWrite(digitalOutState);

  _state = state;

  return(stateHasChanged);
};

