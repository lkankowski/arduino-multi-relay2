#include <ArduinoAbstract.h>

using namespace lkankowski;

#ifdef ARDUINO
#include <Arduino.h>

void Pin::pinMode(uint8_t pin, uint8_t val)
{
  ::pinMode(pin, val);
};

int Pin::digitalRead(uint8_t pin)
{
  return ::digitalRead(pin);
};

void Pin::digitalWrite(uint8_t pin, uint8_t val)
{
  ::digitalWrite(pin, val);
};

#else

void Pin::pinMode(uint8_t pin, uint8_t val)
{
  _mode[pin] = val;
//  if (val == INPUT_PULLUP) _state[pin] = HIGH;
};

int Pin::digitalRead(uint8_t pin)
{
  return _state[pin];
};

void Pin::digitalWrite(uint8_t pin, uint8_t val)
{
  _state[pin] = val;
};

uint8_t Pin::_mode[]  = {0,0,0,0,0,0,0,0,0,0};
uint8_t Pin::_state[] = {0,0,0,0,0,0,0,0,0,0};


/*
unsigned long millis(void)
{

};
*/

#endif

