#pragma once

#include <stdint.h>
#include <Vector.h>

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


struct RelayConfigDef {
  int sensorId;
  int relayPin;
  uint8_t relayOptions;
  int dependsOn;
  char relayDescription[25];
};


struct ButtonConfigDef {
  int buttonPin;
  ButtonType buttonType;
  int clickRelayId;
  int longClickRelayId;
  int doubleClickRelayId;
  char buttonDescription[25];
};

class Configuration
{
  public:
    Configuration(const Vector<const RelayConfigDef> &, const Vector<const ButtonConfigDef> &);
    ~Configuration();

    int getRelayNum(int) const;
    size_t getRelaysCount() const { return _relayConfig.size(); };
    int getRelayPin(size_t relayNum);
    uint8_t getRelayOptions(size_t relayNum) const { return _relayOptions[relayNum]; };
    uint8_t getRelayDependsOn(size_t relayNum);
    int getRelaySensorId(size_t relayNum) const { return _relaySensorId[relayNum]; };
    const char * getRelayDescription(size_t relayNum);

    size_t getButtonsCount() const { return _buttonConfig.size(); };
    ButtonType getButtonType(size_t buttonNum);
    int getButtonPin(size_t buttonNum);
    const char * getButtonDescription(size_t buttonNum);

  private:
    void loadRelayConfigFromPROGMEM(size_t);
    void loadButtonConfigFromPROGMEM(size_t);

    const Vector<const RelayConfigDef> & _relayConfig;
    RelayConfigDef _relayConfigEntryBuf;
    size_t _relayNumInBuf = -1;

    uint8_t * _relaySensorId;
    uint8_t * _relayOptions;

    const Vector<const ButtonConfigDef> & _buttonConfig;
    ButtonConfigDef _buttonConfigEntryBuf;
    size_t _buttonNumInBuf = -1;
};

}; // namespace
