#include <EepromAbstract.h>

using namespace lkankowski;

#ifdef ARDUINO
#include <EEPROM.h>

uint8_t Eeprom::read(int idx)
{
  return EEPROM.read(idx);
};

void Eeprom::write(int idx, uint8_t val)
{
  EEPROM.write(idx, val);
};

#else

//#include <algorithm>
// #include <iostream>

// Eeprom::Eeprom()
// {
//   std::fill(_mem, _mem+0x400, 0);
// };

uint8_t Eeprom::read(int idx)
{
  //std::cout << "EEPROM.read(" << idx << ") = " << (int) _mem[idx] << std::endl;
  return _mem[idx];
};

void Eeprom::write(int idx, uint8_t val)
{
  _mem[idx] = val;
  //std::cout << "EEPROM.write(" << idx << ", " << (int) val << ")" << std::endl;
};

uint8_t Eeprom::_mem[] = {0,0,0,0,0,0,0,0,0,0,0};


#endif
