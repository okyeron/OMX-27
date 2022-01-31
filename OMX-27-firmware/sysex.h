#pragma once
#include <Arduino.h>
#include "config.h"
#include "MM.h"
#include "utils.h"


void processIncomingSysex(uint8_t* sysexData, unsigned size);
void updateAllSettingsAndStoreInEEPROM(uint8_t* newConfig, unsigned size);
void updateDeviceSettingsAndStoreInEEPROM(uint8_t* newConfig, unsigned size);
void updateSettingsBlockAndStoreInEEPROM(uint8_t* configFromSysex, unsigned sysexSize, int configStartIndex, int configDataLength, int EEPROMStartIndex);
void sendCurrentState();
