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
  * REED_SWITCH - door/window sensor, oposite to DING_DONG - state OFF when door/window is closed, ON when opened. All relays with at least one REED_SWITCH are reported throught MySensors as S_DOOR sensor.
* click relay id - sensor id used in relay configuration
* long-click relay id - sensor id used in relay configuration, -1 when not used
* double-click relay id - sensor id used in relay configuration, -1 when not used
* button description - debug only information


## Example config with REED_SWITCH
```
const RelayConfigDef gRelayConfig[] = {
  {26, 31, RELAY_TRIGGER_HIGH, "Garage Door"},
  ...
};
const ButtonConfigDef gButtonConfig[] = {
  {43, REED_SWITCH, 26, -1, -1, "Garage Door reed switch"},
  ...
};
```
In this case relay 26 is always reported throught as S_DOOR sensor, i.e. in Home Assistant this relay can be found in entities as binary_sensor.multi_relay_0_26.


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
