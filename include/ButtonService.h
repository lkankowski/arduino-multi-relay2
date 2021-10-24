#pragma once

#include <Button.h>
#include <Configuration.h>

namespace lkankowski {

class ButtonService
{
  public:
    ButtonService(Configuration &, unsigned int);
    ~ButtonService();

    // void setup(); 
    void setAction(size_t, int, int, int); 
    void attachPin(size_t);
    int checkEvent(size_t, unsigned long);
    bool getRelayState(size_t, bool);
    void printDebug(size_t);

  private:
    Configuration & _configuration;
    ButtonInterface ** _button;  //TODO: check shared_ptr
};

} //namespace
