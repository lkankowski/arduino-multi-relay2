#pragma once

#include <Debounce.h>

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




class Button
{
  public:
    Button();
    void initialize(int, const char *);
    void setAction(int, int, int);
    void attachPin(int);
    void setDebounceInterval(unsigned long interval) { _physicalButton.interval(interval); };
    static void setEventIntervals(unsigned long, unsigned long);
    static void setMonoStableTrigger(uint8_t);
    int updateAndGetRelayNum(unsigned long);
    bool getRelayState(bool);
    String toString();
    #if defined(EXPANDER_PCF8574)
      static void expanderInit(PCF8574 * exp) { Debounce::setExpander(exp); };
    #elif defined(EXPANDER_MCP23017)
      static void expanderInit(Adafruit_MCP23017 * exp) { Debounce::setExpander(exp); };
    #endif

  private:
    int getEvent(bool, int, unsigned long);

    int _pin;
    Debounce _physicalButton;
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
