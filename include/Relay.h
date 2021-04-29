#pragma once

#include <ArduinoAbstract.h>

namespace lkankowski {


class Relay {

  public:
    explicit Relay(PinInterface * pin);
    ~Relay();

    void initialize(int, const char *);
    void attachPin();
    void setTriggerMode(uint8_t mode) { _triggerState = mode; };
    bool changeState(bool);
    bool getState() { return(_state); };
    int getSensorId() { return(_sensorId); }; //TODO: refactor: move to RelayService
    void reportAsSensor() { _reportAsSensor = true; };
    bool isSensor() { return _reportAsSensor; };
    const char * getDescription() { return(_description); };

  private:
    PinInterface * _pin;
    bool _state; // true/false = ON/OFF
    int _sensorId;
    const char * _description;
    uint8_t _triggerState;
    bool _reportAsSensor;
};

typedef Relay* RelayPtr;

} //namespace
