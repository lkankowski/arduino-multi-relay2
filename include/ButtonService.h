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

    // void setup(); 
    // void setAction(size_t, int, int, int); 
    void attachPin(size_t);
    void checkEventsAndDoActions(unsigned long);
    // int checkEvent(int, unsigned long);
    bool getRelayState(size_t);
    void printDebug(size_t);

  private:
    Configuration & _configuration;
    ButtonInterface ** _button;  //TODO: check shared_ptr
    ButtonCallbackInterface & _buttonCallback;
};

} //namespace
