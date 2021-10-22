#include <MySensorsWrapper.h>
#define MY_GATEWAY_SERIAL
#include <core/MySensorsCore.h>

using namespace lkankowski;


MySensorsWrapper::MySensorsWrapper()
{};


void MySensorsWrapper::notify(const uint8_t sensorId, bool state, bool isSensor)
{
  _myMessage.setSensor(sensorId);
  _myMessage.setType(isSensor ? V_TRIPPED : V_STATUS);
  send(_myMessage.set(state));
};
