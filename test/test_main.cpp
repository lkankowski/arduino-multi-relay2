#include <Arduino.h>
#include <Relay.h>
#include <Button.h>
#include <unity.h>

using namespace lkankowski;

#include <config.h>

const int gNumberOfRelays = sizeof(gRelayConfig) / sizeof(RelayConfigDef);
const int gNumberOfButtons = sizeof(gButtonConfig) / sizeof(ButtonConfigDef);
Relay gRelay[gNumberOfRelays];
lkankowski::Button gButton[gNumberOfButtons];

#include <common.h>


void setUp(void) {
};


void test_config() {

  #ifdef USE_EXPANDER
    for (int relayNum = 0; relayNum < gNumberOfRelays; relayNum++) {
      int pin = gRelayConfig[relayNum].relayPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)) {
          Serial.println(String("Configuration failed - expander no or number of pins out of range for relay: ") + relayNum);
        }
      }
    }
  #endif
  for (int buttonNum = 0; buttonNum < gNumberOfButtons; buttonNum++) {
    #ifdef USE_EXPANDER
      int pin = gButtonConfig[buttonNum].buttonPin;
      if (pin & 0xff00) {
        if (((pin >> 8) > gNumberOfExpanders) || ((pin & 0xff) >= EXPANDER_PINS)) {
          Serial.println(String("Configuration failed - expander no or number of pins out of range for button: ") + buttonNum);
        }
      }
    #endif
    TEST_ASSERT_NOT_EQUAL(-1, getRelayNum(gButtonConfig[buttonNum].clickRelayId));
    TEST_ASSERT_NOT_EQUAL(-1, getRelayNum(gButtonConfig[buttonNum].longClickRelayId));
    TEST_ASSERT_NOT_EQUAL(-1, getRelayNum(gButtonConfig[buttonNum].doubleClickRelayId));
  }
};


int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_config);

    return UNITY_END();
};
