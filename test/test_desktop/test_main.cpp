#include <Arduino.h>
#include <iostream>

using namespace std;

//#define USE_EXPANDER

#if defined(BOARD_TARGET_ATMEGA2560)
  #undef LED_BUILTIN
  #undef A0
  #undef A1
  #undef A2
  #undef A3
  #undef A4
  #undef A5
  #undef A6
  #undef A7
  #undef A8
  #undef A9
  #define A0   (54)
  #define A1   (55)
  #define A2   (56)
  #define A3   (57)
  #define A4   (58)
  #define A5   (59)
  #define A6   (60)
  #define A7   (61)
  #define A8   (62)
  #define A9   (63)
  #define A10  (64)
  #define A11  (65)
  #define A12  (66)
  #define A13  (67)
  #define A14  (68)
  #define A15  (69)
  #define NUM_DIGITAL_PINS            70
  #define PIN_WIRE_SDA        (20)
  #define PIN_WIRE_SCL        (21)
  #define SERIAL_PORT_RX        (0)
  #define SERIAL_PORT_TX        (1)
  #define LED_BUILTIN 13
#endif

#include <Relay.h>
#include <RelayService.h>
#include <Button.h>
#include <unity.h>

using namespace lkankowski;

#include <config.h>
#include <common.h>

Relay gRelay[gNumberOfRelays];
RelayService gRelayService(gNumberOfRelays, gRelay, gRelayConfig);


void setUp(void)
{};


void test_config_relays()
{
  // check if relay ID is unique
  for (int relayNum = 0; relayNum < gNumberOfRelays-1; relayNum++) {
    for(int secondRelayNum = relayNum+1; secondRelayNum < gNumberOfRelays; secondRelayNum++) {
//      Serial.println(String(gRelayConfig[relayNum].sensorId) + "==" + gRelayConfig[secondRelayNum].sensorId);
      TEST_ASSERT_NOT_EQUAL_MESSAGE(gRelayConfig[relayNum].sensorId,
                                    gRelayConfig[secondRelayNum].sensorId,
                                    (String("Relay id not unique: ") + relayNum + " & " + secondRelayNum).c_str());
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
                                        (String("Pin number is greater than number of digital pins: ") + pin).c_str());
        }
        TEST_ASSERT_NOT_EQUAL_MESSAGE(SERIAL_PORT_RX,
                                      pin,
                                      (String("Pin conflicts with default RX pin: ") + pin).c_str());
        TEST_ASSERT_NOT_EQUAL_MESSAGE(SERIAL_PORT_TX,
                                      pin,
                                      (String("Pin conflicts with default TX pin: ") + pin).c_str());
        #if defined(USE_EXPANDER)
          TEST_ASSERT_NOT_EQUAL_MESSAGE(PIN_WIRE_SDA,
                                        pin,
                                        (String("Pin conflicts with default expander SDA pin: ") + pin).c_str());
          TEST_ASSERT_NOT_EQUAL_MESSAGE(PIN_WIRE_SCL,
                                        pin,
                                        (String("Pin conflicts with default expander SCL pin: ") + pin).c_str());
          if (pin & 0xff00) {
            TEST_ASSERT_MESSAGE((((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)),
                                String("Configuration failed - expander no or number of pins out of range for button: ") + buttonNum);
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
                                         (String("Pin number must be greater than 0: ") + pin).c_str());
    if (pin & 0xff00) { //exclude expander pins
      TEST_ASSERT_LESS_THAN_MESSAGE(NUM_DIGITAL_PINS,
                                    pin,
                                    (String("Pin number is greater than number of digital pins: ") + pin).c_str());
    }
    TEST_ASSERT_NOT_EQUAL_MESSAGE(SERIAL_PORT_RX,
                                  pin,
                                  (String("Pin conflicts with default RX pin: ") + pin).c_str());
    TEST_ASSERT_NOT_EQUAL_MESSAGE(SERIAL_PORT_TX,
                                  pin,
                                  (String("Pin conflicts with default TX pin: ") + pin).c_str());
    #if defined(USE_EXPANDER)
      TEST_ASSERT_NOT_EQUAL_MESSAGE(PIN_WIRE_SDA,
                                    pin,
                                    (String("Pin conflicts with default expander SDA pin: ") + pin).c_str());
      TEST_ASSERT_NOT_EQUAL_MESSAGE(PIN_WIRE_SCL,
                                    pin,
                                    (String("Pin conflicts with default expander SCL pin: ") + pin).c_str());
      if (pin & 0xff00) {
        TEST_ASSERT_MESSAGE((((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)),
                            String("Configuration failed - expander no or number of pins out of range for button: ") + buttonNum);
      }
    #endif
  }
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


void test_relay_startup_eeprom()
{
  const RelayConfigDef relayConfig[] = {
    {1, 11, RELAY_TRIGGER_LOW | RELAY_STARTUP_ON,  -1, "Lamp 1"},
    {2, 12, RELAY_TRIGGER_LOW | RELAY_STARTUP_OFF, -1, "Lamp 2"},
    {3, 13, RELAY_TRIGGER_LOW, -1, "Lamp 3"},
  };
  int numberOfRelays = sizeof(relayConfig) / sizeof(RelayConfigDef);
  
  Relay * relays = new Relay[numberOfRelays];
  RelayService * relayService = new RelayService(numberOfRelays, relays, relayConfig);
  relayService->initialize(true); // reset eeprom
  TEST_ASSERT_TRUE_MESSAGE(relays[0].getState(), "[1] Lamp 1 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relays[1].getState(), "[1] Lamp 2 should be OFF");
  TEST_ASSERT_FALSE_MESSAGE(relays[2].getState(), "[1] Lamp 3 should be OFF");
  
  relayService->changeState(0, false);
  relayService->changeState(1, true);
  relayService->changeState(2, true);
  TEST_ASSERT_FALSE_MESSAGE(relays[0].getState(), "[2] Lamp 1 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relays[1].getState(), "[2] Lamp 2 should be ON");
  TEST_ASSERT_TRUE_MESSAGE(relays[2].getState(), "[2] Lamp 3 should be ON");

  delete relayService;
  delete relays;

  long dummy = -1L;

  Relay * relays2 = new Relay[numberOfRelays];
  RelayService * relayService2 = new RelayService(numberOfRelays, relays2, relayConfig);
  relayService2->initialize(false); // do not reset eeprom
  TEST_ASSERT_TRUE_MESSAGE(relays[0].getState(), "[3] Lamp 1 should be ON");
  TEST_ASSERT_FALSE_MESSAGE(relays[1].getState(), "[3] Lamp 2 should be OFF");
  TEST_ASSERT_TRUE_MESSAGE(relays[2].getState(), "[3] Lamp 3 should be ON");


  // startup and eeprom state at boot
  // const uint8_t RELAY_STARTUP_ON   = 2;
  // const uint8_t RELAY_STARTUP_OFF  = 4;
  // const uint8_t RELAY_STARTUP_MASK = RELAY_STARTUP_ON | RELAY_STARTUP_OFF;
  // RELAY_IMPULSE - also start OFF
};


void test_relay_impulse()
{
  //
  //const uint8_t RELAY_IMPULSE      = 8;
};

//TODO: changeState() - test if state has changed
//TODO: getRelayNum
//TODO: RELAY_TRIGGER_LOW or RELAY_TRIGGER_HIGH


int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_config_relays);
    RUN_TEST(test_config_buttons);
    RUN_TEST(test_relay_dependsOn);
    RUN_TEST(test_relay_startup_eeprom);
    RUN_TEST(test_relay_impulse);

    return UNITY_END();
};
