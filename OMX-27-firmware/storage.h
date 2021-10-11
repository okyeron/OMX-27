#ifndef _INCLUDE_STORAGE_H
#define _INCLUDE_STORAGE_H

#include <Adafruit_FRAM_I2C.h>

enum StorageType {
  EEPROM_MEMORY = 0,
  FRAM_MEMORY = 1
};

// abstract storage class
class Storage {
public:
  Storage() {}

  virtual int capacity() = 0;

  // read/write bytes
  virtual void write(size_t address, uint8_t val) = 0;
  virtual uint8_t read(size_t address) = 0;

// template reader/writer implementation copied from Adafruit_FRAM_I2C which implements them both
// in terms of reading/writing bytes

template <class T> uint16_t writeObject(uint16_t addr, const T &value) {
  const byte *p = (const byte *)(const void *)&value;
  uint16_t n;
  for (n = 0; n < sizeof(value); n++)
  {
    write(addr++, *p++);
  }
  return n;
}

template <class T> uint16_t readObject(uint16_t addr, T &value) {
  byte *p = (byte *)(void *)&value;
  uint16_t n;
  for (n = 0; n < sizeof(value); n++) {
    *p++ = read(addr++);
  }
  return n;
}

  static Storage* initStorage();
};

class EEPROMStorage : public Storage {
public:
  EEPROMStorage() {}

  void write(size_t address, uint8_t val) override;
  uint8_t read(size_t address) override;
  int capacity() override { return 2048; } // 2KB
};

class FRAMStorage : public Storage {
public:
  FRAMStorage(Adafruit_FRAM_I2C fram) {
    fram = fram;
  }

  void write(size_t address, uint8_t val) override;
  uint8_t read(size_t address) override;
  int capacity() override { return 256000; } // 256Kbits / 32KB

private:
  Adafruit_FRAM_I2C fram;
};

// _INCLUDE_STORAGE_H
#endif
