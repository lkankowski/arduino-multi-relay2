#include <Configuration.h>
#include <ArduinoAbstract.h>

using namespace lkankowski;


Configuration::Configuration(const RelayConfigRef & relayConfig, const ButtonConfigRef & buttonConfig)
  : _relayConfig(relayConfig)
  , _buttonConfig(buttonConfig)
{
  // validate config
  #ifdef USE_EXPANDER
    //TODO: check if I2C pins are not used
    for (int relayNum = 0; relayNum < gRelayConfigRef.size; relayNum++) {
      int pin = gRelayConfig[relayNum].relayPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > sizeof(gExpanderAddresses)) || ((pin & 0xff) >= EXPANDER_PINS)) {
          Serial.println(String("Configuration failed - expander no or number of pins out of range for relay: ") + relayNum);
          delay(1000);
          assert(0);
        }
      }
    }
  #endif
  
  for (int buttonNum = 0; buttonNum < _buttonConfig.size; buttonNum++) {
    #ifdef USE_EXPANDER
      int pin = gButtonConfig[buttonNum].buttonPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > sizeof(gExpanderAddresses)) || ((pin & 0xff) >= EXPANDER_PINS)) {
          Serial.println(String("Configuration failed - expander no or number of pins out of range for button: ") + buttonNum);
          delay(1000);
          assert(0);
        }
      }
    #endif

    const char * failAction[] = {"OK", "click", "long-press", "double-click"};
    int fail = 0;
    if ((_buttonConfig.config[buttonNum].clickRelayId != -1) && (getRelayNum(_buttonConfig.config[buttonNum].clickRelayId) == -1)) fail = 1;
    if ((_buttonConfig.config[buttonNum].longClickRelayId != -1) && (getRelayNum(_buttonConfig.config[buttonNum].longClickRelayId) == -1)) fail = 2;
    if ((_buttonConfig.config[buttonNum].doubleClickRelayId != -1) && (getRelayNum(_buttonConfig.config[buttonNum].doubleClickRelayId) == -1)) fail = 3;
    if (fail) {
      Serial.print("Configuration failed - invalid '");
      Serial.print(failAction[fail]);
      Serial.print(" relay ID' for button: ");
      Serial.println(buttonNum);
      haltSystem();
    }
    // TODO: validate if pin is correct to the current board
  }
};


Configuration::~Configuration()
{
};


int Configuration::getRelayNum(int sensorId) const
{  
  if (sensorId > -1) {
    for (int relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
      if (_relayConfig.config[relayNum].sensorId == sensorId) return(relayNum);
    }
  }
  return(-1);
};

