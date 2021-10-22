#pragma once

#include <stdint.h>

namespace lkankowski {

  class RelayStateNotification {
    public:
      virtual void notify(const uint8_t sensorId, bool state, bool isSensor);
  };

};
