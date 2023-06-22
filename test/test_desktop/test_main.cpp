#include <ArduinoAbstract.h>

#include <Configuration.h>
#include <Relay.h>
#include <RelayService.h>
#include <Button.h>
#include <ButtonService.h>
#include <MySensorsWrapper.h>
#include <unity.h>
#include <iostream>
#include <string>
#include <typeinfo>


// #define USE_EXPANDER

#if defined(BOARD_TARGET_ATMEGA2560)
  #define NUM_DIGITAL_PINS (70)
  #define IS_VALID_DIGITAL_PIN(pin) ((pin >= 0) && (pin < NUM_DIGITAL_PINS))
#elif defined (BOARD_TARGET_ATMEGA328)
  #define IS_VALID_DIGITAL_PIN(pin) ((pin >= 0) && (pin < NUM_DIGITAL_PINS))
#elif defined (BOARD_TARGET_ESP8266)
  #define IS_VALID_DIGITAL_PIN(pin) (((pin >= 0) && (pin < NUM_DIGITAL_PINS)) && !isFlashInterfacePin(pin))
#endif

using namespace std;
using namespace lkankowski;


#include <config.h>

const RelayConfigRef gRelayConfigRef = {gRelayConfig, sizeof(gRelayConfig) / sizeof(RelayConfigDef)};
const ButtonConfigRef gButtonConfigRef = {gButtonConfig, sizeof(gButtonConfig) / sizeof(ButtonConfigDef)};
Configuration gConfiguration(gRelayConfigRef, gButtonConfigRef);

Eeprom gEeprom;
MySensorsWrapper gMySensorsWrapper(gConfiguration);
RelayService gRelayService(gConfiguration, gEeprom, gMySensorsWrapper);


// void suiteSetUp(void) {};
// void setUp(void) {};
// void tearDown(void)  {};
// int suiteTearDown(int num_failures); 



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
        if (pin < 0x100) { //exclude expander pins
          TEST_ASSERT_LESS_THAN_MESSAGE(NUM_DIGITAL_PINS, pin, "Pin number is greater than number of digital pins");
        }
        TEST_ASSERT_NOT_EQUAL_MESSAGE(SERIAL_PORT_RX, pin, "Pin conflicts with default RX pin");
        TEST_ASSERT_NOT_EQUAL_MESSAGE(SERIAL_PORT_TX, pin, "Pin conflicts with default TX pin");
        #if defined(USE_EXPANDER)
          TEST_ASSERT_NOT_EQUAL_MESSAGE(PIN_WIRE_SDA, pin, "Pin conflicts with default expander SDA pin");
          TEST_ASSERT_NOT_EQUAL_MESSAGE(PIN_WIRE_SCL, pin, "Pin conflicts with default expander SCL pin");
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


void test_configuration()
{
  const RelayConfigDef relayConfig[] = {
    {0, 1, RELAY_TRIGGER_LOW, -1, "Lamp 1"},
    {5, 2, RELAY_TRIGGER_LOW, -1, "Lamp 2"},
    {3, 3, RELAY_TRIGGER_LOW  | RELAY_STARTUP_OFF, -1, "Lamp 3"},
    {5, 4, RELAY_TRIGGER_HIGH | RELAY_STARTUP_OFF, 3, "Lamp 4"},
  };
  const ButtonConfigDef buttonConfig[] = {
    {1, MONO_STABLE, 0, -1, -1, "Button 1"},
    {2, BI_STABLE,   5, -1, -1, "Button 2"},
    {3, DING_DONG,   3, -1, -1, "Button 3"},
    {4, REED_SWITCH | PRESSED_STATE_HIGH, 4, 7, 9, "Button 4"},
  };
  const ButtonConfigRef buttonConfigRef = {buttonConfig, sizeof(buttonConfig) / sizeof(ButtonConfigDef)};
  const RelayConfigRef relayConfigRef = {relayConfig, sizeof(relayConfig) / sizeof(RelayConfigDef)};
  Configuration configuration(relayConfigRef, buttonConfigRef);

  TEST_ASSERT_FALSE_MESSAGE(configuration.validate(), "[1] Test should fail because of lack of relay id '1', '2' and '4'");
  //TODO: expander pin validation

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration.getRelayNum(0), "[2] Lamp 1 should be 0");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration.getRelayNum(5), "[3] Lamp 2 should be 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, configuration.getRelayNum(3), "[4] Lamp 3 should be 2");
  
  TEST_ASSERT_EQUAL_INT_MESSAGE(4, configuration.getRelaysCount(), "[5] Relay config has 4 elements");
  TEST_ASSERT_EQUAL_INT_MESSAGE(4, configuration.getRelayPin(3), "[6] Relay pin should be 4");
  TEST_ASSERT_EQUAL_INT_MESSAGE(RELAY_TRIGGER_HIGH | RELAY_STARTUP_OFF, configuration.getRelayOptions(3),
                                "[7] Relay options should be RELAY_TRIGGER_HIGH | RELAY_STARTUP_OFF");
  TEST_ASSERT_EQUAL_INT_MESSAGE(3, configuration.getRelayDependsOn(3), "[8] Relay depandsOn should be 3");
  TEST_ASSERT_EQUAL_INT_MESSAGE(5, configuration.getRelaySensorId(3), "[9] Relay sensorId should be 5");
  TEST_ASSERT_EQUAL_STRING_MESSAGE("Lamp 4", configuration.getRelayDescription(3), "[10] Relay description should be \"Lamp 4\"");
  
  TEST_ASSERT_EQUAL_INT_MESSAGE(4, configuration.getButtonsCount(), "[11] Button count should be 4");
  TEST_ASSERT_EQUAL_INT_MESSAGE(4, configuration.getButtonPin(3), "[12] Button pin should be 4");
  TEST_ASSERT_EQUAL_INT_MESSAGE(REED_SWITCH | PRESSED_STATE_HIGH, configuration.getButtonType(3),
                                "[13] Button type should be REED_SWITCH | PRESSED_STATE_HIGH");
  TEST_ASSERT_EQUAL_STRING_MESSAGE("Button 4", configuration.getButtonDescription(3), "[14] Button description should be \"Button 4\"");
  
  TEST_ASSERT_EQUAL_INT_MESSAGE(4, configuration.getButtonClickAction(3), "[15] Button click action should be 4");
  TEST_ASSERT_EQUAL_INT_MESSAGE(7, configuration.getButtonLongClickAction(3), "[16] Button long click action should be 7");
  TEST_ASSERT_EQUAL_INT_MESSAGE(9, configuration.getButtonDoubleClickAction(3), "[17] Button double click action should be 9");
};


void test_virtual_pin()
{
  VirtualPin pin(1);
  
//  pin.pinMode(OUTPUT);  // mode currentlyis ignored
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(HIGH, pin.digitalRead(), "[1] Must be HIGH");
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(LOW, pin.digitalRead(), "[2] Must be LOW");
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(HIGH, pin.digitalRead(), "[3] Must be HIGH again");
};


void test_pin_creator()
{
  TEST_ASSERT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud be null and not dynamicaly alocated");

  #ifdef USE_EXPANDER
    PinCreator * pinCreator = new PinCreator(gExpander, gExpanderAddresses, sizeof(gExpanderAddresses));
  #else
    PinCreator * pinCreator = new PinCreator;
  #endif
  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[1] PinCreator now should be initialized");
  
  PinInterface * pin = pinCreator->create(-1);
  TEST_ASSERT_NOT_NULL_MESSAGE(pin, "[2] PinCreator should create an instance of a PinInterface");
  
  // (pin instanceOf VirtualPin) - NOT WORK :(
  // VirtualPin * v = dynamic_cast<VirtualPin *>(pin);
  // TEST_ASSERT_NOT_NULL_MESSAGE(v, "[3] PinCreator should create a VirtualPin instance");
  
  delete pin;
  delete pinCreator;
  
  TEST_ASSERT_NULL_MESSAGE(PinCreator::instance(), "[4] PinCreator::instance() shoud be null");
};


void test_relayservice()
{
  PinCreator pinCreator;
  const RelayConfigDef relayConfig[] = {
    {0, 1, RELAY_TRIGGER_LOW, -1, "Lamp 1"},
    {5, 2, RELAY_TRIGGER_LOW, -1, "Lamp 2"},
    {3, 3, RELAY_TRIGGER_LOW  | RELAY_STARTUP_OFF, -1, "Lamp 3"},
    {5, 4, RELAY_TRIGGER_HIGH | RELAY_STARTUP_OFF, -1, "Lamp 4"},
  };
  const RelayConfigRef relayConfigRef = {relayConfig, sizeof(relayConfig) / sizeof(RelayConfigDef)};
  Configuration configuration(relayConfigRef, gButtonConfigRef);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  RelayService relayService(configuration, gEeprom, gMySensorsWrapper);
  relayService.initialize(true); // reset eeprom

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, FakePin::_state[3], "[1] Lamp 3 should have pin state HIGH when turned OFF");
  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW,  FakePin::_state[4], "[2] Lamp 4 should have pin state LOW when turned OFF");

  relayService.changeRelayState(2, true, 0);
  relayService.changeRelayState(3, true, 0);
  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW,  FakePin::_state[3], "[3] Lamp 3 should have pin state LOW when turned ON");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, FakePin::_state[4], "[4] Lamp 4 should have pin state HIGH when turned ON");
};


void test_relay_startup_eeprom()
{
  PinCreator pinCreator;
  const RelayConfigDef relayConfig[] = {
    {1, 11, RELAY_TRIGGER_LOW | RELAY_STARTUP_ON,  -1, "Lamp 1"},
    {2, 12, RELAY_TRIGGER_LOW | RELAY_STARTUP_OFF, -1, "Lamp 2"},
    {3, 13, RELAY_TRIGGER_LOW, -1, "Lamp 3"},
    {4, 14, RELAY_TRIGGER_LOW | RELAY_IMPULSE, -1, "Lamp 4"},
  };
  const RelayConfigRef relayConfigRef = {relayConfig, sizeof(relayConfig) / sizeof(RelayConfigDef)};
  Configuration configuration(relayConfigRef, gButtonConfigRef);
  
  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  RelayService relayService(configuration, gEeprom, gMySensorsWrapper);
  relayService.initialize(true); // reset eeprom
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(0), "[1] Lamp 1 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(1), "[1] Lamp 2 should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(2), "[1] Lamp 3 should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(3), "[1] Lamp 4 should be OFF");
  
  relayService.changeRelayState(0, false, 0);
  relayService.changeRelayState(1, true, 0);
  relayService.changeRelayState(2, true, 0);
  relayService.changeRelayState(3, true, 0);
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(0), "[2] Lamp 1 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(1), "[2] Lamp 2 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(2), "[2] Lamp 3 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(3), "[2] Lamp 4 should be ON");

  long dummy = -1L;

  RelayService relayService2(configuration, gEeprom, gMySensorsWrapper);
  relayService2.initialize(false); // do not reset eeprom
  TEST_ASSERT_TRUE_MESSAGE(relayService2.getState(0), "[3] Lamp 1 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relayService2.getState(1), "[3] Lamp 2 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relayService2.getState(2), "[3] Lamp 3 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relayService2.getState(3), "[3] Lamp 4 (RELAY_IMPULSE) should be OFF");
};


void test_relay_impulse()
{
  PinCreator pinCreator;
  const RelayConfigDef relayConfig[] = {
    {1, 11, RELAY_TRIGGER_LOW | RELAY_IMPULSE,  -1, "Lamp 1"}
  };
  const RelayConfigRef relayConfigRef = {relayConfig, sizeof(relayConfig) / sizeof(RelayConfigDef)};
  Configuration configuration(relayConfigRef, gButtonConfigRef);
  
  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  RelayService relayService(configuration, gEeprom, gMySensorsWrapper);
  relayService.setImpulseInterval(250);
  relayService.initialize(true); // reset eeprom
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(0), "[1] Lamp 1 should be OFF");

  relayService.changeRelayState(0, true, 1000UL);
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(0), "[2] Lamp 1 should be ON");

  relayService.processImpulse(1200UL);
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(0), "[3] Lamp 1 should be ON");
  
  relayService.processImpulse(1300UL);
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(0), "[4] Lamp 1 should be OFF");
};


void test_relay_dependsOn()
{
  PinCreator pinCreator;
  const RelayConfigDef relayConfig[] = {
    {1, 11, RELAY_TRIGGER_LOW, 7, "Lamp 1"},
    {2, 12, RELAY_TRIGGER_LOW, 4,  "Lamp 2"},
    {3, 13, RELAY_TRIGGER_LOW, 5,  "Lamp 3"},
    {4, 14, RELAY_TRIGGER_LOW, -1, "Power Supply"},
    {5, 15, RELAY_TRIGGER_LOW | RELAY_INDEPENDENT, -1, "Stairs light"},
    {6, 13, RELAY_TRIGGER_LOW, 4,  "Lamp 5"},
    {7, 13, RELAY_TRIGGER_LOW, 4,  "Power Supply 2"},
  };
  // simulate previous states
  gEeprom.write(RELAY_STATE_STORAGE + 0, 0);
  gEeprom.write(RELAY_STATE_STORAGE + 1, 1);
  gEeprom.write(RELAY_STATE_STORAGE + 2, 0);
  gEeprom.write(RELAY_STATE_STORAGE + 3, 0);
  gEeprom.write(RELAY_STATE_STORAGE + 4, 0);
  gEeprom.write(RELAY_STATE_STORAGE + 5, 0);
  gEeprom.write(RELAY_STATE_STORAGE + 6, 0);

  const RelayConfigRef relayConfigRef = {relayConfig, sizeof(relayConfig) / sizeof(RelayConfigDef)};
  Configuration configuration(relayConfigRef, gButtonConfigRef);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  RelayService relayService(configuration, gEeprom, gMySensorsWrapper);
  relayService.initialize(false);

  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(1), "[1] Lamp 2 should be ON after startup");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(3), "[2] Power Supply should be ON after startup because Lamp 2 was ON");
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(6), "[3] Power Supply 2 should be OFF");

  relayService.changeRelayState(1, false, 0);
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(1), "[4] Lamp 2 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(3), "[5] Power Supply should be ON after power OFF Lamp 2");
  TEST_ASSERT_FALSE_MESSAGE(relayService.turnOffDependent(0), "[6] turnOffDependent() should return FALSE");
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(3), "[7] Power Supply should be OFF after turnOffDependent");

  relayService.changeRelayState(1, true, 0);
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(1), "[8] Lamp 2 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(3), "[9] Power Supply should be ON after power ON Lamp 2");

  bool isAnyDependentOn = relayService.turnOffDependent(0);
  TEST_ASSERT_TRUE_MESSAGE(isAnyDependentOn, "[10] turnOffDependent() should return TRUE");

  relayService.changeRelayState(1, false, 0);
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(1), "[11] Lamp 2 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(3), "[12] Power Supply should be ON after power OFF Lamp 2");

  relayService.changeRelayState(2, true, 0);
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(2), "[13] Lamp 3 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(4), "[14] Stairs light should be ON after power ON Lamp 3");

  isAnyDependentOn = relayService.turnOffDependent(0);
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(3), "[15] Power Supply should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(isAnyDependentOn, "[16] turnOffDependent() should return FALSE");

  // RELAY_INDEPENDENT
  relayService.changeRelayState(2, false, 0);
  TEST_ASSERT_FALSE_MESSAGE(relayService.getState(2), "[17] Lamp 3 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(4), "[18] Stairs light should be ON after power OFF Lamp 3");
  isAnyDependentOn = relayService.turnOffDependent(0);
  TEST_ASSERT_TRUE_MESSAGE(relayService.getState(4), "[19] Stairs light should be ON even after turnOffDependent()");
  TEST_ASSERT_FALSE_MESSAGE(isAnyDependentOn, "[20] turnOffDependent() should return FALSE");
};


void test_switch_startup_pullup()
{
  PinCreator pinCreator;
  FakePin pin(1);
  pin.digitalWrite(HIGH);
  
  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
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
  PinCreator pinCreator;
  FakePin pin(1);
  pin.digitalWrite(LOW);
  
  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
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
  PinCreator pinCreator;
  FakePin pin(1);
  pin.digitalWrite(LOW);
  
  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
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
  PinCreator pinCreator;
  FakePin pin(1);
  pin.digitalWrite(HIGH);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
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
  PinCreator pinCreator;
  FakePin pin(1);
  pin.digitalWrite(LOW); // pull-down

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
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
  PinCreator pinCreator;
  FakePin pin(1);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  ButtonInterface * button = ButtonInterface::create(MONO_STABLE, 1, 50, 1, -1, -1);
  MonoStableButton::clickTriggerWhenPressed(true);
  pin.digitalWrite(HIGH);
  button->attachPin();

  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[1] checkEvent() should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(), "[2] Button 1 should have pin state HIGH");

  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(0), "[3] checkEvent(0) should return -1 (debouncing start)");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(45), "[4] checkEvent(45) should return -1 (inside debouncing)");
  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW, pin.digitalRead(), "[5] Button 1 should have pin state LOW");

  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(61), "[6] checkEvent(61) should return 1 (after debouncing)");

  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(120), "[7] checkEvent(120) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(121), "[8] checkEvent(121) should return -1");

  delete button;
};


void test_button_mono_only_click_when_released()
{
  PinCreator pinCreator;
  FakePin pin(1);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  ButtonInterface * button = ButtonInterface::create(MONO_STABLE, 1, 50, 1, -1, -1);
  MonoStableButton::clickTriggerWhenPressed(false);
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
  PinCreator pinCreator;
  FakePin pin(1);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  ButtonInterface * button = ButtonInterface::create(MONO_STABLE, 1, 50, 1, 2, 3);
  MonoStableButton::clickTriggerWhenPressed(true);
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
  PinCreator pinCreator;
  FakePin pin(1);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  ButtonInterface * button = ButtonInterface::create(BI_STABLE, 1, 50, 1, -1, -1);
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
  PinCreator pinCreator;
  FakePin pin(1);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  ButtonInterface * button = ButtonInterface::create(BI_STABLE, 1, 50, 1, -1, 2);
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
  PinCreator pinCreator;
  FakePin pin(1);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  ButtonInterface * button = ButtonInterface::create(DING_DONG, 1, 50, 1, -1, -1);
  pin.digitalWrite(HIGH); // initially same state as MONO
  button->attachPin();

  TEST_ASSERT_EQUAL_INT_MESSAGE(HIGH, pin.digitalRead(), "[1] Button 1 should have pin state HIGH");

  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(0), "[2] checkEvent(0) on first call should trigger getting exact state by returning relay num");
  TEST_ASSERT_FALSE_MESSAGE(button->getRelayState(), "[3] relay state initially should be OFF");

  // DING (pressed)
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(100), "[4] checkEvent(0) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(160), "[5] checkEvent(60) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(161), "[6] checkEvent(61) should return -1");

  // DONG (released)
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(500), "[7] checkEvent(500) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(560), "[8] checkEvent(560) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(561), "[9] checkEvent(561) should return -1");

  delete button;
};


void test_button_reed_switch_only()
{
  // TODO: ButtonInterface::clickTriggerWhenPressed(HIGH); ????
  PinCreator pinCreator;
  FakePin pin(1);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  ButtonInterface * button = ButtonInterface::create(REED_SWITCH, 1, 50, 1, -1, -1);
  pin.digitalWrite(LOW); // initially LOW - window closed
  button->attachPin();

  TEST_ASSERT_EQUAL_INT_MESSAGE(LOW, pin.digitalRead(), "[1] Button 1 should have pin state LOW");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(0), "[2] checkEvent(0) on first call should trigger getting exact state by returning relay num");
  TEST_ASSERT_FALSE_MESSAGE(button->getRelayState(), "[3] relay state initially should be OFF");

  // OPEN (reed switch disconnected)
  pin.digitalWrite(HIGH);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(100), "[4] checkEvent(100) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(160), "[5] checkEvent(160) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(161), "[6] checkEvent(161) should return -1");

  // CLOSED (reed switch connected)
  pin.digitalWrite(LOW);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(500), "[7] checkEvent(500) should return -1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, button->checkEvent(560), "[8] checkEvent(560) should return 1");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, button->checkEvent(561), "[9] checkEvent(561) should return -1");

  delete button;
};


void test_button_to_relay_state()
{
  PinCreator pinCreator;
  FakePin pin(1);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  ButtonInterface * buttonMono = ButtonInterface::create(MONO_STABLE, 1, 50, 1, -1, -1);
  buttonMono->attachPin();

  TEST_ASSERT_TRUE_MESSAGE(buttonMono->isToogle(), "[1] isToogle MONO");
  TEST_ASSERT_FALSE_MESSAGE(buttonMono->getRelayState(), "[2] getRelayState MONO");

  delete buttonMono;

  ButtonInterface * buttonBi = ButtonInterface::create(BI_STABLE, 1, 50, 1, -1, -1);
  buttonBi->attachPin();

  TEST_ASSERT_TRUE_MESSAGE(buttonBi->isToogle(), "[3] isToogle BI");
  TEST_ASSERT_FALSE_MESSAGE(buttonBi->getRelayState(), "[4] getRelayState BI");

  delete buttonBi;

  pin.digitalWrite(HIGH); // pullup - OFF
  ButtonInterface * buttonDingDong = ButtonInterface::create(DING_DONG, 1, 50, 1, -1, -1);
  buttonDingDong->attachPin();

  TEST_ASSERT_FALSE_MESSAGE(buttonDingDong->isToogle(), "[5] isToogle DING-DONG");
  buttonDingDong->checkEvent(0);
  buttonDingDong->checkEvent(100);
  TEST_ASSERT_FALSE_MESSAGE(buttonDingDong->getRelayState(), "[6] getRelayState DING-DONG");
  pin.digitalWrite(LOW);  // ON
  buttonDingDong->checkEvent(200);
  buttonDingDong->checkEvent(300);
  TEST_ASSERT_TRUE_MESSAGE(buttonDingDong->getRelayState(), "[7] getRelayState DING-DONG");
  pin.digitalWrite(HIGH); // OFF
  buttonDingDong->checkEvent(400);
  buttonDingDong->checkEvent(500);
  TEST_ASSERT_FALSE_MESSAGE(buttonDingDong->getRelayState(), "[8] getRelayState DING-DONG");

  delete buttonDingDong;

  pin.digitalWrite(LOW); // CLOSED
  ButtonInterface * buttonReedSwitch = ButtonInterface::create(REED_SWITCH, 1, 50, 1, -1, -1);
  buttonReedSwitch->attachPin();

  TEST_ASSERT_FALSE_MESSAGE(buttonReedSwitch->isToogle(), "[9] isToogle REED-SWITCH");
  buttonReedSwitch->checkEvent(600);
  buttonReedSwitch->checkEvent(700);
  TEST_ASSERT_FALSE_MESSAGE(buttonReedSwitch->getRelayState(), "[10] getRelayState REED-SWITCH");
  pin.digitalWrite(HIGH); // OPEN
  buttonReedSwitch->checkEvent(800);
  buttonReedSwitch->checkEvent(900);
  TEST_ASSERT_TRUE_MESSAGE(buttonReedSwitch->getRelayState(), "[11] getRelayState REED-SWITCH");
  pin.digitalWrite(LOW); // CLOSED
  buttonReedSwitch->checkEvent(1000);
  buttonReedSwitch->checkEvent(1100);
  TEST_ASSERT_FALSE_MESSAGE(buttonReedSwitch->getRelayState(), "[12] getRelayState REED-SWITCH");

  delete buttonReedSwitch;
};


class FakeRelayService : public ButtonCallbackInterface
{
  public:
    FakeRelayService(int relaysCount) {
      _changeRelayState = new int[relaysCount];
      _toogleRelayState = new bool[relaysCount];
      for (size_t relayNum = 0; relayNum < relaysCount; relayNum++) {
        _changeRelayState[relayNum] = -1;
        _toogleRelayState[relayNum] = false;
      }
    };
    virtual ~FakeRelayService() {
      delete [] _toogleRelayState;
      delete [] _changeRelayState;
    };

    bool changeRelayState(int relayNum, bool relayState, unsigned long millis) override {
      _changeRelayState[relayNum] = (int) relayState;
      return false;
    };
    bool toogleRelayState(int relayNum, unsigned long millis) override {
      _toogleRelayState[relayNum] = true;
      return false;
    };
  
    int * _changeRelayState;
    bool * _toogleRelayState;
};


void test_buttonservice()
{
  PinCreator pinCreator;
  const ButtonConfigDef buttonConfig[] = {
    {1, MONO_STABLE, 1, -1, -1, "Button 1"},
    {2, BI_STABLE,   2, -1, -1, "Button 2"},
    {3, DING_DONG,   3, -1, -1, "Button 3"},
    {4, REED_SWITCH, 4, -1, -1, "Button 4"},
  };
  const ButtonConfigRef buttonConfigRef = {buttonConfig, sizeof(buttonConfig) / sizeof(ButtonConfigDef)};
  Configuration configuration(gRelayConfigRef, buttonConfigRef);

  TEST_ASSERT_NOT_NULL_MESSAGE(PinCreator::instance(), "[0] PinCreator::instance() shoud exists");
  FakeRelayService fakeRelayService(buttonConfigRef.size);
  ButtonService buttonService(configuration, 50, fakeRelayService);
  FakePin::_state[1] = HIGH; //PULL-UP
  FakePin::_state[2] = HIGH; //PULL-UP
  FakePin::_state[3] = HIGH; //PULL-UP - LOW when pressed
  FakePin::_state[4] = LOW;  //NC - Window closed
  buttonService.attachPins();

  buttonService.checkEventsAndDoActions(1UL);
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, fakeRelayService._changeRelayState[0], "[1] changeRelayState should not be called for MONO_STABLE on first run");
  TEST_ASSERT_EQUAL_INT_MESSAGE(-1, fakeRelayService._changeRelayState[1], "[2] changeRelayState should not be called for BI_STABLE on first run");
  TEST_ASSERT_EQUAL_INT_MESSAGE(0, fakeRelayService._changeRelayState[2], "[3] changeRelayState should be called for DING_DONG on first run");
  TEST_ASSERT_EQUAL_INT_MESSAGE(0, fakeRelayService._changeRelayState[3], "[4] changeRelayState should be called for REED_SWITCH on first run");
  TEST_ASSERT_FALSE_MESSAGE(fakeRelayService._toogleRelayState[0], "[5] toogleRelayState should not be called for MONO_STABLE on first run");
  TEST_ASSERT_FALSE_MESSAGE(fakeRelayService._toogleRelayState[1], "[6] toogleRelayState should not be called for BI_STABLE on first run");
  TEST_ASSERT_FALSE_MESSAGE(fakeRelayService._toogleRelayState[2], "[7] toogleRelayState should not be called for DING_DONG on first run");
  TEST_ASSERT_FALSE_MESSAGE(fakeRelayService._toogleRelayState[3], "[8] toogleRelayState should not be called for REED_SWITCH on first run");
  
  FakePin::_state[1] = LOW;
  fakeRelayService._toogleRelayState[0] = false;
  buttonService.checkEventsAndDoActions(10UL);
  buttonService.checkEventsAndDoActions(100UL);
  TEST_ASSERT_TRUE_MESSAGE(fakeRelayService._toogleRelayState[0], "[9] toogleRelayState should be called for MONO_STABLE");

  FakePin::_state[3] = LOW;
  fakeRelayService._changeRelayState[2] = false;
  buttonService.checkEventsAndDoActions(10UL);
  buttonService.checkEventsAndDoActions(100UL);
  TEST_ASSERT_TRUE_MESSAGE(fakeRelayService._changeRelayState[2], "[10] changeRelayState should be called for DING_DONG");

  TEST_ASSERT_FALSE_MESSAGE(buttonService.getRelayState(0), "[11] getRelayState should always be FALSE for MONO_STABLE");
  TEST_ASSERT_FALSE_MESSAGE(buttonService.getRelayState(1), "[12] getRelayState should always be FALSE for BI_STABLE");
  TEST_ASSERT_TRUE_MESSAGE(buttonService.getRelayState(2), "[13] getRelayState should be TRUE for DING_DONG");
  TEST_ASSERT_FALSE_MESSAGE(buttonService.getRelayState(3), "[14] getRelayState should be FALSE for REED_SWITCH");
};


int main(int argc, char **argv)
{
  ButtonInterface::setEventIntervals(350, 800);

  UNITY_BEGIN();

  RUN_TEST(test_config_relays);
  RUN_TEST(test_config_buttons);
  RUN_TEST(test_configuration);
  RUN_TEST(test_virtual_pin);
  RUN_TEST(test_pin_creator);
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
  RUN_TEST(test_buttonservice);

  return UNITY_END();
};
