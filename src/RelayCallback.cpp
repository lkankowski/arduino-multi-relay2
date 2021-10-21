#include <RelayCallback.h>

using namespace lkankowski;


// static
RelayService * RelayCallback::_relayServiceInstance = nullptr;


void RelayCallback::setRelayNum(int relayNum)
{
  ;
};


bool RelayCallback::changeRelayState(bool state, unsigned long millis) 
{
  return false; //_relayServiceInstance->changeState(state, _relayNum, millis);
};
