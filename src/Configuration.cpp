#include <Configuration.h>
#include <ArduinoAbstract.h>

using namespace lkankowski;


Configuration::Configuration(const Vector<const RelayConfigDef> & relayConfig,
                             const Vector<const ButtonConfigDef> & buttonConfig)
  : _relayConfig(relayConfig)
  , _buttonConfig(buttonConfig)
{
  _relaySensorId = new uint8_t[_relayConfig.size()];
  _relayOptions = new uint8_t[_relayConfig.size()];

  for (size_t relayNum = 0; relayNum < _relayConfig.size(); relayNum++) {
    loadRelayConfigFromPROGMEM(relayNum);
    _relaySensorId[relayNum] = _relayConfigEntryBuf.sensorId;
    _relayOptions[relayNum] = _relayConfigEntryBuf.relayOptions;
  }

  // validate config
  #ifdef USE_EXPANDER
    //TODO: check if I2C pins are not used
    for (int relayNum = 0; relayNum < gRelayConfigRef.size; relayNum++) {
      //TODO: read config from PROGMEM
      // RelayConfigDef relayConfig = {};
      // PROGMEM_readAnything(&gRelayConfig[relayNum], relayConfig);
      int pin = gRelayConfig[relayNum].relayPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > sizeof(gExpanderAddresses)) || ((pin & 0xff) >= EXPANDER_PINS)) {
          printf_P(PSTR("Configuration failed - expander no or number of pins out of range for relay: %i\n"), relayNum);
          haltSystem();
        }
      }
    }
  #endif

  for (size_t buttonNum = 0; buttonNum < _buttonConfig.size(); buttonNum++) {
    loadButtonConfigFromPROGMEM(buttonNum);

    #ifdef USE_EXPANDER
      int pin = _buttonConfigEntryBuf.buttonPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > sizeof(gExpanderAddresses)) || ((pin & 0xff) >= EXPANDER_PINS)) {
          printf_P(PSTR("Configuration failed - expander no or number of pins out of range for button: %i\n"), buttonNum);
          haltSystem();
        }
      }
    #endif

    const char * failAction[] = {"OK", "click", "long-press", "double-click"};
    int fail = 0;
    if ((_buttonConfigEntryBuf.clickRelayId != -1) && (getRelayNum(_buttonConfigEntryBuf.clickRelayId) == -1)) fail = 1;
    if ((_buttonConfigEntryBuf.longClickRelayId != -1) && (getRelayNum(_buttonConfigEntryBuf.longClickRelayId) == -1)) fail = 2;
    if ((_buttonConfigEntryBuf.doubleClickRelayId != -1) && (getRelayNum(_buttonConfigEntryBuf.doubleClickRelayId) == -1)) fail = 3;
    if (fail) {
      printf_P(PSTR("Configuration failed - invalid '%s relay ID' for button: %i\n"), failAction[fail], buttonNum);
      haltSystem();
    }
    // TODO: validate if pin is correct to the current board
  }
};


Configuration::~Configuration()
{
  delete _relayOptions;
  delete _relaySensorId;
};


int Configuration::getRelayNum(int sensorId) const
{  
  if (sensorId > -1) {
    for (size_t relayNum = 0; relayNum < _relayConfig.size(); relayNum++) {
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


ButtonType Configuration::getButtonType(size_t buttonNum)
{
  if (buttonNum != _buttonNumInBuf) loadButtonConfigFromPROGMEM(buttonNum);
  return _buttonConfigEntryBuf.buttonType;
};


int Configuration::getButtonPin(size_t buttonNum)
{
  if (buttonNum != _buttonNumInBuf) loadButtonConfigFromPROGMEM(buttonNum);
  return _buttonConfigEntryBuf.buttonPin;
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
  PROGMEM_readAnything(&_relayConfig[relayNum], _relayConfigEntryBuf);
  _relayNumInBuf = relayNum;
};


void Configuration::loadButtonConfigFromPROGMEM(size_t buttonNum) 
{
  PROGMEM_readAnything(&_buttonConfig[buttonNum], _buttonConfigEntryBuf);
  _buttonNumInBuf = buttonNum;
};

