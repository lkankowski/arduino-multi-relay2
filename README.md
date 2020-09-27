# About
Arduino program to handle relays and control them using switches.
Single pair of relay and switch are configured in single line.
Buttons has debouncing and support for mono- and bi-stable switches.
There is support for multiple switches for single relay and virtual switches for devices only accessible from Home App.

I left configuration variable _myRelayButtons[]_ which is used at my house, so before use you HAVE TO change it.

# Configuration
## First time only
Copy file "config.h.sample" into "config.h". On any future code update, you won't have to copy configuration to a newer version.

## Main config file "config.h"
```
RelayButton myRelayButtons[] = {
    {sensor ID, relay pin, button pin, relay options, button type, relay description}
};
```
Params description:
* sensor ID - sensor ID reported on MySensor Gateway (ie. Domoticz/Home Assistant), must be unique (can be -1 when it is not reported MySensor Gateway)
* relay pin - pin connected to the relay. Expander is supported (see details inseparate section)
* button pin - pin connected to the button. There can be multiple buttons to the same relay. "<0" for virtual buttons (only available in MySensor Gateway, ie. Domoticz/Home Assistant). There is no support for Expander because of de-bouncing.
* relay options - combined with '|' operator:
  * RELAY_TRIGGER_LOW or RELAY_TRIGGER_HIGH - trigger level, required
  * RELAY_STARTUP_ON or RELAY_STARTUP_OFF - startup state, optional
  * RELAY_IMPULSE - optional, relay is turned on only for short period of time (defined in constant RELAY_IMPULSE_INTERVAL, 250ms by default), ignored for DING_DONG and REED_SWITCH buttons
* button type:
  * MONO_STABLE - GND connected to the button pin
  * BI_STABLE - state change from LOW to HIGH and HIGH to LOW, used with mechanical and touch buttons
  * DING_DONG - doorbell button, relay is triggered only when button is pushed (experimental, not tested)
  * REED_SWITCH - door/window sensor, oposite to DING_DONG - state LOW when door/window is closed, HIGH when opened
* relay description - reported on MySensor Gateway, can help identify device on initial configuration in Home Automation App (ie. Domoticz/Home Assistant), can be empty ("")

In my case, I have a documentation of whole electricity of my house in Google Calc, and I have a formula to generate this configuration.

## Mono-stable buttons
For mono-stable buttons you can define in "config.h" if relay is triggered when you push button:
```
const uint8_t MONO_STABLE_TRIGGER = LOW;
```
or when button is released:
```
const uint8_t MONO_STABLE_TRIGGER = HIGH;
```


# Debugging
In a sketch file:
```
#define MY_DEBUG
```
Just comment this on production.

# Sample configurations
## Simple mono-stable switch and relay pair
Relay is connected to pin 12, button is connected to pin 11, relay is triggered to ON using LOW state, button is mono-stable
```
RelayButton myRelayButtons[] = {
    {1, 12, 11, RELAY_TRIGGER_LOW, MONO_STABLE, ""}
};
```

## Simple bi-stable switch and relay pair
Relay is connected to pin 12, button is connected to pin 11, relay is triggered to ON using HIGH state, button is bi-stable
```
RelayButton myRelayButtons[] = {
    {1, 12, 11, RELAY_TRIGGER_HIGH, BI_STABLE, ""}
};
```

## Simple two mono-stable switch for one relay
Relay is connected to pin 12, buttons are connected to pins 11 and 10, relay is triggered to ON using HIGH state, buttons are mono-stable
```
RelayButton myRelayButtons[] = {
    {1, 12, 11, RELAY_TRIGGER_HIGH, MONO_STABLE, ""}
    {2, 12, 10, RELAY_TRIGGER_HIGH, MONO_STABLE, ""}
};
```

## Mono-stable button with relay that starts turned on
```
RelayButton myRelayButtons[] = {
    {1, 12, 11, RELAY_TRIGGER_HIGH | RELAY_STARTUP_ON, MONO_STABLE, ""}
};
```

## Two similar doorbell configuration
```
RelayButton myRelayButtons[] = {
    {1, 12, 11, RELAY_TRIGGER_HIGH | RELAY_IMPULSE, MONO_STABLE, ""}
};
```
is equivalent to:
```
RelayButton myRelayButtons[] = {
    {1, 12, 11, RELAY_TRIGGER_HIGH, DING_DONG, ""}
};
```
Difference is that the first example will do only 250ms impulse (turn on - wait ~250ms - turn off), and the second will turn on the relay as long, as button is pushed.

## Door or window sensor
```
RelayButton myRelayButtons[] = {
    {1, 12, 11, RELAY_TRIGGER_HIGH, REED_SWITCH, ""}
};
```


# Expander
Only one expander library at a time is supported.

## PCF8574
To use expander PCF8574 you have to install library (https://github.com/skywodd/pcf8574_arduino_library)
Basic information about expander and library you can find here - https://youtu.be/JNmVREucfyc (PL, library in description)

And uncomment expander library:
```
#include "PCF8574.h"
```

## MCP23017
https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library

Uncomment expander library:
```
#include "Adafruit_MCP23017.h"
```

## Configuration

Configure all expanders id:
I.e. only one PCF8574 expander with id = 0x20:
```
uint8_t expanderAddresses[] = {0x20};
```
I.e. only one MCP23017 expander with id = 0:
```
uint8_t expanderAddresses[] = {0};
```
From now you can use expander pins in configuration _myRelayButtons[]_. To recognize expander pin, numbers start from 0x0100 and have special meaning:
* first byte - expander number (starts from 1)
* second byte - pin number
In example - "0x0100" means pin 0 on first expander

To simplify using expanders, there is "E(a,b)" macro:
* a - expander number (starts from 0)
* b - pin on expander [0-f]
```
E(0,0) - first pin on first expander
```

# Notes
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
