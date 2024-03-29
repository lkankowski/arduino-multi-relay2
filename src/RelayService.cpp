#include <RelayService.h>

using namespace lkankowski;


RelayService::RelayService(Configuration & configuration, EepromInterface & eeprom, RelayStateNotification & relayStateNotification)
  : _configuration(configuration)
  , _eeprom(eeprom)
  , _relayStateNotification(relayStateNotification)
  , _impulsePending(0)
  , _impulseInterval(250)
  , _isAnyDependentOn(false)
{
  _pin = new PinInterface*[_configuration.getRelaysCount()];
  _relays = new RelayPtr[_configuration.getRelaysCount()];
  for (size_t relayNum = 0; relayNum < _configuration.getRelaysCount(); relayNum++) {
    _pin[relayNum] = PinCreator::instance()->create(_configuration.getRelayPin(relayNum));
    _relays[relayNum] = new Relay(_pin[relayNum]);
  }
  _storeRelayToEEPROM = new bool[_configuration.getRelaysCount()];
  _relayIsImpulse = new bool[_configuration.getRelaysCount()];
  _relayImpulseStartMillis = new unsigned long[_configuration.getRelaysCount()];
  _relayDependsOn = new int[_configuration.getRelaysCount()];
  _isRelayDependent = new bool[_configuration.getRelaysCount()];
};


RelayService::~RelayService()
{
  delete _isRelayDependent;
  delete _relayDependsOn;
  delete _relayImpulseStartMillis;
  delete _relayIsImpulse;
  delete _storeRelayToEEPROM;
  for (size_t relayNum = 0; relayNum < _configuration.getRelaysCount(); relayNum++) {
    delete _relays[relayNum];
    delete _pin[relayNum];
  }
  delete [] _relays;
  delete [] _pin;
};


void RelayService::initialize(bool resetEepromState)
{
  int initialState[_configuration.getRelaysCount()];
  for (size_t relayNum = 0; relayNum < _configuration.getRelaysCount(); relayNum++) {
    _relays[relayNum]->attachPin();
    _relays[relayNum]->setTriggerMode(_configuration.getRelayOptions(relayNum) & RELAY_TRIGGER_HIGH);

    initialState[relayNum] = (_configuration.getRelayOptions(relayNum) & RELAY_STARTUP_ON) > 0;
    _storeRelayToEEPROM[relayNum] = (_configuration.getRelayOptions(relayNum) & (RELAY_IMPULSE | RELAY_STARTUP_MASK)) == 0;
    if (_storeRelayToEEPROM[relayNum]) {
      // Set relay to last known state (using eeprom storage)
      initialState[relayNum] = _eeprom.read(RELAY_STATE_STORAGE + relayNum) == 1; // 1 - true, 0 - false
    }
    if (_storeRelayToEEPROM[relayNum] && resetEepromState && initialState[relayNum]) {
        _eeprom.write(RELAY_STATE_STORAGE + relayNum, 0);
        initialState[relayNum] = false;
    }
    _relayIsImpulse[relayNum] = (_configuration.getRelayOptions(relayNum) & RELAY_IMPULSE) != 0;
    _relayImpulseStartMillis[relayNum] = 0UL;
    _relayDependsOn[relayNum] = (_configuration.getRelaySensorId(relayNum) != _configuration.getRelayDependsOn(relayNum))
                                ? _configuration.getRelayNum(_configuration.getRelayDependsOn(relayNum))
                                : -1;  // can not be self dependent
    _isRelayDependent[relayNum] = false;
  }
  // startup turn on dependents
  for (size_t relayNum = 0; relayNum < _configuration.getRelaysCount(); relayNum++) {
    if (_relayDependsOn[relayNum] != -1) {
      if ((_configuration.getRelayOptions(_relayDependsOn[relayNum]) & RELAY_INDEPENDENT) == 0) {
        _isRelayDependent[_relayDependsOn[relayNum]] = true;
        if (initialState[relayNum]) {
          _isAnyDependentOn = true;
          initialState[_relayDependsOn[relayNum]] = true;
        }
      }
    }
  }
  // set initial state
  for (size_t relayNum = 0; relayNum < _configuration.getRelaysCount(); relayNum++) {
    _relays[relayNum]->changeState(initialState[relayNum]);
  }
};


bool RelayService::changeRelayState(int relayNum, bool relayState, unsigned long millis)
{
  if (relayState && (_relayDependsOn[relayNum] != -1)) {
    changeRelayState(_relayDependsOn[relayNum], true, millis);
    _isAnyDependentOn = true;
  }
  #ifdef DEBUG_STARTUP
    Serial << F("# ") << millis << F(" Relay(") << _configuration.getRelaySensorId(relayNum)
           << F(")::changeState: old_state=") << _relays[relayNum]->getState()
           << F(", new_state=") << relayState << "\n";
  #endif
  bool stateHasChanged = _relays[relayNum]->changeState(relayState);

  if (_storeRelayToEEPROM[relayNum] && stateHasChanged) {
    _eeprom.write(RELAY_STATE_STORAGE + relayNum, (uint8_t) relayState);
  }

  if (_relayIsImpulse[relayNum] && stateHasChanged) {
    if (relayState) {
      _relayImpulseStartMillis[relayNum] = millis;
      _impulsePending++;
    } else {
      _relayImpulseStartMillis[relayNum] = 0UL;
      _impulsePending--;
    }
  }

  if (stateHasChanged) {
    _relayStateNotification.notify(relayNum, relayState);
  }
  return stateHasChanged;
};


bool RelayService::toogleRelayState(int relayNum, unsigned long millis)
{
  return changeRelayState(relayNum, ! getState(relayNum), millis);
};


void RelayService::processImpulse(unsigned long millis)
{
  if (_impulsePending > 0) {
    for (size_t relayNum = 0; relayNum < _configuration.getRelaysCount(); relayNum++) {
      if (_relayIsImpulse[relayNum] && _relayImpulseStartMillis[relayNum] > 0) {
        if ((millis > _relayImpulseStartMillis[relayNum]+_impulseInterval)
            || (millis < _relayImpulseStartMillis[relayNum])) {  // "millis()" overflow protection
          if (changeRelayState(relayNum, false, millis)) {
            _relayStateNotification.notify(_configuration.getRelaySensorId(relayNum), false);
          }
        }
      }
    }
  }
};


bool RelayService::turnOffDependent(unsigned long millis)
{
  if (_isAnyDependentOn) {
    _isAnyDependentOn = false;
    for (size_t relayNum = 0; relayNum < _configuration.getRelaysCount(); relayNum++) {
      if (_isRelayDependent[relayNum] && _relays[relayNum]->getState()) {
        bool allMasterTurnedOff = true;
        for (size_t masterRelayNum = 0; masterRelayNum < _configuration.getRelaysCount(); masterRelayNum++) {
          if ((masterRelayNum != relayNum) && (_relayDependsOn[masterRelayNum] == (int) relayNum) && _relays[masterRelayNum]->getState()) {
            allMasterTurnedOff = false;
            break;
          }
        }
        if (allMasterTurnedOff) {
          changeRelayState(relayNum, false, millis);
        } else {
          _isAnyDependentOn = true;
        }
      }
    }
  }
  return _isAnyDependentOn;
};


void RelayService::printDebug(int relayNum) const
{
  Serial << F("## Relay ") << _configuration.getRelaySensorId(relayNum)
         << F(": state=") << _relays[relayNum]->getState()
        #ifdef ARDUINO
         << F(", pin_state=") << ArduinoPin::digitalRead(_configuration.getRelayPin(relayNum))
        #endif
         << F(", store_eeprom=") << _storeRelayToEEPROM[relayNum]
         << F(", eeprom=") << _eeprom.read(RELAY_STATE_STORAGE + relayNum)
         << F(", DependsOn=") << (_relayDependsOn[relayNum] == -1 ? -1 : _configuration.getRelaySensorId(_relayDependsOn[relayNum]))
         << F(", ") << _configuration.getRelayDescription(relayNum) << "\n";
};
