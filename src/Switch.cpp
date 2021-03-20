#include <Switch.h>

using namespace lkankowski;


HardwareSwitchInterface * HardwareSwitchInterface::create(int type, int pin, unsigned int debounceInterval, uint8_t stateForPressed)
{
  return new DebouncedSwitch(pin, debounceInterval, stateForPressed);
};


DebouncedSwitch::DebouncedSwitch(int pin, unsigned int debounceInterval, uint8_t stateForPressed)
  : _pin(PinCreator::create(pin))
  , _debounceInterval(debounceInterval)
  , _previousMillis(0)
  , _unstableState(false)
  , _debouncedState(false)
  , _stateForPressed(stateForPressed)
{};


DebouncedSwitch::~DebouncedSwitch()
{
  delete _pin;
};


void DebouncedSwitch::attachPin()
{
  _pin->pinMode(INPUT_PULLUP);
};


bool DebouncedSwitch::update(unsigned long millis)
{
  bool currentState = _pin->digitalRead() == _stateForPressed;

  if ( currentState != _unstableState )
  {
    _unstableState =  currentState;      //! _unstableState;
    _previousMillis = millis;
  }
  else if ( millis - _previousMillis >= _debounceInterval )
  {
    if (currentState != _debouncedState )
    {
      _debouncedState = currentState;  // ! _debouncedState;
      _previousMillis = millis;
      return true;
    }
  }
  return false;
};


bool DebouncedSwitch::getState() const
{
  return _debouncedState;
};
