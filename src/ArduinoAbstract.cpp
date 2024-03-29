#include <ArduinoAbstract.h>
#include <assert.h>

using namespace lkankowski;


// static
PinCreator * PinCreator::_instance = nullptr;

PinCreator::PinCreator()
{
  _instance = this;
};


PinCreator::~PinCreator()
{
  _instance = nullptr;
};


// static
PinCreator * PinCreator::instance()
{
  // commented out for avoiding dynamic memory allocation
  // if (_instance == nullptr) {
  //   _instance == new PinCreator();
  // }
  return _instance;
};

VirtualPin::VirtualPin(uint8_t pin)
  : _pin(pin)
  , _value(1)
{};

void VirtualPin::pinMode(uint8_t val) const
{};

uint8_t VirtualPin::digitalRead() const
{
  return _value;
};

void VirtualPin::digitalWrite(uint8_t value)
{
  _value = value;
};


#ifdef ARDUINO
  #include <Arduino.h>


  ArduinoPin::ArduinoPin(uint8_t pin)
  {
    _pin = pin;
  };

  void ArduinoPin::pinMode(uint8_t mode) const
  {
    ::pinMode(_pin, mode);
  };

  uint8_t ArduinoPin::digitalRead() const
  {
    return ::digitalRead(_pin);
  };

  void ArduinoPin::digitalWrite(uint8_t val)
  {
    ::digitalWrite(_pin, val);
  };

  uint8_t ArduinoPin::digitalRead(uint8_t pin)
  {
    return ::digitalRead(pin);
  };


  #ifdef EXPANDER_PCF8574
  PCF8574Pin::PCF8574Pin(uint8_t pin, PCF8574& expander)
    : _pin(pin)
    , _expander(expander)
  {};

  void PCF8574Pin::pinMode(uint8_t mode) const
  {
    _expander.pinMode(_pin, mode);
  };

  uint8_t PCF8574Pin::digitalRead() const
  {
    return _expander.digitalRead(_pin);
  };

  void PCF8574Pin::digitalWrite(uint8_t val)
  {
    _expander.digitalWrite(_pin, val);
  };
  #endif


  #ifdef EXPANDER_MCP23017
  MCP23017Pin::MCP23017Pin(uint8_t pin, Adafruit_MCP23017& expander)
    : _pin(pin)
    , _expander(expander)
  {};


  void MCP23017Pin::pinMode(uint8_t mode) const
  {
    if (mode == INPUT_PULLUP) {
      _expander.pinMode(_pin, INPUT);
      _expander.pullUp(_pin, HIGH);
    } else {
      _expander.pinMode(_pin, mode);
    }
  };


  uint8_t MCP23017Pin::digitalRead() const
  {
    return _expander.digitalRead(_pin);
  };


  void MCP23017Pin::digitalWrite(uint8_t val)
  {
    _expander.digitalWrite(_pin, val);
  };
  #endif


    #ifdef EXPANDER_PCF8574
  PinCreator::PinCreator(PCF8574 * expander, const uint8_t * expanderAddresses, const uint8_t numberOfExpanders)
    : _expanders(expander)
    , _expanderAddresses(expanderAddresses)
    , _numberOfExpanders(numberOfExpanders)
  {
    _instance = this;
  };
  #endif


  #ifdef EXPANDER_MCP23017
  PinCreator::PinCreator(Adafruit_MCP23017 * expander, const uint8_t * expanderAddresses, const uint8_t numberOfExpanders)
    : _expanders(expander)
    , _expanderAddresses(expanderAddresses)
    , _numberOfExpanders(numberOfExpanders)
  {
    _instance = this;
  };
  #endif


  PinInterface * PinCreator::create(int pin)
  {
    if (pin < 0) return new VirtualPin(pin);
    if (pin < 0x100) {
      if (IS_VALID_DIGITAL_PIN(pin)) {
        return new ArduinoPin(pin);
      } else {
        Serial << "Warning: Invalid pin! Creating VirtualPin: " << pin << "\n";
        return new VirtualPin(pin);
      }
    }
    #ifdef EXPANDER_PCF8574
      uint8_t expanderNo = (pin >> 8) - 1;
      return new PCF8574Pin(pin&0xff, _expanders[expanderNo]);
    #endif
    #ifdef EXPANDER_MCP23017
      uint8_t expanderNo = (pin >> 8) - 1;
      return new MCP23017Pin(pin&0xff, _expanders[expanderNo]);
    #endif
    return nullptr;
  };


  #ifdef USE_EXPANDER
  void PinCreator::initExpanders()
  {
    for(int i = 0; i < _numberOfExpanders; i++) {
      _expanders[i].begin(_expanderAddresses[i]);
    }
  };
  #endif


  // Function that printf and related will use to print
  int serial_putchar(char c, FILE* f)
  {
    if (c == '\n') serial_putchar('\r', f);
    return Serial.write(c) == 1? 0 : 1;
  };


  void lkankowski::haltSystem()
  {
    delay(1000);
    assert(0);
  };


  // https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
  #if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32)
    extern int * __brkval;
  #endif

  uint32_t lkankowski::freeMemory()
  {
    #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
      return ESP.getFreeHeap();
    #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
      int top;
      return &top - __brkval;
    #else
      int top;
      return __brkval ? &top - __brkval : &top - __malloc_heap_start;
    #endif
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

  uint8_t FakePin::digitalRead() const
  {
    return _state[_pin];
  };

  void FakePin::digitalWrite(uint8_t val)
  {
    _state[_pin] = val;
  };


  PinInterface * PinCreator::create(int pin)
  {
    return new FakePin(pin);
  };

  uint8_t FakePin::_mode[]  = {0,0,0,0,0,0,0,0,0,0};
  uint8_t FakePin::_state[] = {0,0,0,0,0,0,0,0,0,0};


  //void delay(int) {};

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


  void lkankowski::haltSystem()
  {
  };


#endif

