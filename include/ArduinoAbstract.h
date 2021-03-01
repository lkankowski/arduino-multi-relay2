#pragma once

#include <stdint.h>
#ifdef ARDUINO
#include <Arduino.h>
#endif


namespace lkankowski {

  class PinInterface
  {
  public:
    virtual void pinMode(uint8_t pin, uint8_t mode) = 0;
    virtual int digitalRead(uint8_t pin) = 0;
    virtual void digitalWrite(uint8_t pin, uint8_t val) = 0;
  };


  class Pin : public PinInterface
  {
  public:
    void pinMode(uint8_t pin, uint8_t val);
    int digitalRead(uint8_t pin);
    void digitalWrite(uint8_t pin, uint8_t val);
    
    #ifndef ARDUINO
      static uint8_t _mode[10];
      static uint8_t _state[10];
    #endif
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

