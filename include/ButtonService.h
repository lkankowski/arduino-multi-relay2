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

    void attachPin(size_t);
    void checkEventsAndDoActions(unsigned long);
    bool getRelayState(size_t);
    void printDebug(size_t);

  private:
    Configuration & _configuration;
    ButtonInterface ** _button;  //TODO: check shared_ptr
    ButtonCallbackInterface & _buttonCallback;
};

} //namespace
