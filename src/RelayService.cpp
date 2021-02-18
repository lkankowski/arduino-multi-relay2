#include <RelayService.h>
#include <Arduino.h>
#include <EEPROM.h>

using namespace lkankowski;


RelayService::RelayService(const int numberOfRelays, Relay * relays, const RelayConfigDef * relayConfig)
  : _numberOfRelays(numberOfRelays)
  , _relays(relays)
  , _relayConfig(relayConfig)
  , _impulsePending(0)
  , _impulseInterval(250)
  , _isAnyDependent(false)
{
  _storeRelayToEEPROM = new bool[_numberOfRelays];
  _relayIsImpulse = new bool[_numberOfRelays];
  _relayImpulseStartMillis = new unsigned long[_numberOfRelays];
  _relayDependsOn = new int[_numberOfRelays];
  _isRelayDependent = new bool[_numberOfRelays];
};


void RelayService::initialize(bool resetEepromState)
{
  int initialState[_numberOfRelays];
  for (int relayNum = 0; relayNum < _numberOfRelays; relayNum++) {
    
    _relays[relayNum].initialize(_relayConfig[relayNum].sensorId, _relayConfig[relayNum].relayDescription);
    _relays[relayNum].attachPin(_relayConfig[relayNum].relayPin);
    _relays[relayNum].setTriggerMode(_relayConfig[relayNum].relayOptions & RELAY_TRIGGER_HIGH);

    initialState[relayNum] = (_relayConfig[relayNum].relayOptions & RELAY_STARTUP_ON) > 0;
    _storeRelayToEEPROM[relayNum] = (_relayConfig[relayNum].relayOptions & (RELAY_IMPULSE | RELAY_STARTUP_MASK)) == 0;
    if (_storeRelayToEEPROM[relayNum]) {
      // Set relay to last known state (using eeprom storage)
      initialState[relayNum] = EEPROM.read(RELAY_STATE_STORAGE + relayNum) == 1; // 1 - true, 0 - false
    }
    if (resetEepromState && initialState[relayNum]) {
        EEPROM.write(RELAY_STATE_STORAGE + relayNum, 0);
        initialState[relayNum] = false;
    }
    _relayIsImpulse[relayNum] = (_relayConfig[relayNum].relayOptions & RELAY_IMPULSE) != 0;
    _relayImpulseStartMillis[relayNum] = 0UL;
    _relayDependsOn[relayNum] = getRelayNum(_relayConfig[relayNum].dependsOn);
    _isRelayDependent[relayNum] = false;
  }
  // startup turn on dependents
  for (int relayNum = 0; relayNum < _numberOfRelays; relayNum++) {
    if (_relayDependsOn[relayNum] != -1) {
      if ((_relayConfig[_relayDependsOn[relayNum]].relayOptions & RELAY_INDEPENDENT) == 0) {
        _isRelayDependent[_relayDependsOn[relayNum]] = true;
        if (initialState[relayNum]) _isAnyDependent = true;
      }
      initialState[_relayDependsOn[relayNum]] = initialState[relayNum];
    }
  }
  // set initial state
  for (int relayNum = 0; relayNum < _numberOfRelays; relayNum++) {
    _relays[relayNum].changeState(initialState[relayNum]);
  }
};


bool RelayService::changeState(int relayNum, bool relayState)
{
  if (relayState && (_relayDependsOn[relayNum] != -1)) {
    changeState(_relayDependsOn[relayNum], true);
    _isAnyDependent = true;
  }
  bool stateHasChanged = _relays[relayNum].changeState(relayState);

  if (_storeRelayToEEPROM[relayNum] && stateHasChanged) {
    EEPROM.write(RELAY_STATE_STORAGE + relayNum, (uint8_t) relayState);
  }

  if (_relayIsImpulse[relayNum] && stateHasChanged) {
    if (relayState) {
      _relayImpulseStartMillis[relayNum] = millis();
      _impulsePending++;
    } else {
      _relayImpulseStartMillis[relayNum] = 0UL;
      _impulsePending--;
    }
  }

  return stateHasChanged;
};


bool RelayService::impulseProcess(int relayNum)
{
  if (_relayIsImpulse[relayNum] && _relayImpulseStartMillis[relayNum] > 0) {
    unsigned long currentMillis = millis();

    // the "|| (currentMillis < myRelayImpulseStart[i])" is for "millis()" overflow protection
    if ((currentMillis > _relayImpulseStartMillis[relayNum]+_impulseInterval) || (currentMillis < _relayImpulseStartMillis[relayNum])) {
      return(changeState(relayNum, false));
    }
  }
  return(false);
};


void RelayService::turnOffDependent()
{
  if (_isAnyDependent) {
    for (int relayNum = 0; relayNum < _numberOfRelays; relayNum++) {
      if (_isRelayDependent[relayNum] && _relays[relayNum].getState()) {
        bool allMasterTurnedOff = true;
        for (int masterRelayNum = 0; relayNum < _numberOfRelays; relayNum++) {
          if ((masterRelayNum != relayNum) && (_relayDependsOn[masterRelayNum] == relayNum) && ! _relays[masterRelayNum].getState()) {
            allMasterTurnedOff = false;
            break;
          }
        }
        if (allMasterTurnedOff) changeState(relayNum, false);
      }
    }
  }
};


int RelayService::getRelayNum(int sensorId)
{  
  if (sensorId > -1) {
    for (int relayNum = 0; relayNum < _numberOfRelays; relayNum++) {
      if (_relayConfig[relayNum].sensorId == sensorId) return(relayNum);
    }
  }
  return(-1);
}
