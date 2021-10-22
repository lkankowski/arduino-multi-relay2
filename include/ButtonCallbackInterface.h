#pragma once

namespace lkankowski {

  class ButtonCallbackInterface
  {
    public:
      virtual bool changeRelayState(int, bool, unsigned long);
      virtual bool toogleRelayState(int, unsigned long);
  };

};
