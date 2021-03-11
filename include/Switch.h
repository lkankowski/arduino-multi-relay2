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

    static HardwareSwitchInterface * create(int, PinInterface&, unsigned int, uint8_t);

    enum HWSwitchType {
      SWITCH_DEBOUNCED
    };
};


class DebouncedSwitch : public HardwareSwitchInterface
{
  public:
    DebouncedSwitch(PinInterface&, unsigned int, uint8_t);
    virtual ~DebouncedSwitch();

    void attachPin();
    bool update(unsigned long);
    bool getState() const;
  
  private:
    PinInterface& _pin;
    const unsigned int _debounceInterval;
    unsigned long _previousMillis;
    bool _unstableState;
    bool _debouncedState;
    uint8_t _stateForPressed;
};


}; //namespace
