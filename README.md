# About
Arduino program to handle relays and control them using switches with support for double-click and long-press. Integration with home automation systems is possible through MySensors integration.
Configuration is very simple - one line for a single relay and one line for single switch
Every switch support debouncing, multiple types of switches and additional actions like double-click and long-press - everything in a single line of configuration. Relays can be configured as LOW/HIGH level trigger, startup state ON or OFF and impulse type.
One relay can have one or more switches, or not at all if you want to control it only by home automation system.

# Get Started

## Create config file
Copy file "config.h.sample" into "config.h". There is sample configuration you need to examine and customize for your own needs.

## Build
You need PlatformIO - it is free and you can get it here https://platformio.org/platformio-ide. Arduino IDE is not supported.
By default sketch is build for Arduino Mega 2560. Build options can be customized in `platformio.ini` - more information in separate sections.

# Configuration

## Main config file "config.h"
```
const RelayConfigDef gRelayConfig[] = {
  {sensor id, relay pin, relay options, relay description}
};
```
Params description:
* sensor id - sensor ID reported on MySensor Gateway (ie. Domoticz/Home Assistant), must be unique
* relay pin - pin connected to the relay. Expander is supported (see details in separate section), must be unique
* relay options - combined with '|' operator:
  * RELAY_TRIGGER_LOW or RELAY_TRIGGER_HIGH - required, trigger level
  * RELAY_STARTUP_ON or RELAY_STARTUP_OFF - optional, startup state
  * RELAY_IMPULSE - optional, relay is turned on only for short period of time (defined in constant RELAY_IMPULSE_INTERVAL, 250ms by default), ignored for DING_DONG and REED_SWITCH buttons
* relay description - reported on MySensor Gateway, can help identify device on initial configuration in Home Automation System, can be empty ("")

```
const ButtonConfigDef gButtonConfig[] = {
  {button pin, button type, click relay id, long-click relay id, double-click relay id, button description}
};
```
Params description:
* button pin - pin connected to the button. Expander is supported (see details in separate section) but not recomended for switches
* button type:
  * MONO_STABLE - for impulse switches, mainly mechanical, which are shorted to the GND when pushed
  * BI_STABLE - state change from LOW to HIGH and HIGH to LOW, used with mechanical and touch buttons
  * DING_DONG - doorbell button, relay is triggered only when button is pushed
  * REED_SWITCH - door/window sensor, oposite to DING_DONG - state LOW when door/window is closed, HIGH when opened
* click relay id - sensor id used in relay configuration
* long-click relay id - sensor id used in relay configuration, -1 when not used
* double-click relay id - sensor id used in relay configuration, -1 when not used
* button description - debug only information

## Example config with REED_SWITCH and use expander MCP23017
```
const char MULTI_RELAY_DESCRIPTION[] = "Multi Relay";

// Relays config - ID and PIN MUST BE UNIQUE (can't be repeated)!
// Row params: relay ID - [0-254] sensor ID reported on MySensor Gateway
//             relay pin - pin used for relay, can be expander pin via "E(x, y)" macro
//             relay options - [RELAY_TRIGGER_LOW|RELAY_TRIGGER_HIGH] {RELAY_STARTUP_ON|RELAY_STARTUP_OFF} {RELAY_IMPULSE}
//             relay description - reported on MySensor Gateway, can help identify device on initial configuration in Home Automation App, can be empty ("")
const RelayConfigDef gRelayConfig[] = {
  {0, 12, RELAY_TRIGGER_HIGH, "Ośw. hol-1 sekcja"},  // WŁ: przy wej. z wiatrołapu (podwójny), przy wejściu do salonu (podówjny), doł przy schodach (podwójny)
  {1, 11, RELAY_TRIGGER_HIGH, "Ośw. zew. od ogrodu"},  // WŁ: wiatrołap przy wejściu do domu (podwójny)
  {2, 10, RELAY_TRIGGER_HIGH, "Ośw. nad tarasem"},  // WŁ: przy wyjściu na taras (podwójny)
  {3, 9, RELAY_TRIGGER_HIGH, "Ośw. na tarasie"},  // WŁ: przy wyjściu na taras (podwójny)
  {4, 8, RELAY_TRIGGER_HIGH, "Ośw. klatka-schody"},  // WŁ: klatka przy schodach dół i góra
  {5, 7, RELAY_TRIGGER_HIGH, "Ośw. klatka-kinkiety"},  // WŁ: klatka przy schodach dół i góra
  {6, 6, RELAY_TRIGGER_HIGH, "Ośw. kotłownia"},  // WŁ: przy wejściu z garażu + wyjście z kotłowni na zewnątrz=mostek z garażem
  {7, 5, RELAY_TRIGGER_HIGH, "Ośw. garaż-1 sekcja"},  // WŁ: przy wejściu do garażu (podwójny), wejście do kotłowni (podwójny)
  {8, 4, RELAY_TRIGGER_HIGH, "Ośw. garaż-2 sekcja"},  // WŁ: przy wejściu do garażu (podwójny), wejście do kotłowni (podwójny)
  {9, 3, RELAY_TRIGGER_HIGH, "Ośw. przed wejściem"},   // WŁ: wiatrołap przy wejściu do domu (podwójny)
  {10, 2, RELAY_TRIGGER_HIGH, "Ośw. zew. od kotłowni"},  // WŁ: wiatrołap przy wejściu do domu (podwójny)
  {11, 14, RELAY_TRIGGER_HIGH, "Ośw. zew. przed garażem"},  // WŁ: wiatrołap przy wejściu do domu (podwójny)
  {12, 15, RELAY_TRIGGER_HIGH, "Ośw. wiatrołap"},  // WŁ: wiatrołap przy wejściu do domu i wejściu z wiatrołapu do holu
  {13, 16, RELAY_TRIGGER_HIGH, "Ośw. salon kinkiety"},  // WŁ: wejście do salonu (podwójny), przy wykuszu
  {14, 17, RELAY_TRIGGER_HIGH, "Ośw. salon sufit"},  // WŁ: wejście do salonu (podwójny), przy wykuszu
  {15, 18, RELAY_TRIGGER_HIGH, "Ośw. łazienka_0-kinkiet"},  // WŁ: wejście do łazienki+lustro=zmostkowan
  {16, 19, RELAY_TRIGGER_HIGH, "Ośw. łazienka_0-sufit"},  // WŁ: wejście do łazienki+lustro=zmostkowan
  {17, 22, RELAY_TRIGGER_HIGH, "Ośw. salon nad stołem"},  // WŁ: wejście do salonu, przy wykuszu
  {18, 23, RELAY_TRIGGER_HIGH, "Ośw. gabinet-sufit1"},  // WŁ: wejście do gabinetu (podwójny)
  {19, 24, RELAY_TRIGGER_HIGH, "Ośw. gabinet-kinkiet"},  // WŁ: wejście do gabinetu
  {20, 25, RELAY_TRIGGER_HIGH, "Ośw. kuchnia-1 sekcja"},  // WŁ: kuchnia z prawej strony lodówki (podwójny),wejście do salonu (podwójny)
  {21, 26, RELAY_TRIGGER_HIGH, "Ośw. kuchnia-2 sekcja"},  // WŁ: kuchnia z prawej strony lodówki (podwójny),wejście do salonu (podwójny)
  {22, 27, RELAY_TRIGGER_HIGH, "Ośw. kuchnia-plafon"},  // WŁ: wejście do salonu
  {23, 28, RELAY_TRIGGER_HIGH, "Ośw. schowek"},  // WŁ: schowek
  {24, 29, RELAY_TRIGGER_HIGH | RELAY_IMPULSE, "Brama garażowa"},  // WŁ: wej do garażu
  {25, 30, RELAY_TRIGGER_HIGH | RELAY_IMPULSE, "Brama wjazdowa"},  // WŁ: wej do garażu
  {26, 31, RELAY_TRIGGER_HIGH, "Kontakt. brama wjazdowa"},  // kontaktron na bramie wjazdowej
  {27, 32, RELAY_TRIGGER_HIGH, "WLED salon-wirtualny"},  // wirtualny przekaźnik dla sterowania modułem WLED-salon
  {28, E(2,0), RELAY_TRIGGER_HIGH, "Ośw. zew. balkon"},  // WŁ: przy balkonie Zuzia i Pokój2 (podwójny)
  {29, E(2,1), RELAY_TRIGGER_HIGH, "Ośw. hol piętro"},  // WŁ: przy schodach, przy domofonie piętro, przy garderobie hol
  {30, E(2,2), RELAY_TRIGGER_HIGH, "Ośw. zew. l.choinkowe"},  // WŁ: BRAK
  {31, E(2,3), RELAY_TRIGGER_HIGH, "Ośw. pokój2-sufit"},  // WŁ: przy wejściu
  {32, E(2,4), RELAY_TRIGGER_HIGH, "Ośw. pokój2-kinkiety"},  // WŁ: przy wejściu, przy balkonie (podwójny)
  {33, E(2,5), RELAY_TRIGGER_HIGH, "Ośw. Zuzia-sufit"},  // WŁ: przy wejściu
  {34, E(2,6), RELAY_TRIGGER_HIGH, "Ośw. Zuzia-kinkiety"},  // WŁ: przy wejściu, przy balkonie (podwójny)
  {35, E(2,7), RELAY_TRIGGER_HIGH, "Ośw. łazienka_1-sufit1"},  // WŁ: wej. do łazienki (podwójny)
  {36, E(2,8), RELAY_TRIGGER_HIGH, "Ośw. łazienka_1-LEDY"},  // WŁ: wej. do łazienki (podwójny)
  {37, E(2,9), RELAY_TRIGGER_HIGH, "Ośw. łazienka_1-kink._L"},  // WŁ: przy lustrze po prawo i lewo
  {38, E(2,10), RELAY_TRIGGER_HIGH, "Ośw. pralnia"},  // WŁ: przy wej. do pralni
  {39, E(2,11), RELAY_TRIGGER_HIGH, "Ośw. strych"},  // WŁ: przy wej. do pralni
  {40, E(2,12), RELAY_TRIGGER_HIGH, "Ośw. garderoba hol"},  // WŁ: na holu przy wejściu do garderoby
  {41, E(2,13), RELAY_TRIGGER_HIGH, "Ośw. sypialnia-1 tuba"},  // WŁ: przy wej do sypialni (podwójny)
  {42, E(2,14), RELAY_TRIGGER_HIGH, "Ośw. sypialnia-2 tuby"},  // WŁ: przy wej do sypialni (podwójny)
  {43, E(2,15), RELAY_TRIGGER_HIGH, "Ośw. garderoba sypialnia"},  // WŁ: przy wej. do garderoby w sypialni
  {44, E(3,0), RELAY_TRIGGER_HIGH, "Ośw. sypial.-kink.Piotr"},  // WŁ: przy łóżku PIOTREK
  {45, E(3,1), RELAY_TRIGGER_HIGH, "Ośw. sypial.-kink.Iza"},  // WŁ: przy łóżku IZA
  {46, E(3,2), RELAY_TRIGGER_HIGH, "Ośw. hol-2 sekcja"},  // WŁ: przy wej. z wiatrołapu (podwójny), przy wejściu do salonu (podówjny), doł przy schodach (podwójny)
  {47, E(3,3), RELAY_TRIGGER_HIGH, "Ośw. gabinet-sufit2"},  // WŁ: wejście do gabinetu (podwójny)
  {48, E(3,4), RELAY_TRIGGER_HIGH, "Ośw. łazienka_1-kink._P"},  // WŁ: przy lustrze po prawo i lewo
  {49, E(3,5), RELAY_TRIGGER_HIGH, "Ośw. łazienka_1-sufit2"},  // WŁ: wej. do łazienki (podwójny)
  {50, E(3,6), RELAY_TRIGGER_HIGH, "PUSTY-PUSTY-PUSTY"},  // 
  {51, E(3,7), RELAY_TRIGGER_HIGH, "PUSTY-PUSTY-PUSTY"},  // 
  {52, E(3,8), RELAY_TRIGGER_HIGH, "WLED Sypialnia-wirtualny"},  // wirtualny przekaźnik dla sterowania modułem WLED-sypialnia
  {53, E(3,9), RELAY_TRIGGER_HIGH, "WLED Zuzia-wirtualny"},  // wirtualny przekaźnik dla sterowania modułem WLED-Zuzia
  {54, E(3,10), RELAY_TRIGGER_HIGH, "Ośw. kuchnia-LEDY-wirtualny"},  // wirtualny przekaźnik dla sterowania modułem LED-kuchnia
  {55, 33, RELAY_TRIGGER_HIGH | RELAY_IMPULSE, "B. garażowa-uchylenie"},  // WŁ:
  {56, 34, RELAY_TRIGGER_HIGH | RELAY_IMPULSE, "B. wjazdowa-uchylenie"}  // WŁ:
};

// Buttons config
// Row params: button pin - pin used for button, can be expander pin (but NOT RECOMMENDED) via "E(x, y)" macro
//             button type - [MONO_STABLE|BI_STABLE|DING_DONG|REED_SWITCH]
//             click action relay ID - MUST be defined in gRelayConfig
//             long-press action relay ID - ignored for BI_STABLE, DING_DONG, REED_SWITCH
//             double-click action relay ID - ignored for DING_DONG, REED_SWITCH
//             button description - debug only, can be empty ("")
const ButtonConfigDef gButtonConfig[] = {
  {A0, MONO_STABLE, 0, -1, 29, "Ośw. hol-1 sekcja"},  // WŁ: przy wej. z wiatrołapu (podwójny), przy wejściu do salonu (podówjny), doł przy schodach (podwójny)
  {A1, MONO_STABLE, 1, -1, -1, "Ośw. zew. od ogrodu"},  // WŁ: wiatrołap przy wejściu do domu (podwójny)
  {A2, MONO_STABLE, 2, -1, -1, "Ośw. nad tarasem"},  // WŁ: przy wyjściu na taras (podwójny)
  {A3, MONO_STABLE, 3, -1, -1, "Ośw. na tarasie"},  // WŁ: przy wyjściu na taras (podwójny)
  {A4, MONO_STABLE, 4, -1, -1, "Ośw. klatka-schody"},  // WŁ: klatka przy schodach dół i góra
  {A5, MONO_STABLE, 5, -1, -1, "Ośw. klatka-kinkiety"},  // WŁ: klatka przy schodach dół i góra
  {A6, MONO_STABLE, 6, -1, -1, "Ośw. kotłownia"},  // WŁ: przy wejściu z garażu + wyjście z kotłowni na zewnątrz=mostek z garażem
  {A7, MONO_STABLE, 7, -1, -1, "Ośw. garaż-1 sekcja"},  // WŁ: przy wejściu do garażu (podwójny), wejście do kotłowni (podwójny)
  {A8, MONO_STABLE, 8, -1, -1, "Ośw. garaż-2 sekcja"},  // WŁ: przy wejściu do garażu (podwójny), wejście do kotłowni (podwójny)
  {A9, MONO_STABLE, 9, -1, -1, "Ośw. przed wejściem"},   // WŁ: wiatrołap przy wejściu do domu (podwójny)
  {A10, MONO_STABLE, 10, -1, -1, "Ośw. zew. od kotłowni"},  // WŁ: wiatrołap przy wejściu do domu (podwójny)
  {A11, MONO_STABLE, 11, -1, -1, "Ośw. zew. przed garażem"},  // WŁ: wiatrołap przy wejściu do domu (podwójny)
  {A12, MONO_STABLE, 12, -1, -1, "Ośw. wiatrołap"},  // WŁ: wiatrołap przy wejściu do domu i wejściu z wiatrołapu do holu
  {A13, MONO_STABLE, 13, -1, -1, "Ośw. salon kinkiety"},  // WŁ: wejście do salonu (podwójny), przy wykuszu
  {A14, MONO_STABLE, 14, -1, -1, "Ośw. salon sufit"},  // WŁ: wejście do salonu (podwójny), przy wykuszu
  {A15, MONO_STABLE, 15, -1, 16, "Ośw. łazienka_0-kink."},  // WŁ: wejście do łazienki+lustro=zmostkowan
  {53, MONO_STABLE, 16, -1, 15, "Ośw. łazienka_0-sufit"},  // WŁ: wejście do łazienki+lustro=zmostkowan
  {52, MONO_STABLE, 17, 1, -1, "Ośw. salon nad stołem"},  // WŁ: wejście do salonu, przy wykuszu
  {51, MONO_STABLE, 18, -1, -1, "Ośw. gabinet-sufit1"},  // WŁ: wejście do gabinetu (podwójny)
  {50, MONO_STABLE, 19, -1, -1, "Ośw. gabinet-kinkiet"},  // WŁ: wejście do gabinetu
  {49, MONO_STABLE, 20, 54, 23, "Ośw. kuchnia-1 sekcja"},  // WŁ: kuchnia z prawej strony lodówki (podwójny),wejście do salonu (podwójny)
  {48, MONO_STABLE, 21, 54, 23, "Ośw. kuchnia-2 sekcja"},  // WŁ: kuchnia z prawej strony lodówki (podwójny),wejście do salonu (podwójny)
  {47, MONO_STABLE, 22, -1, -1, "Ośw. kuchnia-plafon"},  // WŁ: wejście do salonu
  {46, MONO_STABLE, 23, -1, -1, "Ośw. schowek"},  // WŁ: schowek
  {45, MONO_STABLE, 24, -1, -1, "Brama garażowa"},  // WŁ: wej do garażu
  {44, MONO_STABLE, 25, -1, -1, "Brama wjazdowa"},  // WŁ: wej do garażu
  {43, REED_SWITCH, 26, -1, -1, "Kontakt. brama wjazdowa"},  // kontaktron na bramie wjazdowej
  {42, MONO_STABLE, 27, -1, -1, "WLED salon-wirtualny"},  // wirtualny przycisk dla sterowania modułem WLED-salon
  {E(0,0), MONO_STABLE, 28, -1, -1, "Ośw. zew. balkon"},  // WŁ: przy balkonie Zuzia i Pokój2 (podwójny)
  {E(0,1), MONO_STABLE, 29, -1, -1, "Ośw. hol piętro"},  // WŁ: przy schodach, przy domofonie piętro, przy garderobie hol
  {E(0,2), MONO_STABLE, 30, -1, -1, "Ośw. zew. l.choinkowe"},  // WŁ: BRAK
  {E(0,3), MONO_STABLE, 31, -1, -1, "Ośw. pokój2-sufit"},  // WŁ: przy wejściu
  {E(0,4), MONO_STABLE, 32, -1, 31, "Ośw. pokój2-kinkiety"},  // WŁ: przy wejściu, przy balkonie (podwójny)
  {E(0,5), MONO_STABLE, 33, 53, -1, "Ośw. Zuzia-sufit"},  // WŁ: przy wejściu
  {E(0,6), MONO_STABLE, 34, 53, 33, "Ośw. Zuzia-kinkiety"},  // WŁ: przy wejściu, przy balkonie (podwójny)
  {E(0,7), MONO_STABLE, 35, 36, -1, "Ośw. łazienka_1-sufit1"},  // WŁ: wej. do łazienki (podwójny)
  {E(0,8), MONO_STABLE, 36, -1, -1, "Ośw. łazienka_1-LEDY"},  // WŁ: wej. do łazienki (podwójny)
  {E(0,9), MONO_STABLE, 37, 36, 49, "Ośw. łazienka_1-kink._L"},  // WŁ: przy lustrze po prawo i lewo
  {E(0,10), MONO_STABLE, 38, -1, 35, "Ośw. pralnia"},  // WŁ: przy wej. do pralni
  {E(0,11), MONO_STABLE, 39, -1, -1, "Ośw. strych"},  // WŁ: przy wej. do pralni
  {E(0,12), MONO_STABLE, 40, -1, -1, "Ośw. garderoba hol"},  // WŁ: na holu przy wejściu do garderoby
  {E(0,13), MONO_STABLE, 41, 52, 44, "Ośw. sypialnia-1 tuba"},  // WŁ: przy wej do sypialni (podwójny)
  {E(0,14), MONO_STABLE, 42, 52, 45, "Ośw. sypialnia-2 tuby"},  // WŁ: przy wej do sypialni (podwójny)
  {E(0,15), MONO_STABLE, 43, -1, -1, "Ośw. garderoba sypialnia"},  // WŁ: przy wej. do garderoby w sypialni
  {E(1,0), MONO_STABLE, 44, 52, 45, "Ośw. sypial.-kink.Piotr"},  // WŁ: przy łóżku PIOTREK
  {E(1,1), MONO_STABLE, 45, 52, 44, "Ośw. sypial.-kink.Iza"},  // WŁ: przy łóżku IZA
  {E(1,2), MONO_STABLE, 46, -1, 29, "Ośw. hol-2 sekcja"},  // WŁ: przy wej. z wiatrołapu (podwójny), przy wejściu do salonu (podówjny), doł przy schodach (podwójny)
  {E(1,3), MONO_STABLE, 47, -1, -1, "Ośw. gabinet-sufit2"},  // WŁ: wejście do gabinetu (podwójny)
  {E(1,4), MONO_STABLE, 48, 36, 35, "Ośw. łazienka_1-kink._P"},  // WŁ: przy lustrze po prawo i lewo
  {E(1,5), MONO_STABLE, 49, 36, -1, "Ośw. łazienka_1-sufit2"},  // WŁ: wej. do łazienki (podwójny)
  {E(1,6), MONO_STABLE, 50, -1, -1, "PUSTY-PUSTY-PUSTY"},  // 
  {E(1,7), MONO_STABLE, 51, -1, -1, "PUSTY-PUSTY-PUSTY"},  // 
  {E(1,8), MONO_STABLE, 52, -1, -1, "WLED sypialnia-wirtualny"},  // wirtualny przycisk dla sterowania modułem WLED-sypialnia
  {E(1,9), MONO_STABLE, 53, -1, -1, "WLED Zuzia-wirtualny"},  // wirtualny przycisk dla sterowania modułem WLED-Zuzia
  {E(1,10), MONO_STABLE, 54, -1, -1, "Ośw. kuchnia-LEDY-wirtualny"},  // wirtualny przycisk dla sterowania modułem LED-kuchnia
  {41, MONO_STABLE, 55, -1, -1, "B. garażowa-uchylenie"},  // WŁ:
  {40, MONO_STABLE, 56, -1, -1, "B. wjazdowa-uchylenie"}  // WŁ:
};
```
In this config in Home Assistant relays are found in entities as switch.multi_relay_0_x and switch contact(REED_SWITCH) is found in entieties as binary_sensor.multi_relay_0_26

## Additional config
Optional configuration only if you want to customize the script.

MONO_STABLE switches can trigger ralay when switch is pressed (LOW) or when switch is released (HIGH):
```
const uint8_t MONO_STABLE_TRIGGER = LOW;
```

Time interval for RELAY_IMPULSE type relay, ignored when button type is DING_DONG or REED_SWITCH:
```
const unsigned long RELAY_IMPULSE_INTERVAL = 250;
```

Debounce interval in milliseconds:
```
const unsigned long BUTTON_DEBOUNCE_INTERVAL = 50;
```

Double-click interval in milliseconds:
```
const unsigned long BUTTON_DOUBLE_CLICK_INTERVAL = 350;
```

Long-press interval in milliseconds:
```
const unsigned long BUTTON_LONG_PRESS_INTERVAL = 800;
```

Sketch description reported via MySensors to Home Automation System:
```
const char MULTI_RELAY_DESCRIPTION[] = "Multi Relay";
```


# Debugging
In a `platformio.ini` file in section [common_env_data] you can uncomment (remove ";" at the beginning) some `build_flags`:
* `DEBUG_STATS=1000` - time statistics can be printed on serial, when they are triggered via appropriate MySensors command - read more in MySensors commands sections
* `DEBUG_COMMUNICATION` - show some debug information about received MySensors commands on serial
* `DEBUG_ACTION` - detailed information about button actions on serial
* `DEBUG_STARTUP` - detaled information about configuration parameters. Usefull when you want report some problems with sketch.


# Expander
Only one expander library at a time is supported.

## PCF8574
To use expander PCF8574 you have to install library:
* download https://github.com/skywodd/pcf8574_arduino_library as zip archive
* extract directory PCF8574 into `lib/PCF8574`
Basic information about expander and library you can find here - https://youtu.be/JNmVREucfyc (PL, library in description)

And in a `platformio.ini` file in section [common_env_data] uncomment `EXPANDER_PCF8574` in `build_flags`.

## MCP23017
https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library

In a `platformio.ini` file in section [common_env_data] uncomment expander library in `lib_deps_builtin`:
```
adafruit/Adafruit MCP23017 Arduino Library @ ^1.2.0
```
and `EXPANDER_MCP23017` in `build_flags`.

## Configuration

Configure expander id in `config.h` file:
```
uint8_t expanderAddresses[] = {0x20}; //PCF8574
```
or MCP23017:
```
uint8_t expanderAddresses[] = {0}; //MCP23017
```
For multiple expanders use different id's:
```
uint8_t expanderAddresses[] = {0, 1, 2}; //MCP23017
```

From now you can use expander pins in configuration of realays and buttons. To recognize expander pin, numbers start from 0x0100 and have special meaning:
* first byte - expander number (starts from 1)
* second byte - pin number
In example - "0x0100" means pin 0 on first expander

To simplify using expanders, there is "E(a,b)" macro:
* a - expander number (starts from 0)
* b - pin on expander [0-f]
```
E(0,0) - first pin on first expander
```

## Relay config example
```
const RelayConfigDef gRelayConfig[] = {
  {1, E(0,3), RELAY_TRIGGER_LOW, "RELAY 1"}
};
```

# Troubleshoting
If you have problems with unstable relay or button states after startup, uncomment `IGNORE_BUTTONS_START_MS=2000` in your `platformio.ini`.

# MySensors special commands
Show debug stats
```
0;255;1;0;24;1
```

Show all relays state
```
0;255;1;0;24;2
```

Show all buttons state
```
0;255;1;0;24;3
```

Dump EEPROM
```
0;255;1;0;24;4
```

Clean EEPROM & reset
```
0;255;1;0;24;5
```

Reset
```
0;255;1;0;24;6
```
