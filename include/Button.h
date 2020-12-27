#ifndef Button_h
#define Button_h

#include <Bounce2.h>
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
    #if defined(EXPANDER_PCF8574)
      static void setExpander(PCF8574 * exp) { _expander = exp; };
    #elif defined(EXPANDER_MCP23017)
      static void setExpander(Adafruit_MCP23017 * exp) { _expander = exp; };
    #endif

  private:
    bool readCurrentState() {
      
      int result;
      if (pin & 0xff00) { // expander
        int expanderNo = (pin >> 8) - 1;
        int expanderPin = pin & 0xff;
        result = _expander[expanderNo].digitalRead(expanderPin);
      } else {
        result = Bounce::readCurrentState();
      }
      return result;
    }
    
    void setPinMode(int _pin, int mode) {
      
      if (_pin & 0xff00) { // expander
        int expanderNo = (_pin >> 8) - 1;
        int expanderPin = _pin & 0xff;
        _expander[expanderNo].pinMode(expanderPin, mode);
      } else {
        Bounce::setPinMode(_pin, mode);
      }
    }

    #if defined(EXPANDER_PCF8574)
      static PCF8574 * _expander;
    #elif defined(EXPANDER_MCP23017)
      static Adafruit_MCP23017 * _expander;
    #endif
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
    inline bool getState() { return(_physicalButton.read()); };
    inline const char * getDescription() { return(_description); };
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
