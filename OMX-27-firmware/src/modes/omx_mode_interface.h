#pragma once
#include "../ClearUI/ClearUI_Input.h"
#include "../hardware/omx_keypad.h"
#include "../config.h"

class OmxModeInterface
{
public:
	OmxModeInterface() {}
	virtual ~OmxModeInterface() {}

	virtual void InitSetup() {} // Called once when mode is created

	virtual void onModeActivated() {}	// Called whenever entering mode
	virtual void onModeDeactivated() {} // Called whenever entering mode

	virtual void onClockTick() {}

	virtual void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) = 0;
	virtual void loopUpdate(Micros elapsedTime) {}
	virtual void updateLEDs() = 0;
	virtual void onEncoderChanged(Encoder::Update enc) = 0;
	virtual void onEncoderButtonDown() = 0;
	virtual void onEncoderButtonUp(){};
	virtual void onEncoderButtonUpLong(){};

	virtual bool shouldBlockEncEdit() { return false; } // return true if should block encoder mode switch / hold down encoder
	virtual void onEncoderButtonDownLong() = 0;			// Will only get called if shouldBlockEncEdit() returns true

	virtual void onKeyUpdate(OMXKeypadEvent e) = 0;
	virtual void onKeyHeldUpdate(OMXKeypadEvent e){};

	virtual void onDisplayUpdate(){};

<<<<<<< HEAD:OMX-27-firmware/src/omx_mode_interface.h
	// #### Inbound MIDI callbacks
	virtual void inMidiNoteOn(byte channel, byte note, byte velocity) {}
	virtual void inMidiNoteOff(byte channel, byte note, byte velocity) {}
	virtual void inMidiControlChange(byte channel, byte control, byte value) {}
=======
    // #### Inbound MIDI callbacks
    virtual void inMidiNoteOn(byte channel, byte note, byte velocity) {}
    virtual void inMidiNoteOff(byte channel, byte note, byte velocity) {}
    virtual void inMidiControlChange(byte channel, byte control,  byte value) {}

>>>>>>> 5fe2be8 (File organization and includePath updates):OMX-27-firmware/src/modes/omx_mode_interface.h
};
