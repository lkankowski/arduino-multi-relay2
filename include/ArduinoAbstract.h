#pragma once

#include <stdint.h>

#ifdef ARDUINO
#include <Arduino.h>

#if defined(EXPANDER_PCF8574) || defined(EXPANDER_MCP23017)
  #if defined(EXPANDER_PCF8574)
    #include "PCF8574.h"
    #define EXPANDER_PINS 8
  #elif defined(EXPANDER_MCP23017)
    #include "Adafruit_MCP23017.h"
    #define EXPANDER_PINS 16
  #endif
  #define USE_EXPANDER
  #include <Wire.h>    // Required for I2C communication
  #define E(expanderNo, ExpanderPin) (((expanderNo+1)<<8) | (ExpanderPin))
#endif

#ifdef USE_EXPANDER
  const int gNumberOfExpanders = sizeof(expanderAddresses);
  #if defined(PCF8574_H)
    PCF8574 gExpander[gNumberOfExpanders];
  #elif defined(_Adafruit_MCP23017_H_)
    Adafruit_MCP23017 gExpander[gNumberOfExpanders];
  #endif
#endif

#endif //ARDUINO


namespace lkankowski {

  class PinInterface
  {
    public:
      virtual ~PinInterface() {};

      virtual void pinMode(uint8_t mode) const = 0;
      virtual int digitalRead() const = 0;
      virtual void digitalWrite(uint8_t val) const = 0;
  };


  class ArduinoPin : public PinInterface
  {
    public:
      ArduinoPin(uint8_t);

      void pinMode(uint8_t mode) const;
      int digitalRead() const;
      void digitalWrite(uint8_t val) const;
      
    private:
      int _pin;
  };

  #ifndef ARDUINO
  class FakePin : public PinInterface
  {
    public:
      FakePin(uint8_t);

      void pinMode(uint8_t mode) const;
      int digitalRead() const;
      void digitalWrite(uint8_t val) const;
      
      static uint8_t _mode[10];
      static uint8_t _state[10];
    
    private:
      int _pin;
  };
  #endif

  class PinCreator
  {
    public:
      static PinInterface& create(int pin);
    
    private:
      PinCreator() {};
  };

} // namespace lkankowski


#ifndef ARDUINO

#include <WString.h>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

unsigned long millis(void);

#if defined(BOARD_TARGET_ATMEGA2560)
  #undef LED_BUILTIN
  #undef A0
  #undef A1
  #undef A2
  #undef A3
  #undef A4
  #undef A5
  #undef A6
  #undef A7
  #undef A8
  #undef A9
  #define A0   (54)
  #define A1   (55)
  #define A2   (56)
  #define A3   (57)
  #define A4   (58)
  #define A5   (59)
  #define A6   (60)
  #define A7   (61)
  #define A8   (62)
  #define A9   (63)
  #define A10  (64)
  #define A11  (65)
  #define A12  (66)
  #define A13  (67)
  #define A14  (68)
  #define A15  (69)
  #define NUM_DIGITAL_PINS            70
  #define PIN_WIRE_SDA        (20)
  #define PIN_WIRE_SCL        (21)
  #define SERIAL_PORT_RX        (0)
  #define SERIAL_PORT_TX        (1)
  #define LED_BUILTIN 13
#endif

#endif

