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
                                                 debounceInterval);
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

void ButtonService::setAction(size_t buttonNum, int clickRelayNum, int longclickRelayNum, int doubleclickRelayNum)
{
  _button[buttonNum]->setAction(clickRelayNum, longclickRelayNum, doubleclickRelayNum);
};


void ButtonService::attachPin(size_t buttonNum)
{
  _button[buttonNum]->attachPin();
};


int ButtonService::checkEvent(size_t buttonNum, unsigned long millis)
{
  return _button[buttonNum]->checkEvent(millis);
};


bool ButtonService::getRelayState(size_t buttonNum, bool relayState)
{
  return _button[buttonNum]->getRelayState(relayState);
};


void ButtonService::printDebug(size_t buttonNum)
{
  Serial << F("state=") << _button[buttonNum]->getState() << "; " << _configuration.getButtonDescription(buttonNum) << "\n";
};
