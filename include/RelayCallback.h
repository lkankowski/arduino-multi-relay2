#pragma once

#include <RelayCallbackInterface.h>

namespace lkankowski {

  class RelayService;

  class RelayCallback : public RelayCallbackInterface
  {
    public:
      RelayCallback() {};

      void setRelayNum(int relayNum);
      void static setRelayServiceInstance(RelayService * relayServiceInstance) { _relayServiceInstance = relayServiceInstance; };
      bool changeRelayState(bool state, unsigned long millis) override;

    private:
      int _relayNum = -1;
      static RelayService * _relayServiceInstance;
  };

};
