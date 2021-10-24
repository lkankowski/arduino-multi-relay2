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
    inline bool getState(int relayNum) { return _relays[relayNum]->getState(); };
    bool impulseProcess(int, unsigned long);
    void setImpulseInterval(unsigned long impulseInterval) { _impulseInterval = impulseInterval; };
    inline bool isImpulsePending() { return(_impulsePending > 0); };
    bool turnOffDependent(unsigned long);
    inline int getSensorId(int relayNum) { return _configuration.getRelaySensorId(relayNum); };
    inline void reportAsSensor(int relayNum) { _reportAsSensor[relayNum] = true; };
    inline bool isSensor(int relayNum) const { return _reportAsSensor[relayNum]; };
    inline const char * getDescription(int relayNum) { return _configuration.getRelayDescription(relayNum); };
    void printDebug(int);

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
