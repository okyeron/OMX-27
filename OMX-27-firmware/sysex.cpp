/*
 * 16n Faderbank Configuration Sysex Processing
 * (c) 2020 by Tom Armitage
 * MIT License
 */

#include "sysex.h"

void processIncomingSysex(uint8_t* sysexData, unsigned size) {
	D(Serial.println("Ooh, sysex"));
	D(printHexArray(sysexData, size));
	D(Serial.println());

	if(size < 3) {
		D(Serial.println("That's an empty sysex, bored now"));
		return;
	}
	// F0 7D 00 00
	if(!(sysexData[1] == 0x7d && sysexData[2] == 0x00 && sysexData[3] == 0x00)) {
		D(Serial.println("That's not a sysex message for us"));
		return;
	}
	switch(sysexData[4]) {
		case 0x1f:
			// 1F = "1nFo" - please send me your current config
			D(Serial.println("Got an 1nFo request"));
			sendCurrentState();
			break;
		case 0x0e:
			// 0E - c0nfig Edit - here is a new config
			D(Serial.println("Incoming c0nfig Edit"));
			updateAllSettingsAndStoreInEEPROM(sysexData, size);
			break;
		case 0x0d:
			// 0D - c0nfig Device edit - new config just for device opts
			D(Serial.println("Incoming c0nfig Device edit"));
			updateDeviceSettingsAndStoreInEEPROM(sysexData, size);
			break;
//		case 0x0c:
//			// 0C - c0nfig usb edit - here is a new config just for usb
//			D(Serial.println("Incoming c0nfig usb edit"));
//			updateUSBSettingsAndStoreInEEPROM(sysexData, size);
//			break;
//		case 0x0b:
//			// 0B - c0nfig trs edit - here is a new config just for trs
//			D(Serial.println("Incoming c0nfig trs edit"));
//			updateTRSSettingsAndStoreInEEPROM(sysexData, size);
//			break;
	}
}

// 0E - c0nfig Edit
void updateAllSettingsAndStoreInEEPROM(uint8_t* newConfig, unsigned size) {
	// store the settings from sysex in flash
	// also update all our settings.
	D(Serial.print("Received a new config with size "));
	D(Serial.println(size));
	// D(printHexArray(newConfig,size));

	updateSettingsBlockAndStoreInEEPROM(newConfig,size,9,80,0);
}

// 0D - c0nfig Device edit
void updateDeviceSettingsAndStoreInEEPROM(uint8_t* newConfig, unsigned size) {
	// store the settings from sysex in flash
	// also update all our settings.
	D(Serial.print("Received a new device config with size "));
	D(Serial.println(size));
	// D(printHexArray(newConfig,size));

	updateSettingsBlockAndStoreInEEPROM(newConfig,size,5,32,0);
}

//void updateUSBSettingsAndStoreInEEPROM(int* newConfig, unsigned size) {
//	// store channels
//	updateSettingsBlockAndStoreInEEPROM(newConfig,size,5,16,16);
//	// store CCs
//	updateSettingsBlockAndStoreInEEPROM(newConfig,size,21,16,48);
//}
//
//void updateTRSSettingsAndStoreInEEPROM(int* newConfig, unsigned size) {
//	// store channels
//	updateSettingsBlockAndStoreInEEPROM(newConfig,size,5,16,32);
//	// store CCs
//	updateSettingsBlockAndStoreInEEPROM(newConfig,size,21,16,64);
//}

void updateSettingsBlockAndStoreInEEPROM(uint8_t* configFromSysex, unsigned sysexSize, int configStartIndex, int configDataLength, int EEPROMStartIndex) { 
	D(Serial.print("Storing data of size "));
	D(Serial.print(configDataLength));
	D(Serial.print(" at location "));
	D(Serial.print(EEPROMStartIndex));
	D(Serial.print(" from data of length "));
	D(Serial.print(sysexSize));
	D(Serial.print(" beginning at byte "));
	D(Serial.println(configStartIndex));
	D(printHexArray(configFromSysex, sysexSize));

	// walk the config, ignoring the top, tail, and firmware version
	uint8_t dataToWrite[configDataLength];

	for(int i = 0; i < (configDataLength); i++) {
		int configIndex = i + configStartIndex;
		dataToWrite[i] = configFromSysex[configIndex];
	}

	
// write new Data
	writeEEArray(EEPROMStartIndex, dataToWrite, configDataLength);

// now load that. -- maybe do this after action in main loop
// 	loadHeader();
}

void sendCurrentState() {
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
	readEEheader(0, buffer, EEPROM_HEADER_SIZE);

	int offset = 8;
	for(int i = 0; i < EEPROM_HEADER_SIZE; i++) {
	int data = buffer[i];
	if(data == 0xff) {
	  data = 0x7f;
	}
	sysexData[i+offset] = data;
	}

	D(Serial.println("Sending this data"));
	D(printHexArray(sysexData,EEPROM_HEADER_SIZE+offset));

	MM::sendSysEx(EEPROM_HEADER_SIZE+offset, sysexData, false);
	//  forceMidiWrite = true;
}