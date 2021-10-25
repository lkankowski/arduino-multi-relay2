#pragma once

#include <Switch.h>
#include <Configuration.h>


namespace lkankowski {

class ButtonInterface
{
  public:
    virtual ~ButtonInterface();

    virtual int checkEvent(unsigned long) = 0;
    virtual bool getRelayState(bool) = 0;

    void setAction(int, int, int);
    void attachPin();
    bool getState() const { return _switch->getState(); };

    static void setEventIntervals(unsigned long, unsigned long);
    static ButtonInterface * create(ButtonType, int, unsigned int);

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
    int _clickRelayNum;
    int _longclickRelayNum;
    int _doubleclickRelayNum;
    ButtonState _eventState;
    unsigned long _startStateMillis;
    static unsigned long _doubleclickInterval;
    static unsigned long _longclickInterval;
};

typedef ButtonInterface* ButtonPtr;


class MonoStableButton : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    int checkEvent(unsigned long) override;
    inline bool getRelayState(bool relayState) override { return ! relayState; };
    static void clickTriggerWhenPressed(bool);

  protected:
    MonoStableButton(HardwareSwitchInterface *);
    int calculateEvent(bool, unsigned long) override;

    static bool _clickTriggerWhenPressed;
};


class BiStableButton : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    int checkEvent(unsigned long) override;
    inline bool getRelayState(bool relayState) override { return ! relayState; };

  protected:
    BiStableButton(HardwareSwitchInterface *);
    int calculateEvent(bool, unsigned long) override;
};


class DingDongButton : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    int checkEvent(unsigned long) override;
    inline bool getRelayState(bool) override { return _switch->getState(); };

  protected:
    DingDongButton(HardwareSwitchInterface *);
    inline int calculateEvent(bool, unsigned long) override { return 0; };
};


class ReedSwitch : public ButtonInterface
{
  friend class ButtonInterface;

  public:
    int checkEvent(unsigned long) override;
    inline bool getRelayState(bool) override { return ! _switch->getState(); };

  protected:
    ReedSwitch(HardwareSwitchInterface *);
    inline int calculateEvent(bool, unsigned long) override { return 0; };
};


} //namespace
