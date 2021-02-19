#include <Bounce2.h>

Bounce::Bounce() {};
// Bounce::Bounce(uint8_t pin, unsigned long interval_millis ) : Bounce() {
//   attach(pin);
//   interval(interval_millis);
// }
void Bounce::attach(int pin, int mode) {};
void Bounce::attach(int pin){};
void Bounce::interval(uint16_t interval_millis) {};
bool Bounce::update() { return false; };
bool Bounce::read() { return false; };
bool Bounce::fell() { return false; };
bool Bounce::rose() { return false; };
bool Bounce::changed() { return false; };
unsigned long Bounce::duration() { return 0; };
unsigned long Bounce::previousDuration() { return 0; };     
