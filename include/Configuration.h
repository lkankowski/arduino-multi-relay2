#pragma once

#include <ArduinoAbstract.h>
#include <stddef.h>

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
  char relayDescription[30];
};

struct RelayConfigRef {
  const RelayConfigDef * cfg;
  size_t size;
};


struct ButtonConfigDef {
  int buttonPin;
  ButtonType buttonType;
  int clickRelayId;
  int longClickRelayId;
  int doubleClickRelayId;
  char buttonDescription[30];
};

struct ButtonConfigRef {
  const ButtonConfigDef * cfg;
  size_t size;
};


class Configuration
{
  public:
    Configuration(const RelayConfigRef &, const ButtonConfigRef &
    #ifdef USE_EXPANDER
      , const uint8_t *, const size_t
    #endif
    );
    ~Configuration();

    int getRelayNum(int) const;
    inline size_t getRelaysCount() const { return _relayConfig.size; };
    int getRelayPin(size_t relayNum);
    inline uint8_t getRelayOptions(size_t relayNum) const { return _relayOptions[relayNum]; };
    uint8_t getRelayDependsOn(size_t relayNum);
    inline int getRelaySensorId(size_t relayNum) const { return _relaySensorId[relayNum]; };
    const char * getRelayDescription(size_t relayNum);

    inline size_t getButtonsCount() const { return _buttonConfig.size; };
    ButtonType getButtonType(size_t buttonNum);
    int getButtonPin(size_t buttonNum);
    const char * getButtonDescription(size_t buttonNum);
    int getButtonClickAction(size_t buttonNum);
    int getButtonLongClickAction(size_t buttonNum);
    int getButtonDoubleClickAction(size_t buttonNum);
    
  private:
    void loadRelayConfigFromPROGMEM(size_t);
    void loadButtonConfigFromPROGMEM(size_t);

    const RelayConfigRef & _relayConfig;
    RelayConfigDef _relayConfigEntryBuf;
    size_t _relayNumInBuf = -1;

    uint8_t * _relaySensorId;
    uint8_t * _relayOptions;

    const ButtonConfigRef & _buttonConfig;
    ButtonConfigDef _buttonConfigEntryBuf;
    size_t _buttonNumInBuf = -1;

  #ifdef USE_EXPANDER
    const uint8_t * _expanderAddresses;
    const size_t _expanderSize;
  #endif
};

}; // namespace
