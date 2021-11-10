#pragma once

#include <Switch.h>
#include <Configuration.h>


namespace lkankowski {

class ButtonInterface
{
  public:
    virtual ~ButtonInterface();

    virtual int checkEvent(unsigned long) = 0;
    virtual bool isToogle() const = 0;
    virtual bool getRelayState() const = 0;

    void attachPin();
    bool getState() const { return _switch->getState(); };

    static void setEventIntervals(unsigned long, unsigned long);
    static ButtonInterface * create(int, int, unsigned int, int, int, int);

  protected:
    ButtonInterface(HardwareSwitchInterface *);
    virtual int calculateEvent(bool, unsigned long) = 0;

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

    HardwareSwitchInterface * _switch;
    ButtonState _eventState;
    unsigned long _startStateMillis;
    int _clickActionId;
    static unsigned long _doubleclickInterval;
    static unsigned long _longclickInterval;
};

typedef ButtonInterface* ButtonPtr;


class MonoStableButton : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    int checkEvent(unsigned long) override;
    bool isToogle() const override { return true; };
    bool getRelayState() const override { return false; };
    static void clickTriggerWhenPressed(bool);

  protected:
    MonoStableButton(HardwareSwitchInterface *, int, int, int);
    int calculateEvent(bool, unsigned long) override;

    int _longClickActionId;
    int _doubleClickActionId;
    static bool _clickTriggerWhenPressed;
};


class BiStableButton : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    int checkEvent(unsigned long) override;
    bool isToogle() const override { return true; };
    bool getRelayState() const override { return false; };

  protected:
    BiStableButton(HardwareSwitchInterface *, int, int);
    int calculateEvent(bool, unsigned long) override;

    int _doubleClickActionId;
};


class DingDongButton : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    int checkEvent(unsigned long) override;
    bool isToogle() const override { return false; };
    bool getRelayState() const override { return _switch->getState(); };

  protected:
    DingDongButton(HardwareSwitchInterface *, int);
    int calculateEvent(bool, unsigned long) override { return 0; };
};


class ReedSwitch : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    int checkEvent(unsigned long) override;
    bool isToogle() const override { return false; };
    bool getRelayState() const override { return ! _switch->getState(); };

  protected:
    ReedSwitch(HardwareSwitchInterface *, int);
    int calculateEvent(bool, unsigned long) override { return 0; };
};


} //namespace
