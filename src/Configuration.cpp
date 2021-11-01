#include <Configuration.h>

using namespace lkankowski;


Configuration::Configuration(const RelayConfigRef & relayConfig,
                             const ButtonConfigRef & buttonConfig
                          #ifdef USE_EXPANDER
                           , const uint8_t * expanderAddresses
                           , size_t expanderSize
                          #endif
                            )
  : _relayConfig(relayConfig)
  , _buttonConfig(buttonConfig)
#ifdef USE_EXPANDER
  , _expanderAddresses(expanderAddresses)
  , _expanderSize(expanderSize)
#endif
{
  _relaySensorId = new uint8_t[_relayConfig.size];
  _relayOptions = new uint8_t[_relayConfig.size];

  for (size_t relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
    loadRelayConfigFromPROGMEM(relayNum);
    _relaySensorId[relayNum] = _relayConfigEntryBuf.sensorId;
    _relayOptions[relayNum] = _relayConfigEntryBuf.relayOptions;
  }

  #ifndef UNIT_TEST
    validate();
  #endif
};


Configuration::~Configuration()
{
  delete _relayOptions;
  delete _relaySensorId;
};


bool Configuration::validate()
{
  #ifdef USE_EXPANDER
    //TODO: check if I2C pins are not used
    for (size_t relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
      loadRelayConfigFromPROGMEM(relayNum);
      int pin = _relayConfigEntryBuf.relayPin;
      if (pin >= 0x0100) {
        if ((((size_t) (pin >> 8)) > _expanderSize) || ((pin & 0xff) >= EXPANDER_PINS)) {
          Serial << F("Configuration failed - expander no or number of pins out of range for relay: ") << relayNum << "\n";
          haltSystem();
          return false;
        }
      }
    }
  #endif

  for (size_t buttonNum = 0; buttonNum < _buttonConfig.size; buttonNum++) {
    loadButtonConfigFromPROGMEM(buttonNum);

    #ifdef USE_EXPANDER
      int pin = _buttonConfigEntryBuf.buttonPin;
      if (pin >= 0x0100) {
        if ((((size_t) (pin >> 8)) > _expanderSize) || ((pin & 0xff) >= EXPANDER_PINS)) {
          Serial << F("Configuration failed - expander no or number of pins out of range for button: ") << buttonNum << "\n";
          haltSystem();
          return false;
        }
      }
    #endif

    const char * failAction[] = {"OK", "click", "long-press", "double-click"};
    int fail = 0;
    if ((_buttonConfigEntryBuf.clickRelayId != -1) && (getRelayNum(_buttonConfigEntryBuf.clickRelayId) == -1)) fail = 1;
    if ((_buttonConfigEntryBuf.longClickRelayId != -1) && (getRelayNum(_buttonConfigEntryBuf.longClickRelayId) == -1)) fail = 2;
    if ((_buttonConfigEntryBuf.doubleClickRelayId != -1) && (getRelayNum(_buttonConfigEntryBuf.doubleClickRelayId) == -1)) fail = 3;
    if (fail) {
      Serial << F("Configuration failed - invalid '") << failAction[fail] << F(" relay ID' for button: ") << buttonNum << "\n";
      haltSystem();
      return false;
    }
    // TODO: validate if pin is correct to the current board
  }
  return true;
};


int Configuration::getRelayNum(int sensorId) const
{  
  if (sensorId > -1) {
    for (size_t relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
      if (_relaySensorId[relayNum] == sensorId) return(relayNum);
    }
  }
  return(-1);
};


int Configuration::getRelayPin(size_t relayNum)
{
  if (relayNum != _relayNumInBuf) loadRelayConfigFromPROGMEM(relayNum);
  return _relayConfigEntryBuf.relayPin;
};


uint8_t Configuration::getRelayDependsOn(size_t relayNum)
{
  if (relayNum != _relayNumInBuf) loadRelayConfigFromPROGMEM(relayNum);
  return _relayConfigEntryBuf.dependsOn;
};


const char * Configuration::getRelayDescription(size_t relayNum)
{
  if (relayNum != _relayNumInBuf) loadRelayConfigFromPROGMEM(relayNum);
  return _relayConfigEntryBuf.relayDescription;
};


int Configuration::getButtonPin(size_t buttonNum)
{
  if (buttonNum != _buttonNumInBuf) loadButtonConfigFromPROGMEM(buttonNum);
  return _buttonConfigEntryBuf.buttonPin;
};


int Configuration::getButtonType(size_t buttonNum)
{
  if (buttonNum != _buttonNumInBuf) loadButtonConfigFromPROGMEM(buttonNum);
  return _buttonConfigEntryBuf.buttonType;
};


const char * Configuration::getButtonDescription(size_t buttonNum)
{
  if (buttonNum != _buttonNumInBuf) loadButtonConfigFromPROGMEM(buttonNum);
  return _buttonConfigEntryBuf.buttonDescription;
};


int Configuration::getButtonClickAction(size_t buttonNum)
{
  if (buttonNum != _buttonNumInBuf) loadButtonConfigFromPROGMEM(buttonNum);
  return _buttonConfigEntryBuf.clickRelayId;
};


int Configuration::getButtonLongClickAction(size_t buttonNum)
{
  if (buttonNum != _buttonNumInBuf) loadButtonConfigFromPROGMEM(buttonNum);
  return _buttonConfigEntryBuf.longClickRelayId;
};


int Configuration::getButtonDoubleClickAction(size_t buttonNum)
{
  if (buttonNum != _buttonNumInBuf) loadButtonConfigFromPROGMEM(buttonNum);
  return _buttonConfigEntryBuf.doubleClickRelayId;
};


void Configuration::loadRelayConfigFromPROGMEM(size_t relayNum) 
{
  PROGMEM_readAnything(&_relayConfig.cfg[relayNum], _relayConfigEntryBuf);
  _relayNumInBuf = relayNum;
};


void Configuration::loadButtonConfigFromPROGMEM(size_t buttonNum) 
{
  PROGMEM_readAnything(&_buttonConfig.cfg[buttonNum], _buttonConfigEntryBuf);
  _buttonNumInBuf = buttonNum;
};

