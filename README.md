# Multi-Relay 2

Stable build Status ![](https://github.com/lkankowski/arduino-multi-relay2/actions/workflows/main.yml/badge.svg?branch=master), 
Develop build Status ![](https://github.com/lkankowski/arduino-multi-relay2/actions/workflows/main.yml/badge.svg?branch=develop)

* [About](#about)
* [Get Started](#get-started)
* [Configuration](#configuration)
* [Debugging](#debugging)
* [Expander](#expander)
* [Troubleshoting](#troubleshoting)

## About
Arduino program to handle relays and control them using switches with support for double-click and long-press. Integration with home automation systems is possible through MySensors integration over serial, LAN or MQTT.
Configuration is very simple - one line for a single relay and one line for single switch.
Every switch support debouncing, multiple types and additional actions like double-click and long-press - everything in a single line of configuration. Relays can be configured as LOW/HIGH level trigger, startup state ON or OFF and impulse type.
One relay can have one or more switches, or not at all if you want to control it only by home automation system.

### Supported platforms and hardware
* Arduino Mega2560 and other AVR ATmega family
* ESP8266
* ESP32 - Experimental! Tests with LAN worked fine, but had no luck with MQTT
* Ethernet (LAN or MQTT)
  * W5100
  * ENC28J60
* expanders (described later in this doc)
  * MCP23017
  * PCF8574


## Get Started

### Download
Most convinient is to clone the repo, but you have to have installed `git`:
```
git clone https://github.com/lkankowski/arduino-multi-relay2.git
```
You can also download zip file.

### Create config file
Copy file "config.h.sample" into "config.h". There is sample configuration you need to examine and customize for your own needs.

### Build
You need PlatformIO - it is free and you can get it here https://platformio.org/platformio-ide. Arduino IDE is not supported.
By default sketch is built for Arduino Mega 2560, but you choose environment that suits your needs.
Build options can be customized in `platformio.ini` - more information in separate sections.
Remote upload (ie. Arduino connected to Raspberry PI) is supported. For more information read https://docs.platformio.org/en/latest/core/userguide/remote/cmd_agent.html.

### Updating
If you have 'git', just type `git pull`. Otherwise download zip file and extract to new directory - remember, to backup 'config.h'.
It is also convinient to have 'config.h' someware else, then you can use symbolic link. In Windows you have 2 options:

CMD:
```
mklink "<your local config directory>\config.h" "<your repo directory>\include\config.h"
```
or PowerShell (assuming that you run in main repo directory):
```
Start-Process -Verb RunAs -FilePath "powershell" -ArgumentList "-NoExit","-command","New-Item -Path '$(Get-Location)\include\config.h' -ItemType SymbolicLink -Value '<your local config directory>\config.h'"
```

## Configuration

### Main config file "config.h"
```
const RelayConfigDef gRelayConfig[] PROGMEM = {
  {sensor id, relay pin, relay options, relay dependOn, relay description}
};
```
Params description:
* sensor id - sensor ID reported on MySensor Gateway (ie. Domoticz/Home Assistant), must be unique
* relay pin - pin connected to the relay. Expander is supported (see details in separate section), must be unique
* relay options - combined with '|' operator:
  * RELAY_TRIGGER_LOW or RELAY_TRIGGER_HIGH - required, trigger level
  * RELAY_STARTUP_ON or RELAY_STARTUP_OFF - optional, startup state
  * RELAY_IMPULSE - optional, relay is turned on only for short period of time (defined in constant RELAY_IMPULSE_INTERVAL, 250ms by default), ignored for DING_DONG and REED_SWITCH buttons
  * RELAY_INDEPENDENT - described in (#dependent-relays)
* relay dependOn - ID of relay that needs to be turned on before this one
* relay description - reported on MySensor Gateway, can help identify device on initial configuration in Home Automation System, max. 30 chars, can be empty ("")

```
const ButtonConfigDef gButtonConfig[] PROGMEM = {
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
* long-click relay id - sensor id used in relay configuration, -1 when not used, ignored for DING_DONG/REED_SWITCH
* double-click relay id - sensor id used in relay configuration, -1 when not used, ignored for DING_DONG/REED_SWITCH
* button description - debug only information, max. 30 chars, can be empty ("")

### Example config with REED_SWITCH
```
const RelayConfigDef gRelayConfig[] PROGMEM = {
  {26, 31, RELAY_TRIGGER_HIGH, -1, "Garage Door"},
  ...
};
const ButtonConfigDef gButtonConfig[] PROGMEM = {
  {43, REED_SWITCH, 26, -1, -1, "Garage Door reed switch"},
  ...
};
```
In this case relay 26 is always reported throught as S_DOOR sensor, i.e. in Home Assistant this relay can be found in entities as binary_sensor.

### Dependent relays
The "dependOn" option in relays configuration is intended for turning ON and OFF power supplies or simple scenes.
For example, you have 12V power supply and two 12V LED lights:
```
...
  {11, A1, RELAY_TRIGGER_LOW, 13, "LED Strip"},
  {12, A2, RELAY_TRIGGER_LOW, 13, "Stairs light"},
  {13, A3, RELAY_TRIGGER_LOW, -1, "Power supply 12V"},
...
```
When you turn on _LED Strip_ and/or _Stairs light_, then _Power supply 12V_ will be automatically turned on.
_Power supply 12V_ will be automatically turned off, when all parent devices will be turned off.

Additionaly there is one way option:
```
...
  {11, A1, RELAY_TRIGGER_LOW, 12, "Hall light"},
  {12, A2, RELAY_TRIGGER_LOW | RELAY_INDEPENDENT, -1, "Stairs light"},
...
```
In this variant, when you turn on _Hall light_, also _Stairs light_ will turn on,
but when you turn off _Hall light_, _Stairs light_ will remain on.


### Additional config
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
const char MULTI_RELAY_DESCRIPTION[] PROGMEM = "Multi Relay";
```

### Network Configuration
You can use network connection (Ethernet or WiFi) based on MySensors capabilities (ESP8266, ESP32, W5100, W5500, ENC28J60).
This needs some additional configuration in `config.h`:
* `#define MY_WIFI_SSID "<YOUR WIFI NAME>"` - WiFi SSID in case of ESP8266/ESP32
* `#define MY_WIFI_PASSWORD "<YOUR WIFI PASSWORD>"` - WiFi password in case of ESP8266/ESP32
* `#define MY_HOSTNAME "MULTI_RELAY_2"`
* `#define MY_MAC_ADDRESS 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED`
* In case of static IP address (no DHCP) you have to define following parameters:
  * `#define MY_IP_ADDRESS 192,168,178,87` - IP address of the device
  * `#define MY_IP_GATEWAY_ADDRESS 192,168,178,1` - IP address of your router
  * `#define MY_IP_SUBNET_ADDRESS 255,255,255,0` - local network mask
* `#define MY_PORT 5003` - if not using MQTT, MySensors gateway will listen on this port (if using default 5003 you dont need this in config.h)
* `#define MY_USE_UDP` - if not using MQTT, you can choose UDP (default is TCP)

This are still experimental options and waiting for contribution with working config.

### MQTT Configuration
* `#define MY_GATEWAY_MQTT_CLIENT` - enable MySensors MQTT support
* `#define MY_MQTT_PUBLISH_TOPIC_PREFIX "multi-relay/out"` - gateway will send events to this topic
* `#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "multi-relay/in"` - gateway will listen for commands on this topic
* `#define MY_MQTT_CLIENT_ID "multi-relay-2"` - client id when connecting to MQTT broker
* `#define MY_CONTROLLER_IP_ADDRESS 192,168,178,17` - MQTT broker IP address
* `#define MY_CONTROLLER_URL_ADDRESS "test.mosquitto.org"` - MQTT broker URL instead of ip address
* `#define MY_PORT 1883` - MQTT broker port
* `#define MY_MQTT_USER "<YOUR MQTT USERNAME>"` - user name, if your MQTT broker requires authentication
* `#define MY_MQTT_PASSWORD "<YOUR MQTT PASSWORD>"` - user password, if your MQTT broker requires authentication
* `#define MY_NODE_ID 1` - may be required (need tests)


## Debugging
In a `platformio.ini` file in section [env] you can uncomment (remove ";" at the beginning) some `build_flags`:
* `DEBUG_STATS=1000` - time statistics can be printed on serial, when they are triggered via appropriate MySensors command - read more in MySensors commands sections
* `DEBUG_COMMUNICATION` - show some debug information about received MySensors commands on serial
* `DEBUG_ACTION` - detailed information about button actions on serial
* `DEBUG_STARTUP` - detaled information about configuration parameters. Usefull when you want report some problems with sketch.

## Expander
Expanders alow you to have more GPIO pins over I2C protocol. On a single I2C data line you can connect one or more expanders.
It is recomended to use expanders only for relays, because they are much slower than internal GPIO, and switches are checked all the time.
Only one expander library at a time is supported.
To build expander version just select appropriate `env` with sufix `pcf` or `mcp`.

### PCF8574
Popular expander with additional 16 GPIO.
* Library `https://github.com/skywodd/pcf8574_arduino_library`.
  Basic information about expander and library you can find here - https://youtu.be/JNmVREucfyc (PL, library in description).

### MCP23017
Popular expander with additional 16 GPIO.
* Library: https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library


### Configuration
Configure expander id in `config.h` file:
```
const uint8_t gExpanderAddresses[] = {0x20}; //PCF8574
```
or MCP23017:
```
const uint8_t gExpanderAddresses[] = {0}; //MCP23017
```
For multiple expanders use different id's:
```
const uint8_t gExpanderAddresses[] = {0, 1, 2}; //MCP23017
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

### Relay config example
```
const RelayConfigDef gRelayConfig[] PROGMEM = {
  {1, E(0,3), RELAY_TRIGGER_LOW, -1, "RELAY 1"}
};
```

## Troubleshoting
1. If you have problems with unstable relay or button states after startup, uncomment `-D IGNORE_BUTTONS_START_MS=2000` in your `platformio.ini`.

2. Relays randomly turn on/off on startup - one reason could be eeprom memory corruption (it has limited number of writes). As a solution you can change eeprom area with `EEPROM_OFFSET` build flag (i.e. `-D EEPROM_OFFSET=100`) .

## MySensors special commands
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

Validate relays & eeprom state
```
0;255;1;0;24;7
```

Free Memory
```
0;255;1;0;24;8
```
