#pragma once

#include <stdint.h>

namespace lkankowski {

const uint8_t RELAY_TRIGGER_LOW  = 0;
const uint8_t RELAY_TRIGGER_HIGH = 1;
const uint8_t RELAY_STARTUP_ON   = 2;
const uint8_t RELAY_STARTUP_OFF  = 4;
const uint8_t RELAY_IMPULSE      = 8;
const uint8_t RELAY_INDEPENDENT  = 16;


enum ButtonType {
  MONO_STABLE = 0,
  BI_STABLE = 1,
  DING_DONG = 2, // HIGH state immediatly after push, LOW state after release
  REED_SWITCH = 3, // magnetic sensor for door or window, LOW - closed, HIGH - opened
  PRESSED_STATE_HIGH = 0x10
};


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


typedef struct {
  int buttonPin;
  ButtonType buttonType;
  int clickRelayId;
  int longClickRelayId;
  int doubleClickRelayId;
  const char * const buttonDescription;
} ButtonConfigDef;

typedef struct {
  const ButtonConfigDef * config;
  int size;
} ButtonConfigRef;


class Configuration
{
  public:
    Configuration(const RelayConfigRef &, const ButtonConfigRef &);
    ~Configuration();

    int getRelayNum(int) const;
    int getRelaysCount() const { return _relayConfig.size; };

  private:
    const RelayConfigRef & _relayConfig;
    const ButtonConfigRef & _buttonConfig;
};

}; // namespace
