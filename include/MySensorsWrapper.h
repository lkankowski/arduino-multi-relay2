#pragma once

#include <RelayStateNotification.h>
#include <core/MyMessage.h>


namespace lkankowski {

  class MySensorsWrapper : public RelayStateNotification
  {
    public:
      MySensorsWrapper();

      void notify(const uint8_t sensorId, bool state, bool isSensor) override;

    private:
      MyMessage _myMessage;
  };

};
