#include <ArduinoAbstract.h>

using namespace lkankowski;

#ifdef ARDUINO
#include <Arduino.h>

#ifdef USE_EXPANDER
  const int gNumberOfExpanders = sizeof(gExpanderAddresses);
  #if defined(PCF8574_H)
    PCF8574 gExpander[gNumberOfExpanders];
  #elif defined(_Adafruit_MCP23017_H_)
    Adafruit_MCP23017 gExpander[gNumberOfExpanders];
  #endif
#endif


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

uint8_t ArduinoPin::digitalRead(uint8_t pin)
{
  return ::digitalRead(pin);
};


#ifdef EXPANDER_PCF8574
PCF8574Pin::ArduinoPin(uint8_t pin)
{
  _pin = pin;
};

void PCF8574Pin::pinMode(uint8_t val) const
{
  ::pinMode(_pin, val);
};

int PCF8574Pin::digitalRead() const
{
  return ::digitalRead(_pin);
};

void PCF8574Pin::digitalWrite(uint8_t val) const
{
  ::digitalWrite(_pin, val);
};
#endif


PinInterface * PinCreator::create(int pin)
{
  if (pin < 0x100 ) return new ArduinoPin(pin);
  #ifdef EXPANDER_PCF8574
    uint8_t expanderNo = (pin >> 8) - 1;
    return new PCF8574Pin(pin&0xff, );

    // Expander:
    // - instance
    // - begin()
    // - pinMode
    // - digital Read/Write
  #endif
  return nullptr;
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


PinInterface * PinCreator::create(int pin)
{
  return new FakePin(pin);
};

uint8_t FakePin::_mode[]  = {0,0,0,0,0,0,0,0,0,0};
uint8_t FakePin::_state[] = {0,0,0,0,0,0,0,0,0,0};


/*
unsigned long millis(void)
{

};
*/

void delay(int) {};

#include <iostream>

void SerialClass::print(const char * s) {
  std::cout << s;
};
void SerialClass::print(int s) {
  std::cout << s;
};
void SerialClass::println(const char * s) {
  std::cout << s << std::endl;
};
void SerialClass::println(int s) {
  std::cout << s << std::endl;
};

SerialClass Serial;


#endif

