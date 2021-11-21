#include <EEPROM.h>
#include <Adafruit_FRAM_I2C.h>

#include "storage.h"

// Storage

Storage* Storage::initStorage() {
	Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();
	// check if FRAM chip can be initialised
	if (fram.begin()) {
		return new FRAMStorage(fram);
	}
	// fall back to EEPROM
	return new EEPROMStorage();
}

// EEPROM

void EEPROMStorage::write(size_t address, uint8_t value) {
	EEPROM.update(address, value);
}

uint8_t EEPROMStorage::read(size_t address) {
	return EEPROM.read(address);
}

// FRAM

void FRAMStorage::write(size_t address, uint8_t value) {
	this->fram.write(address, value);
}

uint8_t FRAMStorage::read(size_t address) {
	return this->fram.read(address);
}
