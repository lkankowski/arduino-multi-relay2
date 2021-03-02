#ifndef Button_h
#define Button_h

#if defined(UNIX_HOST_DUINO_VERSION)
  class Bounce {
  public:
    void interval(uint16_t interval_millis);
    bool read();
  };
#else
  #include <Bounce2.h>
#endif
#include <stdint.h>

#if defined(EXPANDER_PCF8574)
  #include "PCF8574.h"
  #define USE_EXPANDER
#elif defined(EXPANDER_MCP23017)
  #include "Adafruit_MCP23017.h"
  #define USE_EXPANDER
#else
  #define BounceExp Bounce
#endif

namespace lkankowski {

enum ButtonType {
  MONO_STABLE = 0,
  BI_STABLE = 1,
  DING_DONG = 2, // HIGH state immediatly after push, LOW state after release
  REED_SWITCH = 3, // magnetic sensor for door or window, LOW - closed, HIGH - opened
  PRESSED_STATE_HIGH = 0x10
};

enum ButtonState {
  BTN_STATE_INITIAL,
  BTN_STATE_1ST_PRESS,
  BTN_STATE_1ST_RELEASE,
  BTN_STATE_2ND_PRESS,
  BTN_STATE_RELEASE_WAIT,
  BTN_STATE_1ST_CHANGE_BI
};

enum ButtonEvent {
  BUTTON_NO_EVENT = 0,
  BUTTON_CLICK = 0x01,
  BUTTON_DOUBLE_CLICK = 0x02,
  BUTTON_LONG_PRESS = 0x04,
  BUTTON_PRESSED = 0x10,
  BUTTON_CHANGED = 0x20,
  BUTTON_ACTION_MASK = 0x0f
};

typedef struct {
  int buttonPin;
  ButtonType buttonType;
  int clickRelayId;
  int longClickRelayId;
  int doubleClickRelayId;
  const char * buttonDescription;
} ButtonConfigDef;


#ifdef USE_EXPANDER

class BounceExp : public Bounce {
  public:
    BounceExp()
      : Bounce()
      , _isExpander(false)
      , _expanderNo(0)
    {};

    #if defined(EXPANDER_PCF8574)
      static void setExpander(PCF8574 * exp) { _expander = exp; };
    #elif defined(EXPANDER_MCP23017)
      static void setExpander(Adafruit_MCP23017 * exp) { _expander = exp; };
    #endif

  protected:
    bool readCurrentState() {

      int result;
      if (_isExpander) {
        result = _expander[_expanderNo].digitalRead(pin);
      } else {
        result = Bounce::readCurrentState();
      }
      return result;
    }

    void setPinMode(int _pin, int mode) {

      if (_pin & 0xff00) { // expander
        _isExpander = true;
        _expanderNo = (_pin >> 8) - 1;
        int expanderPin = _pin & 0xff;
        #if defined(EXPANDER_MCP23017)
          // https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library/blob/1.2.0/Adafruit_MCP23017.cpp#L145
          // pinMode method doesn't support INPUT_PULLUP, workaround below:
          if (mode == INPUT_PULLUP) {
            _expander[_expanderNo].pinMode(expanderPin, INPUT);
            _expander[_expanderNo].pullUp(expanderPin, HIGH);
          } else {
            _expander[_expanderNo].pinMode(expanderPin, mode);
          }
        #else
           _expander[expanderNo].pinMode(expanderPin, mode);
        #endif
      } else {
        Bounce::setPinMode(_pin, mode);
      }
    }
    
    #if defined(EXPANDER_PCF8574)
      static PCF8574 * _expander;
    #elif defined(EXPANDER_MCP23017)
      static Adafruit_MCP23017 * _expander;
    #endif

    bool _isExpander;
    uint8_t _expanderNo;
};

#endif


class Button {

  public:
    Button();
    void initialize(int, const char *);
    void setAction(int, int, int);
    void attachPin(int);
    void setDebounceInterval(unsigned long interval) { _physicalButton.interval(interval); };
    static void setEventIntervals(unsigned long, unsigned long);
    static void setMonoStableTrigger(uint8_t);
    int updateAndGetRelayNum();
    bool getRelayState(bool);
    int getEvent(bool, int);
    String toString();
    #if defined(EXPANDER_PCF8574)
      static void expanderInit(PCF8574 * exp) { BounceExp::setExpander(exp); };
    #elif defined(EXPANDER_MCP23017)
      static void expanderInit(Adafruit_MCP23017 * exp) { BounceExp::setExpander(exp); };
    #endif

  private:
    int _pin;
    BounceExp _physicalButton;
    int _type;
    const char * _description;
    bool _stateForPressed; // LOW by default
    int _clickRelayNum;
    int _longclickRelayNum;
    int _doubleclickRelayNum;
    int _eventState;
    unsigned long _startStateMillis;
    static uint8_t _monoStableTrigger;
    static unsigned long _doubleclickInterval;
    static unsigned long _longclickInterval;
};

} //namespace

#endif
