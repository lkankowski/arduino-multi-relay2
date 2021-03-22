#include <Button.h>

using namespace lkankowski;

// static variables initialisation
unsigned long lkankowski::Button::_doubleclickInterval = 350;
unsigned long lkankowski::Button::_longclickInterval = 800;
uint8_t lkankowski::Button::_monoStableTrigger = 0;

#if defined(EXPANDER_PCF8574)
  PCF8574 * BounceExp::_expander = NULL;
#elif defined(EXPANDER_MCP23017)
  Adafruit_MCP23017 * BounceExp::_expander = NULL;
#endif


lkankowski::Button::Button()
    : _pin(0)
    , _type(MONO_STABLE)
    , _description(NULL)
    , _stateForPressed(false)
    , _clickRelayNum(-1)
    , _longclickRelayNum(-1)
    , _doubleclickRelayNum(-1)
    , _eventState(BTN_STATE_INITIAL)
    , _startStateMillis(0)
{};


void lkankowski::Button::initialize(int type, const char * desc) {
  _type = type & 0x0f;
  if (type & PRESSED_STATE_HIGH) _stateForPressed = true;
  _description = desc;
};


void lkankowski::Button::setAction(int clickRelayNum, int longclickRelayNum, int doubleclickRelayNum) {
  _clickRelayNum = clickRelayNum;
  _longclickRelayNum = longclickRelayNum;
  _doubleclickRelayNum = doubleclickRelayNum;
};


void lkankowski::Button::attachPin(int pin) {
  _physicalButton.attach(pin, INPUT_PULLUP); // HIGH state when button is not pushed
};


void lkankowski::Button::setEventIntervals(unsigned long doubleclickInterval, unsigned long longclickInterval) {
  _doubleclickInterval = doubleclickInterval;
  _longclickInterval = longclickInterval;
};


void lkankowski::Button::setMonoStableTrigger(unsigned char monoStableTrigger) {
  _monoStableTrigger = monoStableTrigger;
};


int lkankowski::Button::updateAndGetRelayNum() {

  bool isPinChanged = _physicalButton.update();
  int buttonPinState = _physicalButton.read();

  int buttonAction = getEvent(isPinChanged, buttonPinState);

  int relayNum = -1;
  if (isPinChanged && ((_type == DING_DONG) || (_type == REED_SWITCH))) {
      relayNum = _clickRelayNum;
  } else if (buttonAction & BUTTON_CLICK) {
    relayNum = _clickRelayNum;
    #ifdef DEBUG_ACTION
      printf_P(PSTR("%s - Click for relay %i\n"), _description, relayNum);
    #endif
  } else if (buttonAction & BUTTON_DOUBLE_CLICK) {
    relayNum = _doubleclickRelayNum;
    #ifdef DEBUG_ACTION
      printf_P(PSTR("%s - DoubleClick for relay %i"), _description, relayNum);
    #endif
  } else if (buttonAction & BUTTON_LONG_PRESS) {
    relayNum = _longclickRelayNum;
    #ifdef DEBUG_ACTION
      printf_P(PSTR("%s - LongPress for relay %i"), _description, relayNum);
    #endif
  }
  return(relayNum);
};


bool lkankowski::Button::getRelayState(bool relayState) {

  bool result;
  if ((_type == MONO_STABLE) || (_type == BI_STABLE)) { // toggle relay
    result = !relayState;
  } else if (_type == DING_DONG) {
    result = _physicalButton.read();
  } else if (_type == REED_SWITCH) {
    result = ! _physicalButton.read();
  }
  return(result);
};


int lkankowski::Button::getEvent(bool isPinChanged, int pinState) {

  int result = BUTTON_NO_EVENT;
  int activeLevel = pinState == (_type == REED_SWITCH ? ! _stateForPressed : _stateForPressed);

  bool hasLongClick = _longclickRelayNum != -1;
  bool hasDoubleClick = _doubleclickRelayNum != -1;

  unsigned long now = millis();

  if (_eventState == BTN_STATE_INITIAL) { // waiting for change
    if (isPinChanged) {
      _startStateMillis = now;
      if (_type == BI_STABLE) {
        _eventState = BTN_STATE_1ST_CHANGE_BI;
      } else {
        _eventState = BTN_STATE_1ST_PRESS;
        result = BUTTON_PRESSED;
      }
    }

  // BI_STABLE buttons only state
  } else if (_eventState == BTN_STATE_1ST_CHANGE_BI) { // waiting for next change
    // waiting for second change or timeout
    if (!hasDoubleClick || ((now - _startStateMillis) > _doubleclickInterval)) {
      // this was only a single short click
      result = BUTTON_CLICK;
      _eventState = BTN_STATE_INITIAL;
    } else if (isPinChanged) {
      result = BUTTON_DOUBLE_CLICK;
      _eventState = BTN_STATE_INITIAL;
    }

  } else if (_eventState == BTN_STATE_1ST_PRESS) { // waiting for 1st release

    if (!activeLevel) { //released
      if (!hasDoubleClick) {
        result = BUTTON_CLICK;
        _eventState = BTN_STATE_INITIAL;
      } else {
        _eventState = BTN_STATE_1ST_RELEASE;
      }

    } else { // still pressed
      if ((!hasDoubleClick) && (!hasLongClick) && (pinState == _monoStableTrigger)) { // no long/double-click action, do click (old behavior)
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

    } else if (activeLevel) { // pressed
      if (pinState == _monoStableTrigger) {
        result = BUTTON_DOUBLE_CLICK | BUTTON_PRESSED;
        _eventState = BTN_STATE_RELEASE_WAIT;
      } else {
        result = BUTTON_PRESSED;
        _eventState = BTN_STATE_2ND_PRESS;
      }
    }

  } else if (_eventState == BTN_STATE_2ND_PRESS) { // waiting for second release
    if (!activeLevel) { // released
      // this was a 2 click sequence.
      // should we check double-click timeout here?
      result = BUTTON_DOUBLE_CLICK;
      _eventState = BTN_STATE_INITIAL;
    }

  } else if (_eventState == BTN_STATE_RELEASE_WAIT) { // waiting for release after long press
    if (!activeLevel) { // released
      _eventState = BTN_STATE_INITIAL;
    }
  }

  if (isPinChanged) {
    result |= BUTTON_CHANGED;
  }
  return result;
};


String lkankowski::Button::toString() {

    return String("state=") + _physicalButton.read() + ", pin=" + _pin + "; " + _description;
};
