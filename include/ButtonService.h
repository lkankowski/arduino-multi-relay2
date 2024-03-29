#pragma once

#include <Button.h>
#include <ButtonCallbackInterface.h>
#include <Configuration.h>

namespace lkankowski {

class ButtonService
{
  public:
    ButtonService(Configuration &, unsigned int, ButtonCallbackInterface & buttonCallback);
    ~ButtonService();

    void attachPins();
    void checkEventsAndDoActions(unsigned long);
    bool getRelayState(size_t) const;
    void printDebug(size_t) const;

  private:
    Configuration & _configuration;
    ButtonInterface ** _button;  //TODO: check shared_ptr
    ButtonCallbackInterface & _buttonCallback;
};

} //namespace
