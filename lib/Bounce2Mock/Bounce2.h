#pragma once

#include <Arduino.h>
#include <inttypes.h>

class Bounce {
public:
	Bounce();
  //Bounce(uint8_t pin, unsigned long interval_millis) : Bounce();
	void attach(int pin, int mode);
	void attach(int pin);
	void interval(uint16_t interval_millis);
	bool update();
	bool read();
	bool fell();
	bool rose();
  bool changed();
  unsigned long duration();
  unsigned long previousDuration();     
};

