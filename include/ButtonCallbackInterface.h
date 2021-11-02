#pragma once

namespace lkankowski {

  class ButtonCallbackInterface
  {
    public:
      virtual ~ButtonCallbackInterface() {};

      virtual bool changeRelayState(int, bool, unsigned long) = 0;
      virtual bool toogleRelayState(int, unsigned long) = 0;
  };

};
