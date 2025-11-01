#pragma once

// #include "../config.h"
#include "../hardware/storage.h"
#include "midi.h"

class SysEx
{
	SysSettings *settings;
	Storage *storage;

public:
	SysEx(Storage *storage, SysSettings *settings) : storage(storage),
													 settings(settings) {}

	void processIncomingSysex(const uint8_t *sysexData, unsigned size);
	void updateAllSettingsAndStore(const uint8_t *newConfig, unsigned size);
	void updateDeviceSettingsAndStore(const uint8_t *newConfig, unsigned size);
	void updateDeviceSettings(const uint8_t *newConfig, unsigned size);
	void updateSettingsBlockAndStore(const uint8_t *configFromSysex, unsigned sysexSize, int configStartIndex, int configDataLength, int EEPROMStartIndex);
	void loadGlobals();
	void sendCurrentState();
};
