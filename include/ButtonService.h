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
    ButtonService(const ButtonConfigRef & gButtonConfig, unsigned int);
    ~ButtonService();

    void setAction(int, int, int, int); 
    void attachPin(int);
    int checkEvent(int, unsigned long loopStartMillis);
    bool getRelayState(int, bool);
    String toString(int);

  private:
    const ButtonConfigRef & _buttonConfig;
    const int _numberOfButtons;
    ButtonInterface ** _button;  //TODO: check shared_ptr
    PinInterface ** _pin;
};

} //namespace
