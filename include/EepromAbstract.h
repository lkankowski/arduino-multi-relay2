#pragma once

#include <stdint.h>

namespace lkankowski {

  class EepromInterface
  {
  public:
    virtual uint8_t read(int idx) const = 0;
    virtual void write(int idx, uint8_t val) = 0;
    virtual int length() const = 0;
  };


  class Eeprom : public EepromInterface
  {
  public:
    uint8_t read(int idx) const override;
    void write(int idx, uint8_t val) override;
    int length() const override;

    #ifdef ARDUINO
      uint8_t dump(int idx) const;
      void clean() const;
    #else
      static uint8_t _mem[11];
    #endif
  };

} // namespace lkankowski

