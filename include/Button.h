#pragma once

//#include <ArduinoAbstract.h>
#include <Switch.h>

namespace lkankowski {

enum ButtonType {
  MONO_STABLE = 0,
  BI_STABLE = 1,
  DING_DONG = 2, // HIGH state immediatly after push, LOW state after release
  REED_SWITCH = 3, // magnetic sensor for door or window, LOW - closed, HIGH - opened
  PRESSED_STATE_HIGH = 0x10
};


class ButtonInterface
{
  public:
    ButtonInterface(PinInterface&, unsigned int, ButtonType, const char * const);
    virtual ~ButtonInterface();

    virtual int checkEvent(unsigned long) = 0;
    virtual bool getRelayState(bool) = 0;

    void setAction(int, int, int);
    void attachPin();
    String toString();

    static void setEventIntervals(unsigned long, unsigned long);
    static ButtonInterface * create(ButtonType, PinInterface&, unsigned int, const char * const);

  protected:
    virtual int calculateEvent(bool, unsigned long) = 0;

    enum ButtonState {
      BTN_STATE_INITIAL,
      BTN_STATE_DEBOUNCE,
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

    HardwareSwitchInterface * _switch;
    const char * const _description;
    int _clickRelayNum;
    int _longclickRelayNum;
    int _doubleclickRelayNum;
    int _eventState;
    unsigned long _startStateMillis;
    static unsigned long _doubleclickInterval;
    static unsigned long _longclickInterval;
};

typedef ButtonInterface* ButtonPtr;


class MonoStableButton : public ButtonInterface
{
  public:
    MonoStableButton(PinInterface&, unsigned int, ButtonType, const char * const);

    int checkEvent(unsigned long) override;
    bool getRelayState(bool) override;
    static void clickTriggerWhenPressed(bool);

  protected:
    int calculateEvent(bool, unsigned long) override;

    static bool _clickTriggerWhenPressed;
};


class BiStableButton : public ButtonInterface
{
  public:
    BiStableButton(PinInterface&, unsigned int, ButtonType, const char * const);

    int checkEvent(unsigned long) override;
    bool getRelayState(bool) override;

  protected:
    int calculateEvent(bool, unsigned long) override;
};


class DingDongButton : public ButtonInterface
{
  public:
    DingDongButton(PinInterface&, unsigned int, ButtonType, const char * const);

    int checkEvent(unsigned long) override;
    bool getRelayState(bool) override;

  protected:
    int calculateEvent(bool, unsigned long) override;
};


class ReedSwitch : public ButtonInterface
{
  public:
    ReedSwitch(PinInterface&, unsigned int, ButtonType, const char * const);

    int checkEvent(unsigned long) override;
    bool getRelayState(bool) override;

  protected:
    int calculateEvent(bool, unsigned long) override;
};


} //namespace
