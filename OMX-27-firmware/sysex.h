#pragma once

#include "storage.h"

class SysEx {
	Storage *storage;

public:

	SysEx(Storage* storage) {
		this->storage = storage;
	}

	void processIncomingSysex(uint8_t* sysexData, unsigned size);
	void updateAllSettingsAndStore(uint8_t* newConfig, unsigned size);
	void updateDeviceSettingsAndStore(uint8_t* newConfig, unsigned size);
	void updateSettingsBlockAndStore(uint8_t* configFromSysex, unsigned sysexSize, int configStartIndex, int configDataLength, int EEPROMStartIndex);
	void sendCurrentState();

};
