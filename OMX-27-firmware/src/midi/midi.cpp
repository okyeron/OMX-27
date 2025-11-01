#include <MIDI.h>
#include "../globals.h"
#include "./midi.h"
#include "../consts/consts.h"
#include "../config.h"
#include "../utils/omx_util.h"
#include "../hardware/omx_disp.h"
#include "../utils/cvNote_util.h"
#include "../modes/omx_screensaver.h"
#include "../modes/omx_mode_interface.h"
#include "../modes/sequencer.h"
#include "sysex.h"

extern OmxModeInterface *activeOmxMode;
extern OmxScreensaver omxScreensaver;
extern SequencerState sequencer;


namespace
{

#if BOARDTYPE == OMX2040
	Adafruit_USBD_MIDI usb_midi;
	MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, usbMIDI);      // USBMIDI is USB MIDI
	MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, HWMIDI);           // HWMIDI is Hardware MIDI

#else
	using SerialMIDI = midi::SerialMIDI<HardwareSerial>;
	using MidiInterface = midi::MidiInterface<SerialMIDI>;
	SerialMIDI theSerialInstance(Serial1);
	MidiInterface HWMIDI(theSerialInstance);

#endif
}

namespace MM
{	
	void begin()
	{
		#if BOARDTYPE == OMX2040
			HWMIDI.begin(MIDI_CHANNEL_OMNI);
			usbMIDI.begin(MIDI_CHANNEL_OMNI);

			HWMIDI.turnThruOff();
			usbMIDI.turnThruOff();

			// handlers / callbacks
			usbMIDI.setHandleNoteOn(handleNoteOn);
			usbMIDI.setHandleNoteOff(handleNoteOff);
			usbMIDI.setHandleClock(handleClock);
			usbMIDI.setHandleStart(handleStart);
			usbMIDI.setHandleStop(handleStop);
			usbMIDI.setHandleContinue(handleContinue);
			usbMIDI.setHandleControlChange(handleControlChange);
			usbMIDI.setHandleSystemExclusive(OnSysEx);
			
			HWMIDI.setHandleNoteOn(handleNoteOn);
			HWMIDI.setHandleNoteOff(handleNoteOff);
			HWMIDI.setHandleClock(handleClock);
			HWMIDI.setHandleStart(handleStart);
			HWMIDI.setHandleStop(handleStop);
			HWMIDI.setHandleContinue(handleContinue);
			HWMIDI.setHandleControlChange(handleControlChange);
			HWMIDI.setHandleSystemExclusive(OnSysExHW);
		#else
			HWMIDI.begin();
		#endif
	}
	// #### Inbound MIDI callbacks

	// void onControlChange(byte channel, byte number, byte value){
	// 	// if bank select MSB (0)- set flag 
	// 	// if flag, then look for next CC - LSB (32), 
	// 	// then do bank change and reset flag
	// 	// or if not 32, reset flag
	// }

	void handleNoteOn(byte channel, byte note, byte velocity)
	{
		digitalWrite(BLUELED, HIGH);
		if (midiSettings.midiSoftThru)
		{
			sendNoteOnHW(note, velocity, channel);
		}
		if (midiSettings.midiInToCV)
		{
			cvNoteUtil.cvNoteOn(note);
		}
		omxScreensaver.resetCounter();
		activeOmxMode->inMidiNoteOn(channel, note, velocity);
	}

	void handleNoteOff(byte channel, byte note, byte velocity)
	{
		digitalWrite(BLUELED, LOW);
		if (midiSettings.midiSoftThru)
		{
			sendNoteOffHW(note, velocity, channel);
		}
		if (midiSettings.midiInToCV)
		{
			cvNoteUtil.cvNoteOff(note);
		}
		activeOmxMode->inMidiNoteOff(channel, note, velocity);
	}
	
	void handleControlChange(byte channel, byte control, byte value)
	{
		// digitalWrite(REDLED, HIGH);
		if (midiSettings.midiSoftThru)
		{
			sendControlChangeHW(control, value, channel);
		}
		// change potbank on bank select
		if (control == 0){
			midiSettings.isBankSelect = true;
			potSettings.potbank = constrain(value, 0, NUM_CC_BANKS - 1);
			omxDisp.setDirty();
		// }else if (midiSettings.isBankSelect && control == 32){
		// 	midiSettings.isBankSelect = true;
		}else{
			midiSettings.isBankSelect = false;
		}

		// sendControlChange(control, value, channel);
	}

	// absolute_time_t last_ext_tick_at_ = 0;
	// void externalMidiClockTick(absolute_time_t timestamp) {
	// 	uint32_t delta = absolute_time_diff_us(last_ext_tick_at_, timestamp);
	// 	if ( delta > 0) {
	// 		clockConfig.ppqInterval = delta / 4 ; 
	// 		clockConfig.clockbpm = (60000000 / clockConfig.ppqInterval) / PPQ;

	// 		last_ext_tick_at_ = timestamp;
	// 	}
	// }

	// FIXME: This is debug stuff for incoming midi clock average.
	// unsigned int cnt;
	// int cntmax = 24;
		void handleClock() {
		// start a rolling average clock
		// PPQN for MIDI is 24

		// bool clockSource;	// Internal clock (0), external clock (1)

		if (sequencer.clockSource == 1){ // external clock
		
		// 	absolute_time_t Now = time_us_32();
		// 	externalMidiClockTick(Now);
		// 	// omxDisp.setDirty();
		// }

/*
		if (cnt == cntmax)
		{
			Serial.print("BPM: ");
			Serial.print(clockstats.getBPM());
			Serial.print(" Last Interval: ");
			Serial.print(clockstats.getLastInterval());
			Serial.print(" Samples: ");
			Serial.println(clockstats.getSampleCount());
			cnt = 0;
		}
		*/
			clockstats.clockPulse(micros());
			clockConfig.clockbpm = clockstats.getBPM();
		}

		if (midiSettings.midiSoftThru){
			// sendClock();
		}
	}

	void handleStart() {
		digitalWrite(REDLED, HIGH);
		clockstats.start();
		startTransport();
		if (midiSettings.midiSoftThru){
		}
	}

	void handleStop() {
		digitalWrite(REDLED, LOW);
		clockstats.stop();
		stopTransport();
		if (midiSettings.midiSoftThru){
		}
	}

	void handleContinue() {
		digitalWrite(REDLED, HIGH);
		continueTransport();
		if (midiSettings.midiSoftThru){
		}
	}

	void OnSysEx(byte *sysexData, unsigned length)
	{
		sysEx->processIncomingSysex(sysexData, length);
	}
	void OnSysExHW(byte* sysexData, unsigned length) 
	{
		sendSysEx(length, sysexData, false);
	}

	void sendNoteOn(int note, int velocity, int channel)
	{
		HWMIDI.sendNoteOn(note, velocity, channel);
		usbMIDI.sendNoteOn(note, velocity, channel);
	}

	void sendNoteOnHW(int note, int velocity, int channel)
	{
		HWMIDI.sendNoteOn(note, velocity, channel);
	}

	void sendNoteOff(int note, int velocity, int channel)
	{
		HWMIDI.sendNoteOff(note, velocity, channel);
		usbMIDI.sendNoteOff(note, velocity, channel);
	}

	void sendNoteOffHW(int note, int velocity, int channel)
	{
		HWMIDI.sendNoteOff(note, velocity, channel);
	}

	void sendControlChange(int control, int value, int channel)
	{
		HWMIDI.sendControlChange(control, value, channel);
		usbMIDI.sendControlChange(control, value, channel);
	}

	void sendControlChangeHW(int control, int value, int channel)
	{
		HWMIDI.sendControlChange(control, value, channel);
	}
	
	void sendProgramChange(byte program, byte channel)
	{
		// Bank switch?
		HWMIDI.sendProgramChange(program, channel);
		usbMIDI.sendProgramChange(program, channel);
	}

	void sendSysEx(uint32_t length, const uint8_t *sysexData, bool hasBeginEnd)
	{
		usbMIDI.sendSysEx(length, sysexData, hasBeginEnd);
		HWMIDI.sendSysEx(length, sysexData, hasBeginEnd);
	}

	void sendClock()
	{
		// usbMIDI.sendRealTime(midi::Clock);
		if (sequencer.clockSource == 0){ // internal clock
			usbMIDI.sendClock();
			HWMIDI.sendClock();
		}
	}

	void startTransport()
	{
		// usbMIDI.sendRealTime(midi::Start);
		// Serial.println("Start received");
		usbMIDI.sendStart();
		HWMIDI.sendStart();
	}

	void continueTransport()
	{
		// usbMIDI.sendRealTime(midi::Continue);
		// Serial.println("Continue received");
		usbMIDI.sendContinue();
		HWMIDI.sendContinue();
	}

	void stopTransport()
	{
		// usbMIDI.sendRealTime(midi::Stop);
		// Serial.println("Stop received");
		usbMIDI.sendStop();
		HWMIDI.sendStop();
	}

	// NEED SOMETHING FOR usbMIDI.read() / MIDI.read()

	bool usbMidiRead()
	{
		return usbMIDI.read();
	}

	bool midiRead()
	{
		return HWMIDI.read();
	}
}
