#include <Button.h>

using namespace lkankowski;

// static variables initialisation
unsigned long ButtonInterface::_doubleclickInterval = 300;
unsigned long ButtonInterface::_longclickInterval = 800;
bool MonoStableButton::_clickTriggerWhenPressed = true;


ButtonInterface * ButtonInterface::create(ButtonType type,
                                          int pin,
                                          unsigned int debounceInterval,
                                          int clickActionId,
                                          int longClickActionId,
                                          int doubleClickActionId)
{
  HardwareSwitchInterface * switchHW =
    HardwareSwitchInterface::create(HardwareSwitchInterface::SWITCH_DEBOUNCED,
                                    pin,
                                    debounceInterval,
                                    (type & PRESSED_STATE_HIGH) ? HIGH : LOW);

  switch(type & 0x0f)
  {
    case MONO_STABLE: return new MonoStableButton(switchHW, clickActionId, longClickActionId, doubleClickActionId);
    case BI_STABLE:   return new BiStableButton(switchHW, clickActionId, doubleClickActionId);
    case DING_DONG:   return new DingDongButton(switchHW, clickActionId);
    case REED_SWITCH: return new ReedSwitch(switchHW, clickActionId);
  }
  delete switchHW;
  return nullptr;
};


ButtonInterface::ButtonInterface(HardwareSwitchInterface * switchHW)
    : _switch(switchHW)
    , _eventState(BTN_STATE_INITIAL)
    , _startStateMillis(0)
    , _clickActionId(-1)
{};


ButtonInterface::~ButtonInterface()
{
  delete _switch;
};


MonoStableButton::MonoStableButton(HardwareSwitchInterface * switchHW,
                                   int clickActionId,
                                   int longClickActionId,
                                   int doubleClickActionId)
  : ButtonInterface(switchHW)
  , _longClickActionId(longClickActionId)
  , _doubleClickActionId(doubleClickActionId)
{
  _clickActionId = clickActionId;
};


BiStableButton::BiStableButton(HardwareSwitchInterface * switchHW,
                               int clickActionId,
                               int doubleClickActionId)
  : ButtonInterface(switchHW)
  , _doubleClickActionId(doubleClickActionId)
{
  _clickActionId = clickActionId;
};


DingDongButton::DingDongButton(HardwareSwitchInterface * switchHW, int clickActionId)
  : ButtonInterface(switchHW)
{
  _clickActionId = clickActionId;
};


ReedSwitch::ReedSwitch(HardwareSwitchInterface * switchHW, int clickActionId)
  : ButtonInterface(switchHW)
{
  _clickActionId = clickActionId;
};


void ButtonInterface::attachPin()
{
  _switch->attachPin();
};


// static
void ButtonInterface::setEventIntervals(unsigned long doubleclickInterval, unsigned long longclickInterval)
{
  _doubleclickInterval = doubleclickInterval;
  _longclickInterval = longclickInterval;
};


// static
void MonoStableButton::clickTriggerWhenPressed(bool clickTriggerWhenPressed)
{
  _clickTriggerWhenPressed = clickTriggerWhenPressed;
};


int MonoStableButton::checkEvent(unsigned long millis)
{
  bool switchStateChanged = _switch->update(millis);
  int buttonAction = calculateEvent(switchStateChanged, millis);

  int relayNum = -1;
  if (buttonAction & BUTTON_CLICK) {
    relayNum = _clickActionId;
    #ifdef DEBUG_ACTION
      Serial << F("Click for relay ") << relayNum << "\n";
    #endif
  } else if (buttonAction & BUTTON_DOUBLE_CLICK) {
    relayNum = _doubleClickActionId;
    #ifdef DEBUG_ACTION
      Serial << F("DoubleClick for relay ") << relayNum << "\n";
    #endif
  } else if (buttonAction & BUTTON_LONG_PRESS) {
    relayNum = _longClickActionId;
    #ifdef DEBUG_ACTION
      Serial << F("LongPress for relay ") << relayNum << "\n";
    #endif
  }
  return relayNum;
};


int BiStableButton::checkEvent(unsigned long millis)
{
  bool switchStateChanged = _switch->update(millis);
  int buttonAction = calculateEvent(switchStateChanged, millis);

  int relayNum = -1;
  if (buttonAction & BUTTON_CLICK) {
    relayNum = _clickActionId;
    #ifdef DEBUG_ACTION
      Serial << F("Click for relay ") << relayNum << "\n";
    #endif
  } else if (buttonAction & BUTTON_DOUBLE_CLICK) {
    relayNum = _doubleClickActionId;
    #ifdef DEBUG_ACTION
      Serial << F("DoubleClick for relay ") << relayNum << "\n";
    #endif
  }

  return relayNum;
};


int DingDongButton::checkEvent(unsigned long millis)
{
  if (_switch->update(millis)) {
    return _clickActionId;
  }
  return -1;
};


int ReedSwitch::checkEvent(unsigned long millis)
{
  if (_switch->update(millis)) {
    return _clickActionId;
  }
  return -1;
};


int MonoStableButton::calculateEvent(bool switchStateChanged, unsigned long now)
{
  int result = BUTTON_NO_EVENT;
  bool currentState = _switch->getState();

  bool hasLongClick = _longClickActionId != -1;
  bool hasDoubleClick = _doubleClickActionId != -1;

  if (_eventState == BTN_STATE_INITIAL) { // waiting for change
    if (currentState) { //changed from switchStateChanged
      _startStateMillis = now;
      _eventState = BTN_STATE_1ST_PRESS;
      result = BUTTON_PRESSED;
    }
  } else if (_eventState == BTN_STATE_1ST_PRESS) { // waiting for 1st release
    if (!currentState) { //released
      if (!hasDoubleClick) {
        result = BUTTON_CLICK;
        _eventState = BTN_STATE_INITIAL;
      } else {
        _eventState = BTN_STATE_1ST_RELEASE;
      }
    } else { // still pressed
      if ((!hasDoubleClick) && (!hasLongClick) && (currentState == _clickTriggerWhenPressed)) { // no long/double-click action, do click
        result = BUTTON_CLICK | BUTTON_PRESSED;
        _eventState = BTN_STATE_RELEASE_WAIT;
      } else if (hasLongClick && ((now - _startStateMillis) > _longclickInterval)) {
        result = BUTTON_LONG_PRESS | BUTTON_PRESSED;
        _eventState = BTN_STATE_RELEASE_WAIT;
      } else {
        // Button was pressed down and timeout dind't occured - wait in this state
        result = BUTTON_PRESSED;
      }
    }
  } else if (_eventState == BTN_STATE_1ST_RELEASE) {
    // waiting for press the second time or timeout
    if ((now - _startStateMillis) > _doubleclickInterval) {
      // this was only a single short click
      result = BUTTON_CLICK;
      _eventState = BTN_STATE_INITIAL;
    } else if (currentState) { // pressed
      if (currentState == _clickTriggerWhenPressed) {
        result = BUTTON_DOUBLE_CLICK | BUTTON_PRESSED;
        _eventState = BTN_STATE_RELEASE_WAIT;
      } else {
        result = BUTTON_PRESSED;
        _eventState = BTN_STATE_2ND_PRESS;
      }
    }
  } else if (_eventState == BTN_STATE_2ND_PRESS) { // waiting for second release
    if (!currentState) { // released
      // this was a 2 click sequence.
      // should we check double-click timeout here?
      result = BUTTON_DOUBLE_CLICK;
      _eventState = BTN_STATE_INITIAL;
    }
  } else if (_eventState == BTN_STATE_RELEASE_WAIT) { // waiting for release after long press
    if (!currentState) { // released
      _eventState = BTN_STATE_INITIAL;
    }
  }
  return result;
};


int BiStableButton::calculateEvent(bool switchStateChanged, unsigned long now)
{
  int result = BUTTON_NO_EVENT;

  bool hasDoubleClick = _doubleClickActionId != -1;

  if (_eventState == BTN_STATE_INITIAL) { // waiting for change
    if (switchStateChanged) {
      _startStateMillis = now;
      _eventState = BTN_STATE_1ST_CHANGE_BI;
    }

  // BI_STABLE buttons only state
  } else if (_eventState == BTN_STATE_1ST_CHANGE_BI) { // waiting for next change
    // waiting for second change or timeout
    if (!hasDoubleClick || ((now - _startStateMillis) > _doubleclickInterval)) {
      // this was only a single short click
      result = BUTTON_CLICK;
      _eventState = BTN_STATE_INITIAL;
    } else if (switchStateChanged) {
      result = BUTTON_DOUBLE_CLICK;
      _eventState = BTN_STATE_INITIAL;
    }
  }
  return result;
};

int DingDongButton::calculateEvent(bool switchStateChanged, unsigned long now)
{
  return 0;
};

int ReedSwitch::calculateEvent(bool switchStateChanged, unsigned long now)
{
  return 0;
};

