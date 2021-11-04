#pragma once

#include <RelayStateNotification.h>
#include <Configuration.h>
#ifdef ARDUINO
  #include <core/MyMessage.h>
#endif

namespace lkankowski {

  class MySensorsWrapper : public RelayStateNotification
  {
    public:
      MySensorsWrapper(Configuration &);
      virtual ~MySensorsWrapper();

      void notify(const uint8_t relayNum, bool state) override;
      void notifyWithId(const uint8_t relayNum, bool state, const uint8_t sensorId);
      void present() const;

    private:
      Configuration & _configuration;
    #ifdef ARDUINO
      MyMessage _myMessage;
    #endif
      bool * _reportAsSensor;
  };

};
