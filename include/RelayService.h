#pragma once

#include <Relay.h>
#include <ArduinoAbstract.h>
#include <EepromAbstract.h>
#include <Configuration.h>


namespace lkankowski {

const uint8_t RELAY_STARTUP_MASK = RELAY_STARTUP_ON | RELAY_STARTUP_OFF;

#define RELAY_STATE_STORAGE 1

class RelayService {

  public:
    RelayService(Configuration &, EepromInterface &);
    ~RelayService();

    void initialize(bool);
    bool changeState(int, bool, unsigned long);
    bool getState(int);
    bool impulseProcess(int, unsigned long);
    void setImpulseInterval(unsigned long impulseInterval) { _impulseInterval = impulseInterval; };
    bool isImpulsePending() { return(_impulsePending > 0); };
    bool turnOffDependent(unsigned long);
    int getSensorId(int);
    void reportAsSensor(int relayNum) { _reportAsSensor[relayNum] = true; };
    bool isSensor(int relayNum) const { return _reportAsSensor[relayNum]; };
    const char * getDescription(int);
    String toString(int);

  private:
    RelayPtr * _relays;
    PinInterface ** _pin;
    Configuration & _configuration;
    bool * _storeRelayToEEPROM;
    EepromInterface& _eeprom;
    int _impulsePending;
    unsigned long _impulseInterval;
    bool * _relayIsImpulse;
    unsigned long * _relayImpulseStartMillis;
    int * _relayDependsOn;
    bool _isAnyDependentOn;
    bool * _isRelayDependent;
    bool * _reportAsSensor;
};

}; // namespace
