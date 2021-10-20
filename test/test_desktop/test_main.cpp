#include <ArduinoAbstract.h>

#include <vector>
#include <Configuration.h>
#include <Relay.h>
#include <RelayService.h>
#include <Button.h>
#include <ButtonService.h>
#include <unity.h>
#include <iostream>
#include <string>


//#define USE_EXPANDER

using namespace std;
using namespace lkankowski;


#include <config.h>

Vector<const RelayConfigDef> gRelayConfigRef(gRelayConfig, sizeof(gRelayConfig) / sizeof(RelayConfigDef));
Vector<const ButtonConfigDef> gButtonConfigRef(gButtonConfig, sizeof(gButtonConfig) / sizeof(ButtonConfigDef));
Configuration gConfiguration(gRelayConfigRef, gButtonConfigRef);

//const RelayConfigRef gRelayConfigRef = {gRelayConfig, sizeof(gRelayConfig) / sizeof(RelayConfigDef)};

Eeprom gEeprom;
RelayService gRelayService(gConfiguration, gEeprom);


void setUp(void)
{
  ButtonInterface::setEventIntervals(350, 800);
};


void test_config_relays()
{
  // check if relay ID is unique
  for (int relayNum = 0; relayNum < gConfiguration.getRelaysCount()-1; relayNum++) {
    for(int secondRelayNum = relayNum+1; secondRelayNum < gConfiguration.getRelaysCount(); secondRelayNum++) {
//      Serial.println(string(gRelayConfig[relayNum].sensorId) + "==" + gRelayConfig[secondRelayNum].sensorId);
      TEST_ASSERT_NOT_EQUAL_MESSAGE(gRelayConfig[relayNum].sensorId,
                                    gRelayConfig[secondRelayNum].sensorId,
                                    (string("Relay id not unique: ") + to_string(relayNum) + " & " + to_string(secondRelayNum)).c_str());
    }
  }
  // validate if pin is correct to the current board
  #if defined(BOARD_TARGET_ATMEGA2560)
    for (int relayNum = 0; relayNum < gConfiguration.getRelaysCount(); relayNum++) {
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
  for (int buttonNum = 0; buttonNum < gConfiguration.getButtonsCount(); buttonNum++) {
    int pin = gButtonConfig[buttonNum].buttonPin;
    TEST_ASSERT_GREATER_OR_EQUAL(-1, gConfiguration.getRelayNum(gButtonConfig[buttonNum].clickRelayId));
    TEST_ASSERT_GREATER_OR_EQUAL(-1, gConfiguration.getRelayNum(gButtonConfig[buttonNum].longClickRelayId));
    TEST_ASSERT_GREATER_OR_EQUAL(-1, gConfiguration.getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
    if (gButtonConfig[buttonNum].clickRelayId > -1) {
      TEST_ASSERT_NOT_EQUAL(-1, gConfiguration.getRelayNum(gButtonConfig[buttonNum].clickRelayId));
    }
    if (gButtonConfig[buttonNum].longClickRelayId > -1) {
      TEST_ASSERT_NOT_EQUAL(-1, gConfiguration.getRelayNum(gButtonConfig[buttonNum].longClickRelayId));
    }
    if (gButtonConfig[buttonNum].doubleClickRelayId > -1) {
      TEST_ASSERT_NOT_EQUAL(-1, gConfiguration.getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
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
  Vector<const RelayConfigDef> relayConfigRef(relayConfig, sizeof(relayConfig) / sizeof(RelayConfigDef));
  Configuration configuration(relayConfigRef, gButtonConfigRef);

  RelayService relayService(configuration, gEeprom);
  relayService.initialize(true); // reset eeprom

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration.getRelayNum(0), "[1] Lamp 1 should be 0");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration.getRelayNum(5), "[1] Lamp 2 should be 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, configuration.getRelayNum(3), "[1] Lamp 3 should be 2");
  
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, FakePin::_state[3], "[2] Lamp 3 should have pin state HIGH when turned OFF");
  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW,  FakePin::_state[4], "[2] Lamp 4 should have pin state LOW when turned OFF");

  relayService.changeState(2, true, 0);
  relayService.changeState(3, true, 0);
  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW,  FakePin::_state[3], "[3] Lamp 3 should have pin state LOW when turned ON");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, FakePin::_state[4], "[3] Lamp 4 should have pin state HIGH when turned ON");
};


void test_relay_startup_eeprom()
{
  const RelayConfigDef relayConfig[] = {
    {1, 11, RELAY_TRIGGER_LOW | RELAY_STARTUP_ON,  -1, "Lamp 1"},
    {2, 12, RELAY_TRIGGER_LOW | RELAY_STARTUP_OFF, -1, "Lamp 2"},
    {3, 13, RELAY_TRIGGER_LOW, -1, "Lamp 3"},
    {4, 14, RELAY_TRIGGER_LOW | RELAY_IMPULSE, -1, "Lamp 4"},
  };
  Vector<const RelayConfigDef> relayConfigRef(relayConfig, sizeof(relayConfig) / sizeof(RelayConfigDef));
  Configuration configuration(relayConfigRef, gButtonConfigRef);
  
  RelayService relayService(configuration, gEeprom);
  relayService.initialize(true); // reset eeprom
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(0), "[1] Lamp 1 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(1), "[1] Lamp 2 should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(2), "[1] Lamp 3 should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(3), "[1] Lamp 4 should be OFF");
  
  relayService.changeState(0, false, 0);
  relayService.changeState(1, true, 0);
  relayService.changeState(2, true, 0);
  relayService.changeState(3, true, 0);
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(0), "[2] Lamp 1 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(1), "[2] Lamp 2 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(2), "[2] Lamp 3 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(3), "[2] Lamp 4 should be ON");

  long dummy = -1L;

  RelayService relayService2(configuration, gEeprom);
  relayService2.initialize(false); // do not reset eeprom
  TEST_ASSERT_TRUE_MESSAGE(relayService2.getState(0), "[3] Lamp 1 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relayService2.getState(1), "[3] Lamp 2 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relayService2.getState(2), "[3] Lamp 3 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relayService2.getState(3), "[3] Lamp 4 (RELAY_IMPULSE) should be OFF");
};


void test_relay_impulse()
{
  const RelayConfigDef relayConfig[] = {
    {1, 11, RELAY_TRIGGER_LOW | RELAY_IMPULSE,  -1, "Lamp 1"}
  };
  Vector<const RelayConfigDef> relayConfigRef(relayConfig, sizeof(relayConfig) / sizeof(RelayConfigDef));
  Configuration configuration(relayConfigRef, gButtonConfigRef);
  
  RelayService relayService(configuration, gEeprom);
  relayService.setImpulseInterval(250);
  relayService.initialize(true); // reset eeprom
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(0), "[1] Lamp 1 should be OFF");

  relayService.changeState(0, true, 1000UL);
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(0), "[2] Lamp 1 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relayService.isImpulsePending(), "[2] Lamp 1 should be isImpulsePending() == TRUE");

  TEST_ASSERT_FALSE_MESSAGE(relayService.impulseProcess(0, 1200UL), "[3] impulseProcess() should be FALSE");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(0), "[3] Lamp 1 should be ON");
  
  TEST_ASSERT_TRUE_MESSAGE(relayService.impulseProcess(0, 1300UL), "[4] impulseProcess() should be TRUE");
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(0), "[4] Lamp 1 should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(relayService.isImpulsePending(), "[4] Lamp 1 should be isImpulsePending() == FALSE");
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
  Vector<const RelayConfigDef> relayConfigRef(relayConfig, sizeof(relayConfig) / sizeof(RelayConfigDef));
  Configuration configuration(relayConfigRef, gButtonConfigRef);

  RelayService relayService(configuration, gEeprom);
  relayService.initialize(true);
  relayService.changeState(1, true, 0);
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(1), "Lamp 2 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(3), "Power Supply should be ON after power ON Lamp 2");

  bool isAnyDependentOn = relayService.turnOffDependent(0);
  TEST_ASSERT_TRUE_MESSAGE(isAnyDependentOn, "turnOffDependent() should return TRUE");

  relayService.changeState(1, false, 0);
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(1), "Lamp 2 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(3), "Power Supply should be ON after power OFF Lamp 2");

  relayService.changeState(2, true, 0);
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(2), "Lamp 3 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(4), "Stairs light should be ON after power ON Lamp 3");

  isAnyDependentOn = relayService.turnOffDependent(0);
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(3), "Power Supply should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(isAnyDependentOn, "turnOffDependent() should return FALSE");

  // RELAY_INDEPENDENT
  relayService.changeState(2, false, 0);
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(2), "Lamp 3 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(4), "Stairs light should be ON after power OFF Lamp 3");
  isAnyDependentOn = relayService.turnOffDependent(0);
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(4), "Stairs light should be ON even after turnOffDependent()");
  TEST_ASSERT_FALSE_MESSAGE(isAnyDependentOn, "turnOffDependent() should return FALSE [2]");
};


void test_switch_startup_pullup()
{
  FakePin pin(1);
  pin.digitalWrite(HIGH);
  HardwareSwitchInterface * hardwareSwitch = HardwareSwitchInterface::create(HardwareSwitchInterface::SWITCH_DEBOUNCED, 1, 50, LOW);
  hardwareSwitch->attachPin();

  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->getState(), "[1] Switch should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(0), "[2] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(10), "[3] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(100), "[4] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(200), "[5] No switch change");

  delete hardwareSwitch;
};


void test_switch_startup_pullup_bi_on()
{
  FakePin pin(1);
  pin.digitalWrite(LOW);
  HardwareSwitchInterface * hardwareSwitch = HardwareSwitchInterface::create(HardwareSwitchInterface::SWITCH_DEBOUNCED, 1, 50, LOW);
  hardwareSwitch->attachPin();

  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->getState(), "[1] Switch should be ON");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(0), "[2] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(10), "[3] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(100), "[4] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(200), "[5] No switch change");

  delete hardwareSwitch;
};


void test_switch_startup_pulldown()
{
  FakePin pin(1);
  pin.digitalWrite(LOW);
  HardwareSwitchInterface * hardwareSwitch = HardwareSwitchInterface::create(HardwareSwitchInterface::SWITCH_DEBOUNCED, 1, 50, HIGH);
  hardwareSwitch->attachPin();

  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->getState(), "[1] Switch should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(0), "[2] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(10), "[3] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(100), "[4] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(200), "[5] No switch change");

  delete hardwareSwitch;
};


void test_switch_low()
{
  FakePin pin(1);
  pin.digitalWrite(HIGH);
  HardwareSwitchInterface * hardwareSwitch = HardwareSwitchInterface::create(HardwareSwitchInterface::SWITCH_DEBOUNCED, 1, 50, LOW);
  hardwareSwitch->attachPin();

  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->getState(), "[1] Switch should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(0), "[2] No switch change");
  pin.digitalWrite(LOW);
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(10), "[3] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->getState(), "[4] Switch should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(59), "[5] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->getState(), "[6] Switch should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->update(60), "[7] Switch has changed");
  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->getState(), "[8] Switch should be ON");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(61), "[9] No switch change");
  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->getState(), "[10] Switch should be ON");
  pin.digitalWrite(HIGH);
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(100), "[11] No switch change");
  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->getState(), "[12] Switch should be ON");
  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->update(150), "[13] Switch has changed");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->getState(), "[14] Switch should be OFF");

  delete hardwareSwitch;
};


void test_switch_high()
{
  FakePin pin(1);
  pin.digitalWrite(LOW); // pull-down
  HardwareSwitchInterface * hardwareSwitch = HardwareSwitchInterface::create(HardwareSwitchInterface::SWITCH_DEBOUNCED, 1, 50, HIGH);
  hardwareSwitch->attachPin();

  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->getState(), "[1] Switch should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(0), "[2] No switch change");
  pin.digitalWrite(HIGH);
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(10), "[3] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->getState(), "[4] Switch should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(59), "[5] No switch change");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->getState(), "[6] Switch should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->update(60), "[7] Switch has changed");
  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->getState(), "[8] Switch should be ON");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(61), "[9] No switch change");
  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->getState(), "[10] Switch should be ON");
  pin.digitalWrite(LOW);
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->update(100), "[11] No switch change");
  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->getState(), "[12] Switch should be ON");
  TEST_ASSERT_TRUE_MESSAGE(hardwareSwitch->update(150), "[13] Switch has changed");
  TEST_ASSERT_FALSE_MESSAGE(hardwareSwitch->getState(), "[14] Switch should be OFF");

  delete hardwareSwitch;
};


void test_button_mono_only_click_when_pressed()
{
  FakePin pin(1);
  ButtonInterface * button = ButtonInterface::create(MONO_STABLE, 1, 50);
  MonoStableButton::clickTriggerWhenPressed(true);
  button->setAction(1, -1, -1);
  pin.digitalWrite(HIGH);
  button->attachPin();

  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[1] checkEvent() should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(), "[2] Button 1 should have pin state HIGH");

  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[3] checkEvent(0) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(45), "[4] checkEvent(45) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW, pin.digitalRead(), "[5] Button 1 should have pin state LOW");

  // after debounce interval first, run of "checkEvent" will only change state machine, and second invoke CLICK
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(60), "[6] checkEvent(60) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(61), "[7] checkEvent(61) should return 1");

  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(120), "[8] checkEvent(120) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(121), "[9] checkEvent(121) should return -1");

  delete button;
};


void test_button_mono_only_click_when_released()
{
  FakePin pin(1);
  ButtonInterface * button = ButtonInterface::create(MONO_STABLE, 1, 50);
  MonoStableButton::clickTriggerWhenPressed(false);
  button->setAction(1, -1, -1);
  pin.digitalWrite(HIGH);
  button->attachPin();

  button->checkEvent(0);
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(), "[1] Button 1 should have pin state HIGH");
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[2] checkEvent(0) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(60), "[3] checkEvent(60) should return -1");
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(120), "[4] debounce timer start");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(180), "[5] debounced --> state OFF (pull-up)");

  delete button;
};


void test_button_mono_all()
{
  FakePin pin(1);
  ButtonInterface * button = ButtonInterface::create(MONO_STABLE, 1, 50);
  MonoStableButton::clickTriggerWhenPressed(true);
  button->setAction(1, 2, 3);
  pin.digitalWrite(HIGH);
  button->attachPin();

  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[1] initial check");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(), "[2] Physical state HIGH");

  // CLICK
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[2] button pressed - debounce timer start");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(60), "[3] button pressed - debounced");

  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(120), "[4] button released - debounce timer start");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(500), "[5] button released - debounced - init state machine");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(501), "[6] state machine goes through click timeout");

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(), "[7] Button 1 should have pin state HIGH");

  // DOUBLE-CLICK
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1000), "[8] checkEvent(1000) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1060), "[9] checkEvent(1060) should return -1");
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1120), "[10] checkEvent(1120) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1180), "[11] checkEvent(1180) should return -1");
  //TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1181), "[12] checkEvent(1181) should return -1");
  
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1240), "[13] checkEvent(1240) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(3, button->checkEvent(1300), "[14] checkEvent(1300) should return 3");
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1360), "[15] checkEvent(1360) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1420), "[16] checkEvent(1420) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(), "[17] Button 1 should have pin state HIGH");

  // LONG-PRESS
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(2000), "[18] checkEvent(2000) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(2060), "[19] checkEvent(2060) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, button->checkEvent(2900), "[20] checkEvent(2900) should return 2");
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(3000), "[21] checkEvent(1180) should return -1");

  delete button;
};


void test_button_bi_only()
{
  FakePin pin(1);
  ButtonInterface * button = ButtonInterface::create(BI_STABLE, 1, 50);
  button->setAction(1, -1, -1);
  pin.digitalWrite(HIGH);
  button->attachPin();

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(), "[1] Button 1 should have pin state HIGH");

  // CLICK
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[2] checkEvent(0) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(60), "[3] checkEvent(60) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(61), "[4] checkEvent(61) should return 1");

  delete button;
};


void test_button_bi_all()
{
  FakePin pin(1);
  ButtonInterface * button = ButtonInterface::create(BI_STABLE, 1, 50);
  button->setAction(1, -1, 2);
  pin.digitalWrite(HIGH);
  button->attachPin();

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(), "[1] Button 1 should have pin state HIGH");

  // CLICK
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[2] checkEvent(0) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(100), "[3] checkEvent(100) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(500), "[4] checkEvent(500) should return 1");

  // DOUBLE-CLICK
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1000), "[5] checkEvent(1000) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1060), "[6] checkEvent(1060) should return -1");
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1120), "[7] checkEvent(1120) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, button->checkEvent(1180), "[8] checkEvent(1180) should return 2");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(1181), "[9] checkEvent(1181) should return -1");

  delete button;
};


void test_button_ding_dong_only()
{
  // TODO: ButtonInterface::clickTriggerWhenPressed(HIGH); ????
  FakePin pin(1);
  ButtonInterface * button = ButtonInterface::create(DING_DONG, 1, 50);
  button->setAction(1, -1, -1);
  pin.digitalWrite(HIGH); // initially same state as MONO
  button->attachPin();

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(), "[1] Button 1 should have pin state HIGH");

  // DING (pressed)
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[2] checkEvent(0) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(60), "[3] checkEvent(60) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(61), "[4] checkEvent(61) should return -1");

  // DONG (released)
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(500), "[5] checkEvent(500) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(560), "[6] checkEvent(560) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(561), "[7] checkEvent(561) should return -1");

  delete button;
};


void test_button_reed_switch_only()
{
  // TODO: ButtonInterface::clickTriggerWhenPressed(HIGH); ????
  FakePin pin(1);
  ButtonInterface * button = ButtonInterface::create(REED_SWITCH, 1, 50);
  button->setAction(1, -1, -1);
  pin.digitalWrite(LOW); // initially LOW - window closed
  button->attachPin();

  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW, pin.digitalRead(), "[1] Button 1 should have pin state LOW");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[2] checkEvent(0) should return -1");

  // OPEN (reed switch disconnected)
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(100), "[3] checkEvent(100) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(160), "[4] checkEvent(160) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(161), "[5] checkEvent(161) should return -1");

  // CLOSED (reed switch connected)
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(500), "[6] checkEvent(500) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(560), "[7] checkEvent(560) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(561), "[8] checkEvent(561) should return -1");

  delete button;
};


void test_button_to_relay_state()
{
  FakePin pin(1);
  ButtonInterface * buttonMono = ButtonInterface::create(MONO_STABLE, 1, 50);
  buttonMono->attachPin();

  TEST_ASSERT_FALSE_MESSAGE(buttonMono->getRelayState(true), "[1] getRelayState MONO");
  TEST_ASSERT_TRUE_MESSAGE(buttonMono->getRelayState(false), "[2] getRelayState MONO");

  delete buttonMono;

  ButtonInterface * buttonBi = ButtonInterface::create(BI_STABLE, 1, 50);
  buttonBi->attachPin();

  TEST_ASSERT_FALSE_MESSAGE(buttonBi->getRelayState(true), "[3] getRelayState BI");
  TEST_ASSERT_TRUE_MESSAGE(buttonBi->getRelayState(false), "[4] getRelayState BI");

  delete buttonBi;

  pin.digitalWrite(HIGH); // pullup - OFF
  ButtonInterface * buttonDingDong = ButtonInterface::create(DING_DONG, 1, 50);
  buttonDingDong->attachPin();

  buttonDingDong->checkEvent(0);
  buttonDingDong->checkEvent(100);
  TEST_ASSERT_FALSE_MESSAGE(buttonDingDong->getRelayState(false), "[5] getRelayState DING-DONG");
  pin.digitalWrite(LOW);  // ON
  buttonDingDong->checkEvent(200);
  buttonDingDong->checkEvent(300);
  TEST_ASSERT_TRUE_MESSAGE(buttonDingDong->getRelayState(false), "[6] getRelayState DING-DONG");
  pin.digitalWrite(HIGH); // OFF
  buttonDingDong->checkEvent(400);
  buttonDingDong->checkEvent(500);
  TEST_ASSERT_FALSE_MESSAGE(buttonDingDong->getRelayState(false), "[7] getRelayState DING-DONG");

  delete buttonDingDong;

  pin.digitalWrite(LOW); // CLOSED
  ButtonInterface * buttonReedSwitch = ButtonInterface::create(REED_SWITCH, 1, 50);
  buttonReedSwitch->attachPin();

  buttonReedSwitch->checkEvent(600);
  buttonReedSwitch->checkEvent(700);
  TEST_ASSERT_FALSE_MESSAGE(buttonReedSwitch->getRelayState(false), "[8] getRelayState REED-SWITCH");
  pin.digitalWrite(HIGH); // OPEN
  buttonReedSwitch->checkEvent(800);
  buttonReedSwitch->checkEvent(900);
  TEST_ASSERT_TRUE_MESSAGE(buttonReedSwitch->getRelayState(false), "[9] getRelayState REED-SWITCH");
  pin.digitalWrite(LOW); // CLOSED
  buttonReedSwitch->checkEvent(1000);
  buttonReedSwitch->checkEvent(1100);
  TEST_ASSERT_FALSE_MESSAGE(buttonReedSwitch->getRelayState(false), "[10] getRelayState REED-SWITCH");

  delete buttonReedSwitch;
};


void test_PinCreator_instance()
{
  TEST_ASSERT_NULL_MESSAGE(PinCreator::instance(), "PinCreator::instance() shoud be null and not dynamicaly alocated");
  PinCreator pinCreator;
  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "PinCreator::instance() shoud be null and not dynamicaly alocated");
};


int main(int argc, char **argv)
{
  // std::cout << "Hello!" << std::endl; 

  UNITY_BEGIN();

  RUN_TEST(test_config_relays);
  RUN_TEST(test_config_buttons);
  RUN_TEST(test_relayservice);
  RUN_TEST(test_relay_startup_eeprom);
  RUN_TEST(test_relay_impulse);
  RUN_TEST(test_relay_dependsOn);
  RUN_TEST(test_switch_startup_pullup);
  RUN_TEST(test_switch_startup_pullup_bi_on);
  RUN_TEST(test_switch_startup_pulldown);
  RUN_TEST(test_switch_low);
  RUN_TEST(test_switch_high);
  RUN_TEST(test_button_mono_only_click_when_pressed);
  RUN_TEST(test_button_mono_only_click_when_released);
  RUN_TEST(test_button_mono_all);
  RUN_TEST(test_button_bi_only);
  RUN_TEST(test_button_bi_all);
  RUN_TEST(test_button_ding_dong_only);
  RUN_TEST(test_button_reed_switch_only);
  RUN_TEST(test_button_to_relay_state);
  RUN_TEST(test_PinCreator_instance);

  return UNITY_END();
};
