#pragma once

#include <Switch.h>
#include <Configuration.h>


namespace lkankowski {

enum ButtonEvent {
  BUTTON_NO_EVENT = 0,
  BUTTON_CLICK = 0x01,
  BUTTON_DOUBLE_CLICK = 0x02,
  BUTTON_LONG_PRESS = 0x04,
};


class ButtonInterface
{
  public:
    virtual ~ButtonInterface();

    virtual ButtonEvent checkEvent(unsigned long) = 0;
    virtual bool getRelayState(bool) = 0;

    void attachPin();
    bool getState() const { return _switch->getState(); };

    static void setEventIntervals(unsigned long, unsigned long);
    static ButtonInterface * create(ButtonType, int, unsigned int, bool, bool);

  protected:
    enum ButtonState {
      BTN_STATE_INITIAL,
      BTN_STATE_DEBOUNCE,
      BTN_STATE_1ST_PRESS,
      BTN_STATE_1ST_RELEASE,
      BTN_STATE_2ND_PRESS,
      BTN_STATE_RELEASE_WAIT,
      BTN_STATE_1ST_CHANGE_BI
    };

    enum ButtonEventInternal {
      BUTTON_PRESSED = 0x10,
      BUTTON_CHANGED = 0x20,
      BUTTON_ACTION_MASK = 0x0f
    };

    ButtonInterface(HardwareSwitchInterface *);
    virtual int calculateEvent(bool, unsigned long) = 0;

    HardwareSwitchInterface * _switch;
    int _eventState;
    unsigned long _startStateMillis;
    static unsigned long _doubleclickInterval;
    static unsigned long _longclickInterval;
};

typedef ButtonInterface* ButtonPtr;


class MonoStableButton : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    ButtonEvent checkEvent(unsigned long) override;
    bool getRelayState(bool) override;
    static void clickTriggerWhenPressed(bool);

  protected:
    MonoStableButton(HardwareSwitchInterface *, bool, bool);
    int calculateEvent(bool, unsigned long) override;

    bool _hasLongClick;
    bool _hasDoubleClick;
    static bool _clickTriggerWhenPressed;
};


class BiStableButton : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    ButtonEvent checkEvent(unsigned long) override;
    bool getRelayState(bool) override;

  protected:
    BiStableButton(HardwareSwitchInterface *, bool);
    int calculateEvent(bool, unsigned long) override;

    bool _hasDoubleClick;
};


class DingDongButton : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    ButtonEvent checkEvent(unsigned long) override;
    bool getRelayState(bool) override;

  protected:
    DingDongButton(HardwareSwitchInterface *);
    int calculateEvent(bool, unsigned long) override;
};


class ReedSwitch : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    ButtonEvent checkEvent(unsigned long) override;
    bool getRelayState(bool) override;

  protected:
    ReedSwitch(HardwareSwitchInterface *);
    int calculateEvent(bool, unsigned long) override;
};


} //namespace
