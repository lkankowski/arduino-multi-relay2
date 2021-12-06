#pragma once

#include <stdint.h>

#define xstr(a) str(a)
#define str( a ) (#a)

#ifdef ARDUINO
  #include <Arduino.h>

  #if defined(ARDUINO_ARCH_AVR)
    #define IS_VALID_DIGITAL_PIN(pin) ((pin >= 0) && (pin < NUM_DIGITAL_PINS))
  #elif defined (ARDUINO_ARCH_ESP8266)
    #define IS_VALID_DIGITAL_PIN(pin) (((pin >= 0) && (pin < NUM_DIGITAL_PINS)) && !isFlashInterfacePin(pin))
  #elif defined (ARDUINO_ARCH_ESP32)
    #define isFlashInterfacePin(pin)  ((pin) >= 6 && (pin) <= 11)
    #define IS_VALID_DIGITAL_PIN(pin) (digitalPinIsValid(pin) && !isFlashInterfacePin(pin) && \
                                       ((pin) != 20) && ((pin) != 37) && ((pin) != 38))
                                       //((pin) != 24) && !(((pin) >= 28) && (pin) <= 31) // already excluded in digitalPinIsValid()
                                       
  #endif

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
  #else
    // when no expander treat pins as Virtual
    #define E(expanderNo, ExpanderPin) (-(((expanderNo+1)<<8) | (ExpanderPin)))
  #endif

#else //!ARDUINO
  #include <string.h>
  #define F(x) (x)
  #define E(expanderNo, ExpanderPin) (((expanderNo+1)<<8) | (ExpanderPin))
#endif //ARDUINO


namespace lkankowski {

  class PinInterface
  {
    public:
      virtual ~PinInterface() {};

      virtual void pinMode(uint8_t mode) const = 0;
      virtual uint8_t digitalRead() const = 0;
      virtual void digitalWrite(uint8_t val) = 0;
  };


  class ArduinoPin : public PinInterface
  {
    public:
      explicit ArduinoPin(uint8_t);

      void pinMode(uint8_t mode) const override;
      uint8_t digitalRead() const override;
      void digitalWrite(uint8_t val) override;
      static uint8_t digitalRead(uint8_t);
      
    private:
      uint8_t _pin;
  };


  #ifdef EXPANDER_PCF8574

    class PCF8574Pin : public PinInterface
    {
      public:
        explicit PCF8574Pin(uint8_t, PCF8574&);

        void pinMode(uint8_t mode) const override;
        uint8_t digitalRead() const override;
        void digitalWrite(uint8_t val) override;
        
      private:
        uint8_t _pin;
        PCF8574& _expander;
    };
    
  #endif

  #ifdef EXPANDER_MCP23017

    class MCP23017Pin : public PinInterface
    {
      public:
        explicit MCP23017Pin(uint8_t, Adafruit_MCP23017&);

        void pinMode(uint8_t mode) const override;
        uint8_t digitalRead() const override;
        void digitalWrite(uint8_t val) override;
        
      private:
        uint8_t _pin;
        Adafruit_MCP23017& _expander;
    };
    
  #endif

  class VirtualPin : public PinInterface
  {
    public:
      VirtualPin(uint8_t);

      void pinMode(uint8_t mode) const override;
      uint8_t digitalRead() const override;
      void digitalWrite(uint8_t val) override;
      
    private:
      const uint8_t _pin;
      uint8_t _value;
  };


  #ifndef ARDUINO
  class FakePin : public PinInterface
  {
    public:
      FakePin(uint8_t);

      void pinMode(uint8_t) const override;
      uint8_t digitalRead() const override;
      void digitalWrite(uint8_t) override;
      
      static uint8_t _mode[10];
      static uint8_t _state[10];
    
    private:
      uint8_t _pin;
  };
  #endif

  class PinCreator
  {
    public:
      PinCreator();
      #ifdef EXPANDER_PCF8574
        PinCreator(PCF8574 * expander, const uint8_t * expanderAddresses, const uint8_t numberOfExpanders);
      #endif
      #ifdef EXPANDER_MCP23017
        PinCreator(Adafruit_MCP23017 * expander, const uint8_t * expanderAddresses, const uint8_t numberOfExpanders);
      #endif

      ~PinCreator();

      static PinCreator * instance();

      PinInterface * create(int pin);
      #ifdef USE_EXPANDER
        void initExpanders();
      #endif
    
    private:
      static PinCreator * _instance;
      #if defined(EXPANDER_PCF8574)
        PCF8574 * _expanders;
      #elif defined(EXPANDER_MCP23017)
        Adafruit_MCP23017 * _expanders;
      #endif
      #ifdef USE_EXPANDER
        const uint8_t * _expanderAddresses;
        uint8_t _numberOfExpanders;
      #endif
};

  void haltSystem();

  template <typename T> void PROGMEM_readAnything(const T * sce, T& dest) {
    #ifdef ARDUINO
      memcpy_P(&dest, sce, sizeof (T));
    #else
      memcpy(&dest, sce, sizeof (T));
    #endif
  };

  #ifdef ARDUINO
    template<class T> inline Print & operator <<(Print & obj, T arg) { obj.print(arg); return obj; };
  #endif

  uint32_t freeMemory();

} // namespace lkankowski


// only for native compilation (mainly tests)
#ifndef ARDUINO

//#include <WString.h>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define PROGMEM

unsigned long millis(void);
void delay(int);

class SerialClass
{
  public:
    void print(const char *);
    void print(int);
    void println(const char *);
    void println(int);
};
template<class T> inline SerialClass & operator <<(SerialClass & obj, T arg) { obj.print(arg); return obj; };

extern SerialClass Serial;


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
  #define NUM_DIGITAL_PINS  (70)
  #define PIN_WIRE_SDA      (20)
  #define PIN_WIRE_SCL      (21)
  #define SERIAL_PORT_RX    (0)
  #define SERIAL_PORT_TX    (1)
  #define LED_BUILTIN 13
#elif defined(BOARD_TARGET_ATMEGA328)
  #undef LED_BUILTIN
  #undef A0
  #undef A1
  #undef A2
  #undef A3
  #undef A4
  #undef A5
  #undef A6
  #undef A7
  #define A0                (14)
  #define A1                (15)
  #define A2                (16)
  #define A3                (17)
  #define A4                (18)
  #define A5                (19)
  #define A6                (20) //nano
  #define A7                (21) //nano
  #define NUM_DIGITAL_PINS  (20)
  #define PIN_WIRE_SDA      (18)
  #define PIN_WIRE_SCL      (19)
  #define SERIAL_PORT_RX    (0)
  #define SERIAL_PORT_TX    (1)
  #define LED_BUILTIN       (13)
#elif defined(BOARD_TARGET_ESP8266)
  #define D0                (16)
  #define D1                (5)
  #define D2                (4)
  #define D3                (0)
  #define D4                (2)
  #define D5                (14)
  #define D6                (12)
  #define D7                (13)
  #define D8                (15)
  #define D9                (3)
  #define D10               (1)
  #define A0                (17)
  #define NUM_DIGITAL_PINS  (17)
  #define PIN_WIRE_SDA      (4)
  #define PIN_WIRE_SCL      (5)
  #define SERIAL_PORT_RX    (3)
  #define SERIAL_PORT_TX    (1)
  #define LED_BUILTIN       (2)


  #define isFlashInterfacePin(p)      ((p) >= 6 && (p) <= 11)

#endif

#endif

