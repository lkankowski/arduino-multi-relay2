#include <ButtonService.h>

using namespace lkankowski;


ButtonService::ButtonService(Configuration & configuration, unsigned int debounceInterval)
  : _configuration(configuration)
{
  _button = new ButtonInterface*[_configuration.getButtonsCount()];
  for (size_t buttonNum = 0; buttonNum < _configuration.getButtonsCount(); buttonNum++)
  {
    _button[buttonNum] = ButtonInterface::create(_configuration.getButtonType(buttonNum),
                                                 _configuration.getButtonPin(buttonNum),
                                                 debounceInterval,
                                                 _configuration.getButtonLongClickAction(buttonNum) != -1,
                                                 _configuration.getButtonDoubleClickAction(buttonNum) != -1);
    // setAction(buttonNum,
    //           _configuration.getRelayNum(_configuration.getButtonClickAction(buttonNum)),
    //           _configuration.getRelayNum(_configuration.getButtonLongClickAction(buttonNum)),
    //           _configuration.getRelayNum(_configuration.getButtonDoubleClickAction(buttonNum)));
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


// void ButtonService::setup()
// {
//   for (size_t buttonNum = 0; buttonNum < _configuration.getButtonsCount(); buttonNum++)
//   {
//     gButtonService.setAction(buttonNum,
//                             gRelayService.getRelayNum(gButtonConfig[buttonNum].clickRelayId),
//                             gRelayService.getRelayNum(gButtonConfig[buttonNum].longClickRelayId),
//                             gRelayService.getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
//     gButtonService.attachPin(buttonNum);
//   }
// }; 

void ButtonService::setAction(int buttonNum,
                              RelayCallbackInterface & clickRelayNum,
                              RelayCallbackInterface & longclickRelayNum,
                              RelayCallbackInterface & doubleclickRelayNum)
{
  //_button[buttonNum]->setAction(clickRelayNum, longclickRelayNum, doubleclickRelayNum);
};


void ButtonService::attachPin(int buttonNum)
{
  _button[buttonNum]->attachPin();
};


int ButtonService::checkEvent(int buttonNum, unsigned long millis)
{
  return _button[buttonNum]->checkEvent(millis);
};


bool ButtonService::getRelayState(int buttonNum, bool relayState)
{
  return _button[buttonNum]->getRelayState(relayState);
};


String ButtonService::toString(int buttonNum)
{
  return String("state=") + _button[buttonNum]->getState() + "; " + _configuration.getButtonDescription(buttonNum);
};
