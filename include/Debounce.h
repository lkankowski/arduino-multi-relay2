#include <ArduinoAbstract.h>
#include <Bounce2.h>
#include <stdint.h>


#if defined(EXPANDER_PCF8574)
  #include "PCF8574.h"
  #define USE_EXPANDER
#elif defined(EXPANDER_MCP23017)
  #include "Adafruit_MCP23017.h"
  #define USE_EXPANDER
#endif

namespace lkankowski {

class Debounce : public Bounce
{
  public:
    #if defined(EXPANDER_PCF8574)
      static void setExpander(PCF8574 * exp) { _expander = exp; };
    #elif defined(EXPANDER_MCP23017)
      static void setExpander(Adafruit_MCP23017 * exp) { _expander = exp; };
    #endif

    void attach(int pin)
    {
      this->pin = pin; // type changed from uint8_t to uint16_t
      
      // SET INITIAL STATE
      begin();
    };

    void attach(int pin, int mode)
    {
      setPinMode(pin, mode);
      this->attach(pin);
    };

  protected:
    bool readCurrentState()
    {
      int result;
      #ifdef USE_EXPANDER
        if (pin & 0xff00) { // expander
          int expanderNo = (pin >> 8) - 1;
          int expanderPin = pin & 0xff;
          result = _expander[expanderNo].digitalRead(expanderPin);
        } else {
      #endif
          result = _pinObj.digitalRead(pin);
      #ifdef USE_EXPANDER
        }
      #endif
      return result;
    }

    void setPinMode(int _pin, int mode)
    {
      #ifdef USE_EXPANDER
        if (_pin & 0xff00) { // expander
          int expanderNo = (_pin >> 8) - 1;
          int expanderPin = _pin & 0xff;
          #if defined(EXPANDER_MCP23017)
            // https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library/blob/1.2.0/Adafruit_MCP23017.cpp#L145
            // pinMode method doesn't support INPUT_PULLUP, workaround below:
            if (mode == INPUT_PULLUP) {
              _expander[expanderNo].pinMode(expanderPin, INPUT);
              _expander[expanderNo].pullUp(expanderPin, HIGH);
            } else {
              _expander[expanderNo].pinMode(expanderPin, mode);
            }
          #else
            _expander[expanderNo].pinMode(expanderPin, mode);
          #endif
        } else {
      #endif
          _pinObj.pinMode(_pin, mode);
      #ifdef USE_EXPANDER
        }
      #endif
    }
    
    #if defined(EXPANDER_PCF8574)
      static PCF8574 * _expander;
    #elif defined(EXPANDER_MCP23017)
      static Adafruit_MCP23017 * _expander;
    #endif

    uint16_t pin;
    ArduinoPin _pinObj;
};

} // namespace lkankowski
