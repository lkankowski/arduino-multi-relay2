#pragma once

namespace lkankowski {

  class RelayCallbackInterface
  {
    public:
      virtual bool changeRelayState(bool, unsigned long);
  };

};
