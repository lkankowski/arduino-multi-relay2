#include <ArduinoAbstract.h>

using namespace lkankowski;

#ifdef ARDUINO
#include <Arduino.h>

ArduinoPin::ArduinoPin(uint8_t pin)
{
  _pin = pin;
};

void ArduinoPin::pinMode(uint8_t val) const
{
  ::pinMode(_pin, val);
};

int ArduinoPin::digitalRead() const
{
  return ::digitalRead(_pin);
};

void ArduinoPin::digitalWrite(uint8_t val) const
{
  ::digitalWrite(_pin, val);
};


PinInterface& PinCreator::create(int pin)
{
//  PinInterface* pinObj = new ArduinoPin();
  return *(new ArduinoPin(pin));
};


#else

FakePin::FakePin(uint8_t pin)
{
  _pin = pin;
};

void FakePin::pinMode(uint8_t val) const
{
  _mode[_pin] = val;
//  if (val == INPUT_PULLUP) _state[pin] = HIGH;
};

int FakePin::digitalRead() const
{
  return _state[_pin];
};

void FakePin::digitalWrite(uint8_t val) const
{
  _state[_pin] = val;
};


PinInterface& PinCreator::create(int pin)
{
  return *(new FakePin(pin));
};

uint8_t FakePin::_mode[]  = {0,0,0,0,0,0,0,0,0,0};
uint8_t FakePin::_state[] = {0,0,0,0,0,0,0,0,0,0};


/*
unsigned long millis(void)
{

};
*/

#endif

