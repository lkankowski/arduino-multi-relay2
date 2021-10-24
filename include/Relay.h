#pragma once

#include <ArduinoAbstract.h>

namespace lkankowski {


class Relay {

  public:
    explicit Relay(PinInterface * pin);
    ~Relay();

    void attachPin();
    inline void setTriggerMode(uint8_t mode) { _triggerState = mode; };
    bool changeState(bool);
    inline bool getState() { return(_state); };

  private:
    PinInterface * _pin;
    bool _state; // true/false = ON/OFF
    uint8_t _triggerState;
};

typedef Relay* RelayPtr;

} //namespace
