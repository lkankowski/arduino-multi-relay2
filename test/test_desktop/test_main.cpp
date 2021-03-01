#include <ArduinoAbstract.h>

#include <Relay.h>
#include <RelayService.h>
#include <Button.h>
#include <unity.h>
#include <iostream>
#include <string>


//#define USE_EXPANDER

using namespace std;
using namespace lkankowski;


#include <config.h>
#include <common.h>

Relay gRelay[gNumberOfRelays];
RelayService gRelayService(gNumberOfRelays, gRelay, gRelayConfig);
unsigned long millisForBounce2 = 0UL;


void setUp(void)
{
  Button::setEventIntervals(350, 800);
};


void test_config_relays()
{
  // check if relay ID is unique
  for (int relayNum = 0; relayNum < gNumberOfRelays-1; relayNum++) {
    for(int secondRelayNum = relayNum+1; secondRelayNum < gNumberOfRelays; secondRelayNum++) {
//      Serial.println(string(gRelayConfig[relayNum].sensorId) + "==" + gRelayConfig[secondRelayNum].sensorId);
      TEST_ASSERT_NOT_EQUAL_MESSAGE(gRelayConfig[relayNum].sensorId,
                                    gRelayConfig[secondRelayNum].sensorId,
                                    (string("Relay id not unique: ") + to_string(relayNum) + " & " + to_string(secondRelayNum)).c_str());
    }
  }
  // validate if pin is correct to the current board
  #if defined(BOARD_TARGET_ATMEGA2560)
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      int pin = gRelayConfig[relayNum].relayPin;
      if (pin >= 0) { // exclude virtual relays
        if (pin & 0xff00) { //exclude expander pins
          TEST_ASSERT_LESS_THAN_MESSAGE(NUM_DIGITAL_PINS,
                                        pin,
                                        (string("Pin number is greater than number of digital pins: ") + to_string(pin)).c_str());
        }
        TEST_ASSERT_NOT_EQUAL_MESSAGE(SERIAL_PORT_RX,
                                      pin,
                                      (string("Pin conflicts with default RX pin: ") + to_string(pin)).c_str());
        TEST_ASSERT_NOT_EQUAL_MESSAGE(SERIAL_PORT_TX,
                                      pin,
                                      (string("Pin conflicts with default TX pin: ") + to_string(pin)).c_str());
        #if defined(USE_EXPANDER)
          TEST_ASSERT_NOT_EQUAL_MESSAGE(PIN_WIRE_SDA,
                                        pin,
                                        (string("Pin conflicts with default expander SDA pin: ") + to_string(pin)).c_str());
          TEST_ASSERT_NOT_EQUAL_MESSAGE(PIN_WIRE_SCL,
                                        pin,
                                        (string("Pin conflicts with default expander SCL pin: ") + to_string(pin)).c_str());
          if (pin & 0xff00) {
            TEST_ASSERT_MESSAGE((((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)),
                                (string("Configuration failed - expander no or number of pins out of range for button: ") + to_string(buttonNum)).c_str());
          }
        #endif
      }
    }
  #endif
};


void test_config_buttons()
{
  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
    int pin = gButtonConfig[buttonNum].buttonPin;
    TEST_ASSERT_GREATER_OR_EQUAL(-1, gRelayService.getRelayNum(gButtonConfig[buttonNum].clickRelayId));
    TEST_ASSERT_GREATER_OR_EQUAL(-1, gRelayService.getRelayNum(gButtonConfig[buttonNum].longClickRelayId));
    TEST_ASSERT_GREATER_OR_EQUAL(-1, gRelayService.getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
    if (gButtonConfig[buttonNum].clickRelayId > -1) {
      TEST_ASSERT_NOT_EQUAL(-1, gRelayService.getRelayNum(gButtonConfig[buttonNum].clickRelayId));
    }
    if (gButtonConfig[buttonNum].longClickRelayId > -1) {
      TEST_ASSERT_NOT_EQUAL(-1, gRelayService.getRelayNum(gButtonConfig[buttonNum].longClickRelayId));
    }
    if (gButtonConfig[buttonNum].doubleClickRelayId > -1) {
      TEST_ASSERT_NOT_EQUAL(-1, gRelayService.getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
    }

    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(0,
                                         pin,
                                         (string("Pin number must be greater than 0: ") + to_string(pin)).c_str());
    if (pin & 0xff00) { //exclude expander pins
      TEST_ASSERT_LESS_THAN_MESSAGE(NUM_DIGITAL_PINS,
                                    pin,
                                    (string("Pin number is greater than number of digital pins: ") + to_string(pin)).c_str());
    }
    TEST_ASSERT_NOT_EQUAL_MESSAGE(SERIAL_PORT_RX,
                                  pin,
                                  (string("Pin conflicts with default RX pin: ") + to_string(pin)).c_str());
    TEST_ASSERT_NOT_EQUAL_MESSAGE(SERIAL_PORT_TX,
                                  pin,
                                  (string("Pin conflicts with default TX pin: ") + to_string(pin)).c_str());
    #if defined(USE_EXPANDER)
      TEST_ASSERT_NOT_EQUAL_MESSAGE(PIN_WIRE_SDA,
                                    pin,
                                    (string("Pin conflicts with default expander SDA pin: ") + to_string(pin)).c_str());
      TEST_ASSERT_NOT_EQUAL_MESSAGE(PIN_WIRE_SCL,
                                    pin,
                                    (string("Pin conflicts with default expander SCL pin: ") + to_string(pin)).c_str());
      if (pin & 0xff00) {
        TEST_ASSERT_MESSAGE((((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)),
                            (string("Configuration failed - expander no or number of pins out of range for button: ") + to_string(buttonNum)).c_str());
      }
    #endif
  }
};


void test_relayservice()
{
  const RelayConfigDef relayConfig[] = {
    {0, 1, RELAY_TRIGGER_LOW, -1, "Lamp 1"},
    {5, 2, RELAY_TRIGGER_LOW, -1, "Lamp 2"},
    {3, 3, RELAY_TRIGGER_LOW  | RELAY_STARTUP_OFF, -1, "Lamp 3"},
    {5, 4, RELAY_TRIGGER_HIGH | RELAY_STARTUP_OFF, -1, "Lamp 4"},
  };
  int numberOfRelays = sizeof(relayConfig) / sizeof(RelayConfigDef);
  
  Relay * relays = new Relay[numberOfRelays];
  RelayService * relayService = new RelayService(numberOfRelays, relays, relayConfig);
  relayService->initialize(true); // reset eeprom

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, relayService->getRelayNum(0), "[1] Lamp 1 should be 0");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, relayService->getRelayNum(5), "[1] Lamp 2 should be 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, relayService->getRelayNum(3), "[1] Lamp 3 should be 2");
  
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, Pin::_state[3], "[2] Lamp 3 should have pin state HIGH when turned OFF");
  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW,  Pin::_state[4], "[2] Lamp 4 should have pin state LOW when turned OFF");

  relayService->changeState(2, true);
  relayService->changeState(3, true);
  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW,  Pin::_state[3], "[3] Lamp 3 should have pin state LOW when turned ON");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, Pin::_state[4], "[3] Lamp 4 should have pin state HIGH when turned ON");
};


void test_relay_startup_eeprom()
{
  const RelayConfigDef relayConfig[] = {
    {1, 11, RELAY_TRIGGER_LOW | RELAY_STARTUP_ON,  -1, "Lamp 1"},
    {2, 12, RELAY_TRIGGER_LOW | RELAY_STARTUP_OFF, -1, "Lamp 2"},
    {3, 13, RELAY_TRIGGER_LOW, -1, "Lamp 3"},
    {4, 14, RELAY_TRIGGER_LOW | RELAY_IMPULSE, -1, "Lamp 4"},
  };
  int numberOfRelays = sizeof(relayConfig) / sizeof(RelayConfigDef);
  
  Relay * relays = new Relay[numberOfRelays];
  RelayService * relayService = new RelayService(numberOfRelays, relays, relayConfig);
  relayService->initialize(true); // reset eeprom
  TEST_ASSERT_TRUE_MESSAGE(relays[0].getState(), "[1] Lamp 1 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relays[1].getState(), "[1] Lamp 2 should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(relays[2].getState(), "[1] Lamp 3 should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(relays[3].getState(), "[1] Lamp 4 should be OFF");
  
  relayService->changeState(0, false);
  relayService->changeState(1, true);
  relayService->changeState(2, true);
  relayService->changeState(3, true);
  TEST_ASSERT_FALSE_MESSAGE(relays[0].getState(), "[2] Lamp 1 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relays[1].getState(), "[2] Lamp 2 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relays[2].getState(), "[2] Lamp 3 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relays[3].getState(), "[2] Lamp 4 should be ON");

  delete relayService;
  delete relays;

  long dummy = -1L;

  Relay * relays2 = new Relay[numberOfRelays];
  RelayService * relayService2 = new RelayService(numberOfRelays, relays2, relayConfig);
  relayService2->initialize(false); // do not reset eeprom
  TEST_ASSERT_TRUE_MESSAGE(relays[0].getState(), "[3] Lamp 1 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relays[1].getState(), "[3] Lamp 2 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relays[2].getState(), "[3] Lamp 3 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relays[3].getState(), "[3] Lamp 4 (RELAY_IMPULSE) should be OFF");
};


void test_relay_impulse()
{
  const RelayConfigDef relayConfig[] = {
    {1, 11, RELAY_TRIGGER_LOW | RELAY_IMPULSE,  -1, "Lamp 1"}
  };
  int numberOfRelays = sizeof(relayConfig) / sizeof(RelayConfigDef);
  
  Relay * relays = new Relay[numberOfRelays];
  RelayService * relayService = new RelayService(numberOfRelays, relays, relayConfig);
  relayService->setImpulseInterval(250);
  relayService->initialize(true); // reset eeprom
  TEST_ASSERT_FALSE_MESSAGE(relays[0].getState(), "[1] Lamp 1 should be OFF");

  relayService->changeState(0, true, 1000UL);
  TEST_ASSERT_TRUE_MESSAGE(relays[0].getState(), "[2] Lamp 1 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relayService->isImpulsePending(), "[2] Lamp 1 should be isImpulsePending() == TRUE");

  TEST_ASSERT_FALSE_MESSAGE(relayService->impulseProcess(0, 1200UL), "[3] impulseProcess() should be FALSE");
  TEST_ASSERT_TRUE_MESSAGE(relays[0].getState(), "[3] Lamp 1 should be ON");
  
  TEST_ASSERT_TRUE_MESSAGE(relayService->impulseProcess(0, 1300UL), "[4] impulseProcess() should be TRUE");
  TEST_ASSERT_FALSE_MESSAGE(relays[0].getState(), "[4] Lamp 1 should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(relayService->isImpulsePending(), "[4] Lamp 1 should be isImpulsePending() == FALSE");
};


void test_relay_dependsOn()
{
  const RelayConfigDef relayConfig[] = {
    {1, 11, RELAY_TRIGGER_LOW, -1, "Lamp 1"},
    {2, 12, RELAY_TRIGGER_LOW, 4,  "Lamp 2"},
    {3, 13, RELAY_TRIGGER_LOW, 5,  "Lamp 3"},
    {4, 14, RELAY_TRIGGER_LOW, -1, "Power Supply"},
    {5, 15, RELAY_TRIGGER_LOW | RELAY_INDEPENDENT, -1, "Stairs light"}
  };
  int numberOfRelays = sizeof(relayConfig) / sizeof(RelayConfigDef);
  Relay relays[numberOfRelays];
  RelayService relayService(numberOfRelays, relays, relayConfig);
  relayService.initialize(true);
  relayService.changeState(1, true);
  TEST_ASSERT_TRUE_MESSAGE(relays[1].getState(), "Lamp 2 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relays[3].getState(), "Power Supply should be ON after power ON Lamp 2");

  bool isAnyDependentOn = relayService.turnOffDependent();
  TEST_ASSERT_TRUE_MESSAGE(isAnyDependentOn, "turnOffDependent() should return TRUE");

  relayService.changeState(1, false);
  TEST_ASSERT_FALSE_MESSAGE(relays[1].getState(), "Lamp 2 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relays[3].getState(), "Power Supply should be ON after power OFF Lamp 2");

  relayService.changeState(2, true);
  TEST_ASSERT_TRUE_MESSAGE(relays[2].getState(), "Lamp 3 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relays[4].getState(), "Stairs light should be ON after power ON Lamp 3");

  isAnyDependentOn = relayService.turnOffDependent();
  TEST_ASSERT_FALSE_MESSAGE(relays[3].getState(), "Power Supply should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(isAnyDependentOn, "turnOffDependent() should return FALSE");

  // RELAY_INDEPENDENT
  relayService.changeState(2, false);
  TEST_ASSERT_FALSE_MESSAGE(relays[2].getState(), "Lamp 3 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relays[4].getState(), "Stairs light should be ON after power OFF Lamp 3");
  isAnyDependentOn = relayService.turnOffDependent();
  TEST_ASSERT_TRUE_MESSAGE(relays[4].getState(), "Stairs light should be ON even after turnOffDependent()");
  TEST_ASSERT_FALSE_MESSAGE(isAnyDependentOn, "turnOffDependent() should return FALSE [2]");
};


void test_button_mono_only_click_when_pressed()
{
  Button button;
  Pin pin;
  Button::setMonoStableTrigger(LOW);
  button.initialize(MONO_STABLE, "Button 1");
  button.setAction(1, -1, -1);
  button.setDebounceInterval(50);
  pin.digitalWrite(1, HIGH);
  button.attachPin(1);

  millisForBounce2 = 0UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[1] updateAndGetRelayNum() should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(1), "[1] Button 1 should have pin state HIGH");

  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[2] updateAndGetRelayNum(0) should return -1");
  millisForBounce2 = 45UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[2] updateAndGetRelayNum(45) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW, pin.digitalRead(1), "[2] Button 1 should have pin state LOW");

  // well - should we test external library? Yes, maybe in future Bounce2 will be removed
  millisForBounce2 = 60UL;
  // after debounce interval first, run of "updateAndGetRelayNum" will only change state machine, and second invoke CLICK
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[3] updateAndGetRelayNum(60) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button.updateAndGetRelayNum(++millisForBounce2), "[3] updateAndGetRelayNum(61) should return 1");

  millisForBounce2 = 120UL;
  pin.digitalWrite(1, HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[4] updateAndGetRelayNum(120) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(++millisForBounce2), "[4] updateAndGetRelayNum(121) should return -1");
};


void test_button_mono_only_click_when_released()
{
  Button button;
  Pin pin;
  Button::setMonoStableTrigger(HIGH);
  button.initialize(MONO_STABLE, "Button 2");
  button.setAction(1, -1, -1);
  button.setDebounceInterval(50);
  pin.digitalWrite(1, HIGH);
  button.attachPin(1);

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(1), "[1] Button 1 should have pin state HIGH");
  millisForBounce2 = 0UL;
  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[2] updateAndGetRelayNum(0) should return -1");
  millisForBounce2 = 60UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[3] updateAndGetRelayNum(60) should return -1");
  millisForBounce2 = 120UL;
  pin.digitalWrite(1, HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[4] updateAndGetRelayNum(120) should return -1");
  millisForBounce2 = 180UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button.updateAndGetRelayNum(millisForBounce2), "[5] updateAndGetRelayNum(180) should return 1");
};


void test_button_mono_all()
{
  Button button;
  Pin pin;
  Button::setMonoStableTrigger(LOW);
  button.initialize(MONO_STABLE, "Button 3");
  button.setAction(1, 2, 3);
  button.setDebounceInterval(50);
  pin.digitalWrite(1, HIGH);
  button.attachPin(1);

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(1), "[1] Button 1 should have pin state HIGH");

  // CLICK
  millisForBounce2 = 0UL;
  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[2] updateAndGetRelayNum(0) should return -1");

  millisForBounce2 = 60UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[3] updateAndGetRelayNum(60) should return -1");

  millisForBounce2 = 120UL;
  pin.digitalWrite(1, HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[4] updateAndGetRelayNum(120) should return -1");
  millisForBounce2 = 500UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[4] updateAndGetRelayNum(500) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button.updateAndGetRelayNum(++millisForBounce2), "[4] updateAndGetRelayNum(501) should return 1");

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(1), "[4] Button 1 should have pin state HIGH");

  // DOUBLE-CLICK
  millisForBounce2 = 1000UL;
  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[5] updateAndGetRelayNum(1000) should return -1");
  millisForBounce2 = 1060UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[5] updateAndGetRelayNum(1060) should return -1");
  millisForBounce2 = 1120UL;
  pin.digitalWrite(1, HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[5] updateAndGetRelayNum(1120) should return -1");
  millisForBounce2 = 1180UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[5] updateAndGetRelayNum(1180) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(++millisForBounce2), "[5] updateAndGetRelayNum(1181) should return -1");
  
  millisForBounce2 = 1240UL;
  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[6] updateAndGetRelayNum(1240) should return -1");
  millisForBounce2 = 1300UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(3, button.updateAndGetRelayNum(millisForBounce2), "[6] updateAndGetRelayNum(1300) should return 3");
  millisForBounce2 = 1360UL;
  pin.digitalWrite(1, HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[6] updateAndGetRelayNum(1360) should return -1");
  millisForBounce2 = 1420UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[6] updateAndGetRelayNum(1420) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(1), "[6] Button 1 should have pin state HIGH");

  // LONG-PRESS
  millisForBounce2 = 2000UL;
  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[7] updateAndGetRelayNum(2000) should return -1");
  millisForBounce2 = 2060UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[7] updateAndGetRelayNum(2060) should return -1");
  millisForBounce2 = 2900UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, button.updateAndGetRelayNum(millisForBounce2), "[7] updateAndGetRelayNum(2900) should return 2");
  millisForBounce2 = 3000UL;
  pin.digitalWrite(1, HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[7] updateAndGetRelayNum(1180) should return -1");
};


void test_button_bi_only()
{
  Button button;
  Pin pin;
  button.initialize(BI_STABLE, "Button 4");
  button.setAction(1, -1, -1);
  button.setDebounceInterval(50);
  pin.digitalWrite(1, HIGH);
  button.attachPin(1);

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(1), "[1] Button 1 should have pin state HIGH");

  // CLICK
  millisForBounce2 = 0UL;
  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[2] updateAndGetRelayNum(0) should return -1");
  millisForBounce2 = 60UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[3] updateAndGetRelayNum(60) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button.updateAndGetRelayNum(++millisForBounce2), "[4] updateAndGetRelayNum(61) should return 1");
};


void test_button_bi_all()
{
  Button button;
  Pin pin;
  button.initialize(BI_STABLE, "Button 5");
  button.setAction(1, -1, 2);
  button.setDebounceInterval(50);
  pin.digitalWrite(1, HIGH);
  button.attachPin(1);

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(1), "[1] Button 1 should have pin state HIGH");

  // CLICK
  millisForBounce2 = 0UL;
  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[2] updateAndGetRelayNum(0) should return -1");
  millisForBounce2 = 100UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[3] updateAndGetRelayNum(100) should return -1");
  millisForBounce2 = 500UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button.updateAndGetRelayNum(millisForBounce2), "[4] updateAndGetRelayNum(500) should return 1");

  // DOUBLE-CLICK
  millisForBounce2 = 1000UL;
  pin.digitalWrite(1, HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[5] updateAndGetRelayNum(1000) should return -1");
  millisForBounce2 = 1060UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[6] updateAndGetRelayNum(1060) should return -1");
  millisForBounce2 = 1120UL;
  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[7] updateAndGetRelayNum(1120) should return -1");
  millisForBounce2 = 1180UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, button.updateAndGetRelayNum(millisForBounce2), "[8] updateAndGetRelayNum(1180) should return 2");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(++millisForBounce2), "[9] updateAndGetRelayNum(1181) should return -1");
};


void test_button_ding_dong_only()
{
  // TODO: Button::setMonoStableTrigger(HIGH); ????
  Button button;
  Pin pin;
  button.initialize(DING_DONG, "Button 6");
  button.setAction(1, -1, -1);
  button.setDebounceInterval(50);
  pin.digitalWrite(1, HIGH); // initially same state as MONO
  button.attachPin(1);

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(1), "[1] Button 1 should have pin state HIGH");

  // DING (pressed)
  millisForBounce2 = 0UL;
  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[2] updateAndGetRelayNum(0) should return -1");
  millisForBounce2 = 60UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button.updateAndGetRelayNum(millisForBounce2), "[3] updateAndGetRelayNum(60) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(++millisForBounce2), "[4] updateAndGetRelayNum(61) should return -1");

  // DONG (released)
  millisForBounce2 = 500UL;
  pin.digitalWrite(1, HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[5] updateAndGetRelayNum(500) should return -1");
  millisForBounce2 = 560UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button.updateAndGetRelayNum(millisForBounce2), "[6] updateAndGetRelayNum(560) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(++millisForBounce2), "[7] updateAndGetRelayNum(561) should return -1");
};


void test_button_reed_switch_only()
{
  // TODO: Button::setMonoStableTrigger(HIGH); ????
  Button button;
  Pin pin;
  button.initialize(REED_SWITCH, "Button 7");
  button.setAction(1, -1, -1);
  button.setDebounceInterval(50);
  pin.digitalWrite(1, LOW); // initially LOW - window closed
  button.attachPin(1);

  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW, pin.digitalRead(1), "[1] Button 1 should have pin state LOW");

  // OPEN (reed switch disconnected)
  millisForBounce2 = 100UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[2] updateAndGetRelayNum(100) should return -1");
  pin.digitalWrite(1, HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[3] updateAndGetRelayNum(100) should return -1");
  millisForBounce2 = 160UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button.updateAndGetRelayNum(millisForBounce2), "[4] updateAndGetRelayNum(160) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(++millisForBounce2), "[5] updateAndGetRelayNum(161) should return -1");

  // CLOSED (reed switch connected)
  millisForBounce2 = 500UL;
  pin.digitalWrite(1, LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(millisForBounce2), "[6] updateAndGetRelayNum(500) should return -1");
  millisForBounce2 = 560UL;
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button.updateAndGetRelayNum(millisForBounce2), "[6] updateAndGetRelayNum(560) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button.updateAndGetRelayNum(++millisForBounce2), "[7] updateAndGetRelayNum(561) should return -1");
};




void test_button_to_relay_state()
{
  Pin pin;

  Button buttonMono;
  buttonMono.initialize(MONO_STABLE, "Button 8");
  buttonMono.attachPin(1);

  TEST_ASSERT_FALSE_MESSAGE(buttonMono.getRelayState(true), "[1] getRelayState MONO");
  TEST_ASSERT_TRUE_MESSAGE(buttonMono.getRelayState(false), "[2] getRelayState MONO");

  Button buttonBi;
  buttonBi.initialize(BI_STABLE, "Button 9");
  buttonBi.attachPin(1);

  TEST_ASSERT_FALSE_MESSAGE(buttonBi.getRelayState(true), "[3] getRelayState BI");
  TEST_ASSERT_TRUE_MESSAGE(buttonBi.getRelayState(false), "[4] getRelayState BI");

  Button buttonDingDong;
  buttonDingDong.initialize(DING_DONG, "Button 10");
  buttonDingDong.attachPin(1);

  pin.digitalWrite(1, HIGH); // OFF
  millisForBounce2 =    0; buttonDingDong.updateAndGetRelayNum(millisForBounce2);
  millisForBounce2 += 100; buttonDingDong.updateAndGetRelayNum(millisForBounce2);
  TEST_ASSERT_FALSE_MESSAGE(buttonDingDong.getRelayState(false), "[5] getRelayState DING-DONG");
  pin.digitalWrite(1, LOW);  // ON
  millisForBounce2 += 100; buttonDingDong.updateAndGetRelayNum(millisForBounce2);
  millisForBounce2 += 100; buttonDingDong.updateAndGetRelayNum(millisForBounce2);
  TEST_ASSERT_TRUE_MESSAGE(buttonDingDong.getRelayState(false), "[6] getRelayState DING-DONG");
  pin.digitalWrite(1, HIGH); // OFF
  millisForBounce2 += 100; buttonDingDong.updateAndGetRelayNum(millisForBounce2);
  millisForBounce2 += 100; buttonDingDong.updateAndGetRelayNum(millisForBounce2);
  TEST_ASSERT_FALSE_MESSAGE(buttonDingDong.getRelayState(false), "[7] getRelayState DING-DONG");

  Button buttonReedSwitch;
  buttonReedSwitch.initialize(REED_SWITCH, "Button 11");
  buttonReedSwitch.attachPin(1);

  pin.digitalWrite(1, LOW); // CLOSED
  millisForBounce2 += 100; buttonReedSwitch.updateAndGetRelayNum(millisForBounce2);
  millisForBounce2 += 100; buttonReedSwitch.updateAndGetRelayNum(millisForBounce2);
  TEST_ASSERT_FALSE_MESSAGE(buttonReedSwitch.getRelayState(false), "[8] getRelayState REED-SWITCH");
  pin.digitalWrite(1, HIGH); // OPEN
  millisForBounce2 += 100; buttonReedSwitch.updateAndGetRelayNum(millisForBounce2);
  millisForBounce2 += 100; buttonReedSwitch.updateAndGetRelayNum(millisForBounce2);
  TEST_ASSERT_TRUE_MESSAGE(buttonReedSwitch.getRelayState(false), "[9] getRelayState REED-SWITCH");
  pin.digitalWrite(1, LOW); // CLOSED
  millisForBounce2 += 100; buttonReedSwitch.updateAndGetRelayNum(millisForBounce2);
  millisForBounce2 += 100; buttonReedSwitch.updateAndGetRelayNum(millisForBounce2);
  TEST_ASSERT_FALSE_MESSAGE(buttonReedSwitch.getRelayState(false), "[10] getRelayState REED-SWITCH");
};



int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_config_relays);
    RUN_TEST(test_config_buttons);
    RUN_TEST(test_relayservice);
    RUN_TEST(test_relay_startup_eeprom);
    RUN_TEST(test_relay_impulse);
    RUN_TEST(test_relay_dependsOn);
    RUN_TEST(test_button_mono_only_click_when_pressed);
    RUN_TEST(test_button_mono_only_click_when_released);
    RUN_TEST(test_button_mono_all);
    RUN_TEST(test_button_bi_only);
    RUN_TEST(test_button_bi_all);
    RUN_TEST(test_button_ding_dong_only);
    RUN_TEST(test_button_reed_switch_only);
    RUN_TEST(test_button_to_relay_state);

    return UNITY_END();
};
