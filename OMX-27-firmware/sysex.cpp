#include "sysex.h"
#include "MM.h"
#include "config.h"

const uint8_t INFO = 0x1F;
const uint8_t CONFIG_EDIT = 0x0E;
const uint8_t CONFIG_DEVICE_EDIT = 0x0D;

void SysEx::processIncomingSysex(const uint8_t* sysexData, unsigned size) {
	if(size < 3) {
// 		Serial.println("That's an empty sysex");
		return;
	}
	// F0 7D 00 00
	if(!(sysexData[1] == 0x7d && sysexData[2] == 0x00 && sysexData[3] == 0x00)) {
// 		Serial.println("Not a valid sysex message for us");
		return;
	}

	switch(sysexData[4]) {
		case INFO:
			// 1F = "1nFo" - please send me your current config
// 			Serial.println("Got an 1nFo request");
			this->sendCurrentState();
			break;
		case CONFIG_EDIT:
			// 0E - c0nfig Edit - here is a new config
// 			Serial.println("Got an c0nfig Edit");
			this->updateAllSettingsAndStore(sysexData, size);
			break;
		case CONFIG_DEVICE_EDIT:
			// 0D - c0nfig Device edit - new config just for device opts
// 			Serial.println("Got an c0nfig Device Edit");
			this->updateDeviceSettingsAndStore(sysexData, size);
			break;
		default:
			break;
//		case 0x0c:
//			// 0C - c0nfig usb edit - here is a new config just for usb
//			updateUSBSettingsAndStore(sysexData, size);
//			break;
//		case 0x0b:
//			// 0B - c0nfig trs edit - here is a new config just for trs
//			updateTRSSettingsAndStore(sysexData, size);
//			break;
	}
}

void SysEx::updateAllSettingsAndStore(const uint8_t* newConfig, unsigned size) {
	this->updateSettingsBlockAndStore(newConfig,size,9,80,0);
}

void SysEx::updateDeviceSettingsAndStore(const uint8_t* newConfig, unsigned size) {
	this->updateSettingsBlockAndStore(newConfig,size,5,32,0);
}

void SysEx::updateSettingsBlockAndStore(const uint8_t* configFromSysex, unsigned sysexSize, int configStartIndex, int configDataLength, int EEPROMStartIndex) {
	// walk the config, ignoring the top, tail, and firmware version
	uint8_t dataToWrite[configDataLength];

	for(int i = 0; i < (configDataLength); i++) {
		int configIndex = i + configStartIndex;
		dataToWrite[i] = configFromSysex[configIndex];
	}

	// write new Data
	this->storage->writeArray(EEPROMStartIndex, dataToWrite, configDataLength);
}

void SysEx::sendCurrentState() {
	//   0F - "c0nFig" - outputs its config:
	uint8_t sysexData[EEPROM_HEADER_SIZE+8];

	sysexData[0] = 0x7d; // manufacturer
	sysexData[1] = 0x00;
	sysexData[2] = 0x00;

	sysexData[3] = 0x0F; // ConFig;

	sysexData[4] = DEVICE_ID; // Device 01, ie, dev board
	sysexData[5] = MAJOR_VERSION; // major version
	sysexData[6] = MINOR_VERSION; // minor version
	sysexData[7] = POINT_VERSION; // point version

	// 	32 bytes of data:
	// 	EEPROM VERSION
	// 	MODE
	// 	PlayingPattern
	//	MidiChannel
	//	Pots (x25 - 5 banks of 5 pots)
	// 	00
	// 	00
	// 	00

	uint8_t buffer[EEPROM_HEADER_SIZE];
	this->storage->readArray(0, buffer, EEPROM_HEADER_SIZE);

	int offset = 8;
	for(int i = 0; i < EEPROM_HEADER_SIZE; i++) {
		int data = buffer[i];
		if(data == 0xff) {
		  data = 0x7f;
		}
		sysexData[i+offset] = data;
	}

	MM::sendSysEx(EEPROM_HEADER_SIZE + offset, sysexData, false);
}
