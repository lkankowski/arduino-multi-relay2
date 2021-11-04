#include <MySensorsWrapper.h>
#ifdef ARDUINO
  #define MY_GATEWAY_SERIAL
  #include <core/MySensorsCore.h>
#endif

using namespace lkankowski;


MySensorsWrapper::MySensorsWrapper(Configuration & configuration)
  : _configuration(configuration)
{
  _reportAsSensor = new bool[_configuration.getRelaysCount()];
  for (size_t relayNum = 0; relayNum < _configuration.getRelaysCount(); relayNum++) {
    _reportAsSensor[relayNum] = false;
  }
  for (size_t buttonNum = 0; buttonNum < _configuration.getButtonsCount(); buttonNum++)
  {
    int clickActionRelayNum = _configuration.getRelayNum(_configuration.getButtonClickAction(buttonNum));
    if (((_configuration.getButtonType(buttonNum) & 0x0f) == REED_SWITCH) && (clickActionRelayNum > -1)) {
      _reportAsSensor[clickActionRelayNum] = true;
    }
  }
};


MySensorsWrapper::~MySensorsWrapper()
{
  delete [] _reportAsSensor;
};


inline void MySensorsWrapper::notify(const uint8_t relayNum, bool state)
{
  notifyWithId(relayNum, state, _configuration.getRelaySensorId(relayNum));
};


void MySensorsWrapper::notifyWithId(const uint8_t relayNum, bool state, const uint8_t sensorId)
{
  #ifdef ARDUINO
    _myMessage.setSensor(sensorId);
    _myMessage.setType(_reportAsSensor[relayNum] ? V_TRIPPED : V_STATUS);
    send(_myMessage.set(state));
  #endif
};


void MySensorsWrapper::present() const
{
  for (size_t relayNum = 0; relayNum < _configuration.getRelaysCount(); relayNum++) {
    #ifdef ARDUINO
      ::present(_configuration.getRelaySensorId(relayNum),
                _reportAsSensor[relayNum] ? S_DOOR : S_BINARY,
                _configuration.getRelayDescription(relayNum));
    #endif
  }
};
