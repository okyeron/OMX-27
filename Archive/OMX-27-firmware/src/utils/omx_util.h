#pragma once
#include "../config.h"
#include "../utils/music_scales.h"
#include "../modes/omx_mode_interface.h"
#include "../modes/submodes/submode_clearstorage.h"

enum GlobalParams
{
	GPARAM_MOUT_OCT,
	GPARAM_MOUT_CHAN,
	GPARAM_MOUT_VEL,
	GPARAM_MIDI_THRU,
	GPARAM_MIDI_LASTNOTE,
	GPARAM_MIDI_LASTVEL,
	GPARAM_POTS_LASTVAL,
	GPARAM_POTS_LASTCC,
	GPARAM_POTS_PBANK,
	GPARAM_SCALE_ROOT,
	GPARAM_SCALE_PAT,
	GPARAM_SCALE_LOCK,
	GPARAM_SCALE_GRP16,
	GPARAM_MACRO_MODE,
	GPARAM_MACRO_CHAN,
	GPARAM_CLOCK_SOURCE,
	GPARAM_CLOCK_SEND
};

class OmxUtil
{
public:
	OmxUtil()
	{
	}

	void setup();

	void sendPots(int val, int channel);

	// random float between 0.0 to 1.0, inclusive
	static float randFloat();

	// Assumes b is greater than a
	static float lerp(float a, float b, float t);

	// #### Clocks, might want to put in own class
	void advanceClock(OmxModeInterface *activeOmxMode, Micros advance);
	void advanceSteps(Micros advance);
	void setGlobalSwing(int swng_amt);
	void resetPPQCounter();
	void resetClocks();
	void restartClocks();

	void startClocks();
	void resumeClocks();
	void stopClocks();

	bool areClocksRunning();

	// #### Outbound CV note on/off
	// void cvNoteOn(uint8_t notenum);
	// void cvNoteOff(uint8_t notenum);

	// #### Outbound MIDI note on/off
	void midiNoteOn(int notenum, int velocity, int channel);
	void midiNoteOn(MusicScales *scale, int notenum, int velocity, int channel);
	void midiNoteOff(int notenum, int channel);

	void allOff();

	MidiNoteGroup midiNoteOn2(MusicScales *scale, int notenum, int velocity, int channel);
	MidiNoteGroup midiNoteOff2(int notenum, int channel);

	MidiNoteGroup midiDrumNoteOn(uint8_t keyIndex, uint8_t notenum, int velocity, int channel);
	MidiNoteGroup midiDrumNoteOff(uint8_t keyIndex);

	// Used for global params defined in GlobalParams to avoid code duplication
	// called on Encoder update to edit a parameter
	void onEncoderChangedEditParam(Encoder::Update *enc, uint8_t selectedParmIndex, uint8_t targetParamIndex, uint8_t paramType);
	void onEncoderChangedEditParam(Encoder::Update *enc, MusicScales *musicScale, uint8_t selectedParmIndex, uint8_t targetParamIndex, uint8_t paramType);

	// Used for global page legends defined in GlobalParams to avoid code duplication
	void setupPageLegend(uint8_t index, uint8_t paramType);
	void setupPageLegend(MusicScales *musicScale, uint8_t index, uint8_t paramType);

	SubModeClearStorage subModeClearStorage;

private:
	// int potbank = 0;
	// int analogValues[5] = {0,0,0,0,0};		// default values
	// int potValues[5] = {0,0,0,0,0};
	// int potCC = pots[potbank][0];
	// int potVal = analogValues[0];

	// signed to avoid rollover
	signed long long timeToNextClock = 0;

	bool sendClocks_ = true;
	OmxModeInterface *activeOmxMode_;
};

extern OmxUtil omxUtil;
