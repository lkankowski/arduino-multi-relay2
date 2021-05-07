#pragma once

#include <Button.h>
#include <Configuration.h>

namespace lkankowski {

class ButtonService
{
  public:
    ButtonService(const ButtonConfigRef &, unsigned int);
    ~ButtonService();

    // void setup(); 
    void setAction(int, int, int, int); 
    void attachPin(int);
    int checkEvent(int, unsigned long);
    bool getRelayState(int, bool);
    String toString(int);

  private:
    const ButtonConfigRef & _buttonConfig;
    ButtonInterface ** _button;  //TODO: check shared_ptr
};

} //namespace
