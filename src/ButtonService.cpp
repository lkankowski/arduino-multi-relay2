#include <ButtonService.h>

using namespace lkankowski;


ButtonService::ButtonService(Configuration & configuration,
                             unsigned int debounceInterval,
                             ButtonCallbackInterface & buttonCallback)
  : _configuration(configuration)
  , _buttonCallback(buttonCallback)
{
  _button = new ButtonInterface*[_configuration.getButtonsCount()];
  for (size_t buttonNum = 0; buttonNum < _configuration.getButtonsCount(); buttonNum++)
  {
    _button[buttonNum] = ButtonInterface::create(_configuration.getButtonType(buttonNum),
                                                 _configuration.getButtonPin(buttonNum),
                                                 debounceInterval,
                                                 _configuration.getRelayNum(_configuration.getButtonClickAction(buttonNum)),
                                                 _configuration.getRelayNum(_configuration.getButtonLongClickAction(buttonNum)),
                                                 _configuration.getRelayNum(_configuration.getButtonDoubleClickAction(buttonNum)));
  }
};


ButtonService::~ButtonService()
{
  for (size_t buttonNum = 0; buttonNum < _configuration.getButtonsCount(); buttonNum++)
  {
    delete _button[buttonNum];
  }
  delete [] _button;
};


void ButtonService::attachPin(size_t buttonNum)
{
  _button[buttonNum]->attachPin();
};


void ButtonService::checkEventsAndDoActions(unsigned long millis)
{
  for (size_t buttonNum = 0; buttonNum < _configuration.getButtonsCount(); buttonNum++)
  {    
    int relayNum = _button[buttonNum]->checkEvent(millis);
    if (relayNum > -1) {
      // mono/bi-stable button toggles the relay, ding-dong/reed-switch switch to exact state
      #ifdef IGNORE_BUTTONS_START_MS
        if (millis > IGNORE_BUTTONS_START_MS) {
      #endif
          if (_button[buttonNum]->isToogle()) {
            _buttonCallback.toogleRelayState(relayNum, millis);
          } else {
            _buttonCallback.changeRelayState(relayNum, _button[buttonNum]->getRelayState(), millis);
          }
      #ifdef IGNORE_BUTTONS_START_MS
        }
      #endif
    }
  }
};


bool ButtonService::getRelayState(size_t buttonNum)
{
  return _button[buttonNum]->getRelayState();
};


void ButtonService::printDebug(size_t buttonNum)
{
  Serial << F("state=") << _button[buttonNum]->getState() << "; " << _configuration.getButtonDescription(buttonNum) << "\n";
};
