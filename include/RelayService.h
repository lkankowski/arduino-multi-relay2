#pragma once

#include <Relay.h>

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


class RelayService {

  public:
    RelayService(const int, Relay *, const RelayConfigDef *);
    ~RelayService();

    void initialize(bool);
    bool changeState(int, bool);
    bool impulseProcess(int);
    void setImpulseInterval(unsigned long impulseInterval) { _impulseInterval = impulseInterval; };
    bool isImpulsePending() { return(_impulsePending > 0); };
    bool turnOffDependent();
    int getRelayNum(int);


  private:
    int _numberOfRelays;
    Relay * _relays;
    const RelayConfigDef * _relayConfig;
    bool * _storeRelayToEEPROM;
    int _impulsePending;
    unsigned long _impulseInterval;
    bool * _relayIsImpulse;
    unsigned long * _relayImpulseStartMillis;
    int * _relayDependsOn;
    bool _isAnyDependentOn;
    bool * _isRelayDependent;
};

}; // namespace
