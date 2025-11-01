#pragma once

#include <Adafruit_FRAM_I2C.h>

enum StorageType
{
	EEPROM_MEMORY = 0,
	FRAM_MEMORY = 1
};

// EEPROM available
// Teensy 3.2	2048 bytes
// Teensy 4.0	1080 bytes
// FRAM = 32 KBytes


// abstract storage class
class Storage
{
public:
	static Storage *initStorage();

	virtual int capacity() = 0;

	// read/write bytes
	virtual void write(size_t address, uint8_t val) = 0;
	virtual uint8_t read(size_t address) = 0;
	virtual bool isEeprom() = 0;

	void readArray(size_t address, uint8_t buffer[], int length);
	void writeArray(size_t address, uint8_t buffer[], int length);

	// reset entire storage back to 0
	void clear()
	{
		for (int address = 0; address < capacity(); address++)
		{
			write(address, 0);
		}
	}

	// template reader/writer implementation copied from Adafruit_FRAM_I2C which implements them both
	// in terms of reading/writing bytes

	/**************************************************************************/
	/*!
			@brief  Write any object to memory
			@param addr
									The 16-bit address to write to in EEPROM memory
			@param value  The templated object we will be writing
			@returns    The number of bytes written
	*/
	/**************************************************************************/
	template <class T>
	uint16_t writeObject(uint16_t addr, const T &value)
	{
		const byte *p = (const byte *)(const void *)&value;
		uint16_t n;
		for (n = 0; n < sizeof(value); n++)
		{
			write(addr++, *p++);
		}
		return n;
	}

	/**************************************************************************/
	/*!
			@brief Read any object from memory
			@param addr
									The 16-bit address to write to in EEPROM memory
			@param value  The address of the templated object we will be writing INTO
			@returns    The number of bytes read
	*/
	/**************************************************************************/
	template <class T>
	uint16_t readObject(uint16_t addr, T &value)
	{
		byte *p = (byte *)(void *)&value;
		uint16_t n;
		for (n = 0; n < sizeof(value); n++)
		{
			*p++ = read(addr++);
		}
		return n;
	}

protected:
	Storage() {}
};

class EEPROMStorage : public Storage
{
public:
	EEPROMStorage() {}

	bool isEeprom() override { return true; }
	void write(size_t address, uint8_t val) override;
	uint8_t read(size_t address) override;
	int capacity() override { return 2048; } // 2KB
};

class FRAMStorage : public Storage
{
public:
	FRAMStorage(Adafruit_FRAM_I2C fram)
	{
		this->fram = fram;
	}

	bool isEeprom() override { return false; }
	void write(size_t address, uint8_t val) override;
	uint8_t read(size_t address) override;
	int capacity() override { return 32000; } // 32KB

private:
	Adafruit_FRAM_I2C fram;
};
