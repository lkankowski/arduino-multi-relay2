#pragma once

#include <Relay.h>
#include <ArduinoAbstract.h>
#include <EepromAbstract.h>

namespace lkankowski {

const uint8_t RELAY_TRIGGER_LOW  = 0;
const uint8_t RELAY_TRIGGER_HIGH = 1;
const uint8_t RELAY_STARTUP_ON   = 2;
const uint8_t RELAY_STARTUP_OFF  = 4;
const uint8_t RELAY_IMPULSE      = 8;
const uint8_t RELAY_INDEPENDENT  = 16;
const uint8_t RELAY_STARTUP_MASK = RELAY_STARTUP_ON | RELAY_STARTUP_OFF;

#define RELAY_STATE_STORAGE 1

typedef struct {
  int sensorId;
  int relayPin;
  uint8_t relayOptions;
  int dependsOn;
  const char * relayDescription;
} RelayConfigDef;

typedef struct {
  const RelayConfigDef * config;
  int size;
} RelayConfigRef;


class RelayService {

  public:
    RelayService(const RelayConfigRef &, EepromInterface &);
    ~RelayService();

    void initialize(bool);
    bool changeState(int, bool, unsigned long);
    bool getState(int);
    bool impulseProcess(int, unsigned long);
    void setImpulseInterval(unsigned long impulseInterval) { _impulseInterval = impulseInterval; };
    bool isImpulsePending() { return(_impulsePending > 0); };
    bool turnOffDependent(unsigned long);
    int getSensorId(int);
    int getRelayNum(int);
    const char * getDescription(int);
    String toString(int);

  private:
    int _numberOfRelays;
    RelayPtr * _relays;
    const RelayConfigRef & _relayConfig;
    bool * _storeRelayToEEPROM;
    EepromInterface& _eeprom;
    int _impulsePending;
    unsigned long _impulseInterval;
    bool * _relayIsImpulse;
    unsigned long * _relayImpulseStartMillis;
    int * _relayDependsOn;
    bool _isAnyDependentOn;
    bool * _isRelayDependent;
};

}; // namespace
