#pragma once

#include <stdint.h>

namespace lkankowski {

  class RelayStateNotification {
    public:
      virtual ~RelayStateNotification() {};

      virtual void notify(const uint8_t relayNum, bool state) = 0;
  };
};
