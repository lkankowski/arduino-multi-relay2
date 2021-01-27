#include <Arduino.h>
//#ifdef ARDUINO_VERSION
//#include "Arduino.h"
//#endif

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
#include <Button.h>
#include <unity.h>

using namespace lkankowski;

#include <config.h>
#include <common.h>


void setUp(void) {
};


void test_config_relays() {

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


void test_config_buttons() {

  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
    int pin = gButtonConfig[buttonNum].buttonPin;
    TEST_ASSERT_GREATER_OR_EQUAL(-1, getRelayNum(gButtonConfig[buttonNum].clickRelayId));
    TEST_ASSERT_GREATER_OR_EQUAL(-1, getRelayNum(gButtonConfig[buttonNum].longClickRelayId));
    TEST_ASSERT_GREATER_OR_EQUAL(-1, getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
    if (gButtonConfig[buttonNum].clickRelayId > -1) {
      TEST_ASSERT_NOT_EQUAL(-1, getRelayNum(gButtonConfig[buttonNum].clickRelayId));
    }
    if (gButtonConfig[buttonNum].longClickRelayId > -1) {
      TEST_ASSERT_NOT_EQUAL(-1, getRelayNum(gButtonConfig[buttonNum].longClickRelayId));
    }
    if (gButtonConfig[buttonNum].doubleClickRelayId > -1) {
      TEST_ASSERT_NOT_EQUAL(-1, getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
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


int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_config_relays);
    RUN_TEST(test_config_buttons);

    return UNITY_END();
};
