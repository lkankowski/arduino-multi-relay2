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
    // void setAction(int, int, int, int); 
    void attachPin(int);
    void checkEventsAndDoActions(unsigned long);
    // int checkEvent(int, unsigned long);
    bool getRelayState(int);
    String toString(int);

  private:
    Configuration & _configuration;
    ButtonInterface ** _button;  //TODO: check shared_ptr
    ButtonCallbackInterface & _buttonCallback;
};

} //namespace
