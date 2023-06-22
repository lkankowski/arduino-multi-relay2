#pragma once

#include <ButtonCallbackInterface.h>
#include <RelayStateNotification.h>
#include <Relay.h>
#include <ArduinoAbstract.h>
#include <EepromAbstract.h>
#include <Configuration.h>


namespace lkankowski {

const uint8_t RELAY_STARTUP_MASK = RELAY_STARTUP_ON | RELAY_STARTUP_OFF;

#ifndef RELAY_STATE_STORAGE
  #define RELAY_STATE_STORAGE 1
#endif

class RelayService : public ButtonCallbackInterface
{
  public:
    RelayService(Configuration &, EepromInterface &, RelayStateNotification &);
    virtual ~RelayService();

    void initialize(bool);
    bool changeRelayState(int, bool, unsigned long) override;
    bool toogleRelayState(int, unsigned long) override;
    inline bool getState(int relayNum) { return _relays[relayNum]->getState(); };
    void processImpulse(unsigned long);
    void setImpulseInterval(unsigned long impulseInterval) { _impulseInterval = impulseInterval; };
    bool turnOffDependent(unsigned long);
    inline const char * getDescription(int relayNum) const { return _configuration.getRelayDescription(relayNum); };
    void printDebug(int) const;

  private:
    RelayPtr * _relays;
    PinInterface ** _pin;
    Configuration & _configuration;
    bool * _storeRelayToEEPROM;
    EepromInterface & _eeprom;
    RelayStateNotification & _relayStateNotification;
    int _impulsePending;
    unsigned long _impulseInterval;
    bool * _relayIsImpulse;
    unsigned long * _relayImpulseStartMillis;
    int * _relayDependsOn;
    bool _isAnyDependentOn;
    bool * _isRelayDependent;
};

}; // namespace
