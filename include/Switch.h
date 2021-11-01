#pragma once

#include <ArduinoAbstract.h>

namespace lkankowski {


class HardwareSwitchInterface
{
  public:
    virtual ~HardwareSwitchInterface() {};

    virtual void attachPin() = 0;
    virtual bool update(unsigned long) = 0;
    virtual bool getState() const = 0;

    static HardwareSwitchInterface * create(int, int, unsigned int, uint8_t);

    enum HWSwitchType {
      SWITCH_DEBOUNCED
    };
};


class DebouncedSwitch : public HardwareSwitchInterface
{
  friend class HardwareSwitchInterface;
  
  public:
    virtual ~DebouncedSwitch();

    void attachPin() override;
    bool update(unsigned long) override;
    inline bool getState() const override { return _debouncedState; };
  
  private:
    DebouncedSwitch(int, unsigned int, uint8_t);

    PinInterface * _pin;
    const unsigned int _debounceInterval;
    unsigned long _previousMillis;
    bool _unstableState;
    bool _debouncedState;
    uint8_t _stateForPressed;
};


}; //namespace
