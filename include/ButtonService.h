#pragma once

#include <Button.h>

namespace lkankowski {

typedef struct {
  int buttonPin;
  ButtonType buttonType;
  int clickRelayId;
  int longClickRelayId;
  int doubleClickRelayId;
  const char * const buttonDescription;
} ButtonConfigDef;

typedef struct {
  const ButtonConfigDef * config;
  int size;
} ButtonConfigRef;


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
