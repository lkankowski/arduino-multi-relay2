#include <RelayService.h>
//#include <iostream>

using namespace lkankowski;

//TODO: check if relayNum is lower than _relayConfig.size

RelayService::RelayService(const RelayConfigRef & relayConfig, EepromInterface & eeprom)
  : _relayConfig(relayConfig)
  , _eeprom(eeprom)
  , _impulsePending(0)
  , _impulseInterval(250)
  , _isAnyDependentOn(false)
{
  _pin = new PinInterface*[_relayConfig.size];
  _relays = new RelayPtr[_relayConfig.size];
  for (int relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
    _pin[relayNum] = PinCreator::create(relayConfig.config[relayNum].relayPin);
    _relays[relayNum] = new Relay(_pin[relayNum]);
  }
  _storeRelayToEEPROM = new bool[_relayConfig.size];
  _relayIsImpulse = new bool[_relayConfig.size];
  _relayImpulseStartMillis = new unsigned long[_relayConfig.size];
  _relayDependsOn = new int[_relayConfig.size];
  _isRelayDependent = new bool[_relayConfig.size];
};


RelayService::~RelayService()
{
  delete _isRelayDependent;
  delete _relayDependsOn;
  delete _relayImpulseStartMillis;
  delete _relayIsImpulse;
  delete _storeRelayToEEPROM;
  for (int relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
    delete _relays[relayNum];
    delete _pin[relayNum];
  }
  delete [] _relays;
  delete [] _pin;
};


void RelayService::initialize(bool resetEepromState)
{
  int initialState[_relayConfig.size];
  for (int relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
    
    _relays[relayNum]->initialize(_relayConfig.config[relayNum].sensorId, _relayConfig.config[relayNum].relayDescription);
    _relays[relayNum]->attachPin();
    _relays[relayNum]->setTriggerMode(_relayConfig.config[relayNum].relayOptions & RELAY_TRIGGER_HIGH);

    initialState[relayNum] = (_relayConfig.config[relayNum].relayOptions & RELAY_STARTUP_ON) > 0;
    _storeRelayToEEPROM[relayNum] = (_relayConfig.config[relayNum].relayOptions & (RELAY_IMPULSE | RELAY_STARTUP_MASK)) == 0;
    if (_storeRelayToEEPROM[relayNum]) {
      // Set relay to last known state (using eeprom storage)
      initialState[relayNum] = _eeprom.read(RELAY_STATE_STORAGE + relayNum) == 1; // 1 - true, 0 - false
    }
    if (_storeRelayToEEPROM[relayNum] && resetEepromState && initialState[relayNum]) {
        _eeprom.write(RELAY_STATE_STORAGE + relayNum, 0);
        initialState[relayNum] = false;
    }
    _relayIsImpulse[relayNum] = (_relayConfig.config[relayNum].relayOptions & RELAY_IMPULSE) != 0;
    _relayImpulseStartMillis[relayNum] = 0UL;
    _relayDependsOn[relayNum] = (_relayConfig.config[relayNum].sensorId != _relayConfig.config[relayNum].dependsOn)
                                ? getRelayNum(_relayConfig.config[relayNum].dependsOn)
                                : -1;
    _isRelayDependent[relayNum] = false;
  }
  // startup turn on dependents
  for (int relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
    if (_relayDependsOn[relayNum] != -1) {
      if ((_relayConfig.config[_relayDependsOn[relayNum]].relayOptions & RELAY_INDEPENDENT) == 0) {
        _isRelayDependent[_relayDependsOn[relayNum]] = true;
        if (initialState[relayNum]) _isAnyDependentOn = true;
      }
      initialState[_relayDependsOn[relayNum]] = initialState[relayNum];
    }
  }
  // set initial state
  for (int relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
    _relays[relayNum]->changeState(initialState[relayNum]);
  }
};


bool RelayService::changeState(int relayNum, bool relayState, unsigned long millis)
{
  if (relayState && (_relayDependsOn[relayNum] != -1)) {
    changeState(_relayDependsOn[relayNum], true, millis);
    _isAnyDependentOn = true;
  }
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

  return stateHasChanged;
};


bool RelayService::getState(int relayNum)
{
  return _relays[relayNum]->getState();
};

bool RelayService::impulseProcess(int relayNum, unsigned long millis)
{
  if (_relayIsImpulse[relayNum] && _relayImpulseStartMillis[relayNum] > 0) {

    // the "|| (millis < myRelayImpulseStart[i])" is for "millis()" overflow protection
    if ((millis > _relayImpulseStartMillis[relayNum]+_impulseInterval) || (millis < _relayImpulseStartMillis[relayNum])) {
      return changeState(relayNum, false, millis);
    }
  }
  return(false);
};


bool RelayService::turnOffDependent(unsigned long millis)
{
  if (_isAnyDependentOn) {
    _isAnyDependentOn = false;
    for (int relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
      if (_isRelayDependent[relayNum] && _relays[relayNum]->getState()) {
        bool allMasterTurnedOff = true;
        for (int masterRelayNum = 0; masterRelayNum < _relayConfig.size; masterRelayNum++) {
          if ((masterRelayNum != relayNum) && (_relayDependsOn[masterRelayNum] == relayNum) && _relays[masterRelayNum]->getState()) {
            allMasterTurnedOff = false;
            break;
          }
        }
        if (allMasterTurnedOff) {
          changeState(relayNum, false, millis);
        } else {
          _isAnyDependentOn = true;
        }
      }
    }
  }
  return _isAnyDependentOn;
};


int RelayService::getSensorId(int relayNum)
{
  return _relays[relayNum]->getSensorId();
};

int RelayService::getRelayNum(int sensorId)
{  
  if (sensorId > -1) {
    for (int relayNum = 0; relayNum < _relayConfig.size; relayNum++) {
      if (_relayConfig.config[relayNum].sensorId == sensorId) return(relayNum);
    }
  }
  return(-1);
};


const char * RelayService::getDescription(int relayNum)
{
  return _relays[relayNum]->getDescription();
};


String RelayService::toString(int relayNum)
{
  return String("## Relay ") + _relays[relayNum]->getSensorId()
          + ": state=" + _relays[relayNum]->getState()
          //+ ", pin_state=" + _pin.digitalRead()
          + ", store_eeprom=" + _storeRelayToEEPROM[relayNum]
          + ", eeprom=" + _eeprom.read(RELAY_STATE_STORAGE + relayNum)
          + ", DependsOn=" + _relayDependsOn[relayNum]
          + ", " + _relays[relayNum]->getDescription();
};
