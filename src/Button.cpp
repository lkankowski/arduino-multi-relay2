#include <Button.h>


// static variables initialisation
unsigned long MyButton::_doubleclickInterval = 350;
unsigned long MyButton::_longclickInterval = 800;
uint8_t MyButton::_monoStableTrigger = 0;
#if defined(EXPANDER_PCF8574)
  PCF8574 * BounceExp::_expander = NULL;
#elif defined(EXPANDER_MCP23017)
  Adafruit_MCP23017 * BounceExp::_expander = NULL;
#endif


MyButton::MyButton()
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


void MyButton::initialize(int type, const char * desc) {
  _type = type & 0x0f;
  if (type & PRESSED_STATE_HIGH) _stateForPressed = true;
  _description = desc;
};


void MyButton::setAction(int clickRelayNum, int longclickRelayNum, int doubleclickRelayNum) {
  _clickRelayNum = clickRelayNum;
  _longclickRelayNum = longclickRelayNum;
  _doubleclickRelayNum = doubleclickRelayNum;
};


void MyButton::attachPin(int pin) {

  // No Expander support for buttons (de-bouncing)
  _physicalButton.attach(pin, INPUT_PULLUP); // HIGH state when button is not pushed
};


void MyButton::setEventIntervals(unsigned long doubleclickInterval, unsigned long longclickInterval) {
  _doubleclickInterval = doubleclickInterval;
  _longclickInterval = longclickInterval;
};


void MyButton::setMonoStableTrigger(unsigned char monoStableTrigger) {
  _monoStableTrigger = monoStableTrigger;
};


int MyButton::updateAndGetRelayNum() {

  bool isPinChanged = _physicalButton.update();
  int buttonPinState = _physicalButton.read();

  int buttonAction = getEvent(isPinChanged, buttonPinState);

  int relayNum = -1;
  if (isPinChanged && ((_type == DING_DONG) || (_type == REED_SWITCH))) {
      relayNum = _clickRelayNum;
  } else if (buttonAction & BUTTON_CLICK) {
    relayNum = _clickRelayNum;
    Serial.print(_description);
    Serial.print(" - Click for relay ");
    Serial.println(relayNum);
  } else if (buttonAction & BUTTON_DOUBLE_CLICK) {
    relayNum = _doubleclickRelayNum;
    Serial.print(_description);
    Serial.print(" - DoubleClick for relay ");
    Serial.println(relayNum);
  } else if (buttonAction & BUTTON_LONG_PRESS) {
    relayNum = _longclickRelayNum;
    Serial.print(_description);
    Serial.print(" - LongPress for relay ");
    Serial.println(relayNum);
  }
  return(relayNum);
};


bool MyButton::getRelayState(bool relayState) {
  
  bool result;
  if ((_type == MONO_STABLE) || (_type == BI_STABLE)) { // toggle relay
    result = !relayState;
  } else if (_type == DING_DONG) {
    result = getState();
  } else if (_type == REED_SWITCH) {
    result = !getState();
  }
  return(result);
};


int MyButton::getEvent(bool isPinChanged, int pinState) {

  int result = BUTTON_NO_EVENT;
  int activeLevel = pinState == (_type == REED_SWITCH ? ! _stateForPressed : _stateForPressed);

  bool hasLongClick = _longclickRelayNum != -1;
  bool hasDoubleClick = _doubleclickRelayNum != -1;


  #ifdef MY_DEBUG
    // Serial.print("# Button ");
    // Serial.print(buttonNum);
    // Serial.print(" changed to: ");
    // Serial.println(pinState);
  #endif

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
