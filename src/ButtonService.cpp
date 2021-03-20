#include <ButtonService.h>

using namespace lkankowski;


ButtonService::ButtonService(const ButtonConfigRef & buttonConfig, unsigned int debounceInterval)
  : _buttonConfig(buttonConfig)
{
  _button = new ButtonInterface*[_buttonConfig.size];
  for (int buttonNum = 0; buttonNum < _buttonConfig.size; buttonNum++)
  {
    _button[buttonNum] = ButtonInterface::create(_buttonConfig.config[buttonNum].buttonType,
                                                 _buttonConfig.config[buttonNum].buttonPin,
                                                 debounceInterval,
                                                 _buttonConfig.config[buttonNum].buttonDescription);
  }
};


ButtonService::~ButtonService()
{
  for (int buttonNum = 0; buttonNum < _buttonConfig.size; buttonNum++)
  {
    delete _button[buttonNum];
  }
  delete [] _button;
};


// void ButtonService::setup()
// {
//   for (int buttonNum = 0; buttonNum < _buttonConfig.size; buttonNum++)
//   {
//     gButtonService.setAction(buttonNum,
//                             gRelayService.getRelayNum(gButtonConfig[buttonNum].clickRelayId),
//                             gRelayService.getRelayNum(gButtonConfig[buttonNum].longClickRelayId),
//                             gRelayService.getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
//     gButtonService.attachPin(buttonNum);
//   }
// }; 

void ButtonService::setAction(int buttonNum, int clickRelayNum, int longclickRelayNum, int doubleclickRelayNum)
{
  _button[buttonNum]->setAction(clickRelayNum, longclickRelayNum, doubleclickRelayNum);
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
  return _button[buttonNum]->toString();
};
