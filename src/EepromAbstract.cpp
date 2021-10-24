#include <EepromAbstract.h>

using namespace lkankowski;

#ifdef ARDUINO
// #include <EEPROM.h>
#include <avr/io.h>
#include <avr/eeprom.h>
extern uint8_t loadState(const uint8_t);
extern void saveState(const uint8_t, const uint8_t);


uint8_t Eeprom::read(int idx) const
{
  // return EEPROM.read(idx);
  return loadState(idx);
};

void Eeprom::write(int idx, uint8_t val)
{
  // EEPROM.write(idx, val);
  saveState(idx, val);
};


int Eeprom::length() const
{
  return E2END+1;
};


uint8_t Eeprom::dump(int idx) const
{
  return eeprom_read_byte((const uint8_t *) idx);
};


void Eeprom::clean() const
{
  for(int i = 0; i <= E2END; i++) {
    eeprom_write_byte((uint8_t *) i, 0);
  }
};

#else

//#include <algorithm>
// #include <iostream>

// Eeprom::Eeprom()
// {
//   std::fill(_mem, _mem+0x400, 0);
// };

uint8_t Eeprom::read(int idx) const
{
  //std::cout << "EEPROM.read(" << idx << ") = " << (int) _mem[idx] << std::endl;
  return _mem[idx];
};

void Eeprom::write(int idx, uint8_t val)
{
  _mem[idx] = val;
  //std::cout << "EEPROM.write(" << idx << ", " << (int) val << ")" << std::endl;
};


int Eeprom::length() const
{
  return 10;
};



uint8_t Eeprom::_mem[] = {0,0,0,0,0,0,0,0,0,0,0};


#endif
