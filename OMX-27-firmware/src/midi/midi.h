#pragma once

#include <Arduino.h>
#include <MIDI.h>

#include "../consts/consts.h"
#include "../modes/omx_mode_interface.h"

#if BOARDTYPE == OMX2040
#include <Adafruit_TinyUSB.h>	// https://github.com/adafruit/Adafruit_TinyUSB_Arduino
#endif

namespace MM
{

	void begin();

	void sendNoteOn(int note, int velocity, int channel);
	void sendNoteOff(int note, int velocity, int channel);
	void sendControlChange(int control, int value, int channel);
	void sendProgramChange(byte program, byte channel);
	void sendNoteOnHW(int note, int velocity, int channel);
	void sendNoteOffHW(int note, int velocity, int channel);
	void sendControlChangeHW(int control, int value, int channel);
	void sendSysEx(uint32_t length, const uint8_t *sysexData, bool hasBeginEnd);

	// handlers/callbacks?
	// void handleProgramChange(byte program, byte channel);
	void handleNoteOn(byte channel, byte note, byte velocity);
	void handleNoteOff(byte channel, byte note, byte velocity);
	void handleControlChange(byte channel, byte number, byte value);

	void OnControlChange(byte channel, byte number, byte value);
	void OnSysEx(unsigned char *data, unsigned length);
	void OnSysExHW(unsigned char *data, unsigned length);
	
	void handleClock();
	void handleStart();
	void handleStop();
	void handleContinue();
		
	void sendClock();
	void startTransport();
	void continueTransport();
	void stopTransport();

	bool usbMidiRead();
	bool midiRead();


}
