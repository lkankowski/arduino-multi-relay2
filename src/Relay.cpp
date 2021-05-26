#include <Relay.h>

using namespace lkankowski;


Relay::Relay(PinInterface * pin)
  : _pin(pin)
  , _state(false)
  , _sensorId(0)
  , _description(NULL)
  , _triggerState(0)
  , _reportAsSensor(false)
{};


Relay::~Relay()
{};


void Relay::initialize(int sensorId, const char * description)
{
  _sensorId = sensorId;
  _description = description;
};


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

