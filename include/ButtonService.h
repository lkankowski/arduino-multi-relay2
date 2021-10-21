#pragma once

#include <Button.h>
#include <RelayCallbackInterface.h>
#include <Configuration.h>

namespace lkankowski {

class ButtonService
{
  public:
    ButtonService(Configuration &, unsigned int);
    ~ButtonService();

    // void setup(); 
    void setAction(int, RelayCallbackInterface &, RelayCallbackInterface &, RelayCallbackInterface &); 
    void attachPin(int);
    int checkEvent(int, unsigned long);
    bool getRelayState(int, bool);
    String toString(int);

  private:
    Configuration & _configuration;
    ButtonInterface ** _button;  //TODO: check shared_ptr
};

} //namespace
