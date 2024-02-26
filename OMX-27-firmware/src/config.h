#pragma once

#ifndef OMX_CONFIG_DONE
#define OMX_CONFIG_DONE // prevent redifinition pragma once should handle though.

#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <ResponsiveAnalogRead.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include "consts/colors.h"

// #include <cstdarg>

/* * firmware metadata  */
// OMX_VERSION = 1.12.18
const int MAJOR_VERSION = 1;
const int MINOR_VERSION = 12;
const int POINT_VERSION = 18;

const int DEVICE_ID = 2;

// DAC
extern Adafruit_MCP4725 dac;

enum OMXMode
{
	MODE_MIDI = 0,
	MODE_DRUM,
	MODE_CHORDS,
	MODE_S1,
	MODE_S2,
	MODE_GRIDS,
	MODE_EUCLID,
	MODE_OM,

	NUM_OMX_MODES
};

enum MIDIFXTYPE
{
	MIDIFX_NONE,
	MIDIFX_CHANCE,
	MIDIFX_TRANSPOSE,
	MIDIFX_RANDOMIZER,
	MIDIFX_SELECTOR,
	MIDIFX_CHORD,
	MIDIFX_HARMONIZER,
	MIDIFX_SCALER,
	MIDIFX_MONOPHONIC,
	MIDIFX_REPEAT,
	MIDIFX_ARP,
	MIDIFX_COUNT
};

extern const OMXMode DEFAULT_MODE;

enum FUNCKEYMODE
{
	FUNCKEYMODE_NONE, // No function keys
	FUNCKEYMODE_F1,	  // F1 held
	FUNCKEYMODE_F2,	  // F2 held
	FUNCKEYMODE_F3	  // F1 + F3 held
};

// Increment this when data layout in EEPROM changes. May need to write version upgrade readers when this changes.
extern const uint8_t EEPROM_VERSION;

#define EEPROM_HEADER_ADDRESS 0
#define EEPROM_HEADER_SIZE 36
#define EEPROM_PATTERN_ADDRESS 64

// next address 1104 (was 1096 before clock)

extern const byte DAC_ADDR;

// DEFINE CC NUMBERS FOR POTS // CCS mapped to Organelle Defaults
extern const int CC1;
extern const int CC2;
extern const int CC3;
extern const int CC4;
extern const int CC5; // change to 25 for EYESY Knob 5

extern const int CC_AUX; // Mother mode - AUX key
extern const int CC_OM1; // Mother mode - enc switch
extern const int CC_OM2; // Mother mode - enc turn

extern const int LED_BRIGHTNESS;

// DONT CHANGE ANYTHING BELOW HERE

extern const int LED_PIN;
extern const int LED_COUNT;

// POTS/ANALOG INPUTS - teensy pins for analog inputs
extern const int analogPins[];

#define NUM_CC_BANKS 5
#define NUM_CC_POTS 5
extern int pots[NUM_CC_BANKS][NUM_CC_POTS]; // the MIDI CC (continuous controller) for each analog input

using Micros = unsigned long; // for tracking time per pattern

struct SysSettings
{
	OMXMode omxMode = DEFAULT_MODE;
	OMXMode newmode = DEFAULT_MODE;
	uint8_t midiChannel = 0;
	int playingPattern;
	bool refresh = false;
	bool screenSaverMode = false;
	unsigned long timeElasped;
};

extern SysSettings sysSettings;

extern const int potCount;

struct PotSettings
{
	ResponsiveAnalogRead *analog[NUM_CC_POTS];

	// ANALOGS
	int potbank = 0;
	int analogValues[NUM_CC_POTS] = {0, 0, 0, 0, 0}; // default values
	int potValues[NUM_CC_POTS] = {0, 0, 0, 0, 0};
	int hiResPotVal[NUM_CC_POTS] = {0, 0, 0, 0, 0};
	int potCC = pots[potbank][0];
	int potVal = analogValues[0];
	int potNum = 0;
};
// Put in global struct to share across classes
extern PotSettings potSettings;

extern int potMinVal;
extern int potMaxVal;

struct MidiConfig
{
	uint8_t defaultVelocity = 100;
	int octave = 0; // default C4 is 0 - range is -4 to +5
	// int newoctave = octave;
	int transpose = 0;
	int rotationAmt = 0;

	uint8_t swing = 0;
	const int maxswing = 100;
	// int swing_values[maxswing] = {0, 1, 3, 5, 52, 66, 70, 72, 80, 99 }; // 0 = off, <50 early swing , >50 late swing, 99=drunken swing

	bool keyState[27] = {false};
	int midiKeyState[27] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	int midiChannelState[27] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	int rrChannel = 0;
	bool midiRoundRobin = false;
	int midiRRChannelOffset = 0;
	int midiRRChannelCount = 1;
	uint8_t midiLastNote = 0;
	uint8_t midiLastVel = 0;
	int currpgm = 0;
	int currbank = 0;
	bool midiInToCV = true;
	bool midiSoftThru = false;
	int pitchCV;
	bool midiAUX = false;
};

extern MidiConfig midiSettings;

// struct MidiPage {
// 	const int numPages = 4;
// 	const int numParams = numPages * 5;
// 	int miparam = 0; // midi params item counter
//     int mmpage = 0;
// };
// extern MidiPage midiPageParams;

struct MidiMacroConfig
{
	int midiMacro = 0;
	bool m8AUX = false;
	int midiMacroChan = 10;
};

extern MidiMacroConfig midiMacroConfig;

// extern bool m8mutesolo[];

struct EncoderConfig
{
	bool enc_edit = false;
};

extern EncoderConfig encoderConfig;

struct ClockConfig
{
	float clockbpm = 120;
	float newtempo = clockbpm;
	unsigned long tempoStartTime;
	unsigned long tempoEndTime;
	float step_delay; // 16th note step length in milliseconds
	unsigned long minDelta = 5000;

	volatile unsigned long step_micros; // 16th note step in microseconds (quarter of quarter note), 124992 for 120 bpm : 35712 for 300 bpm
	volatile unsigned long ppqInterval; // time in microseconds between clock ticks,  5208 or 5.2ms for 120 bpm : 1488 for 300 bpm, 5.2 * 96 = 500ms
};

extern ClockConfig clockConfig;

struct SequencerConfig
{
	int selectedStep = 0;
	int selectedNote = 0;

	bool plockDirty[NUM_CC_POTS] = {false, false, false, false, false};
	int prevPlock[NUM_CC_POTS] = {0, 0, 0, 0, 0};

	volatile unsigned long noteon_micros;
	volatile unsigned long noteoff_micros;

	uint32_t currentFrameMicros;
	uint32_t lastClockMicros;

	uint8_t midiOutClockTick; // Shouldn't be modified

	uint8_t currentClockTick; // Counter that wraps from 0-96 on the clock tick. currentClockTick % 96 will align with global 1/4 note, currentClockTick % 96/2=48 global 8th note and 96/4=24 global 16th note

	int numOfActiveArps = 0;

	// bool noteSelect = false;
	// bool noteSelection = false;

	// int omxSeqSelectedStep = 0;
	// bool stepSelect = false;
	// bool stepRecord = false;
	// bool stepDirty = false;
};
extern SequencerConfig seqConfig;

// struct SequencerPage {
// 	bool patternParams = false;
//     bool seqPages = false;

// 	int nspage = 0;
//     int pppage = 0;
//     int sqpage = 0;
//     int srpage = 0;

//     int nsparam = 0; // note select params
//     int ppparam = 0; // pattern params
//     int sqparam = 0; // seq params
//     int srparam = 0; // step record params
// };
// extern SequencerPage seqPageParams;

struct ColorConfig
{
	uint32_t screensaverColor = 0xFF0000;
	uint32_t stepColor = 0x000000;
	uint32_t muteColor = 0x000000;
	uint16_t midiBg_Hue = 0;
	uint8_t midiBg_Sat = 255;
	uint8_t midiBg_Brightness = 255;

	uint32_t selMidiFXGRPOffColor = SALMON; // Color of FX Group key when selected
	uint32_t midiFXGRPOffColor = RED;		// Color of FX Group key to turn off MidiFX
	uint32_t selMidiFXGRPColor = LTCYAN;
	uint32_t midiFXGRPColor = BLUE;
	uint32_t midiFXEmptyColor = 0x080808;

	uint32_t arpOn = MINT;
	uint32_t arpOff = DKCYAN;
	uint32_t arpHoldOn = YELLOW;
	uint32_t arpHoldOff = DKYELLOW;

	uint32_t gotoArpParams = DKORANGE;
	uint32_t nextArpPattern = DKPURPLE;
	uint32_t nextArpOctave = DKPURPLE;

	uint32_t octDnColor = ORANGE;
	uint32_t octUpColor = RBLUE;

	uint32_t mfxNone = LEDOFF; 
	uint32_t mfxChance = MEDRED; 
	uint32_t mfxTranspose = PURPLE;		
	uint32_t mfxRandomizer = RED;
	uint32_t mfxSelector = ORANGE;
	uint32_t mfxChord = CYAN;
	uint32_t mfxHarmonizer = ROSE;
	uint32_t mfxScaler = YELLOW;
	uint32_t mfxMonophonic = INDIGO;
	uint32_t mfxRepeat = RED;
	uint32_t mfxArp = BLUE;

	uint32_t getMidiFXColor(uint8_t mfxType)
	{
		switch (mfxType)
		{
		case MIDIFX_NONE:
			return mfxNone;
		case MIDIFX_CHANCE:
			return mfxChance;
		case MIDIFX_TRANSPOSE:
			return mfxTranspose;
		case MIDIFX_RANDOMIZER:
			return mfxRandomizer;
		case MIDIFX_SELECTOR:
			return mfxSelector;
		case MIDIFX_CHORD:
			return mfxChord;
		case MIDIFX_HARMONIZER:
			return mfxHarmonizer;
		case MIDIFX_SCALER:
			return mfxScaler;
		case MIDIFX_MONOPHONIC:
			return mfxMonophonic;
		case MIDIFX_REPEAT:
			return mfxRepeat;
		case MIDIFX_ARP:
			return mfxArp;
		};

		return LEDOFF;
	}
};

extern ColorConfig colorConfig;

struct ScaleConfig
{
	int scaleRoot = 0;
	int scalePattern = -1;
	bool lockScale = false; // If Scale is locked you will be unable to play notes out of the scale.
	bool group16 = false;	// If group16 is active, all notes in scale will be grouped into lower 16 notes.
	bool scaleSelectHold;
	bool showScaleInSeq = false;
};

extern ScaleConfig scaleConfig;

struct MidiNoteGroup
{
	uint8_t channel = 1;
	uint8_t noteNumber = 0;
	// uint8_t keyIndex = 0; // use if t
	uint8_t prevNoteNumber = 0; // note number before being modified by midiFX
	uint8_t velocity = 100;
	float stepLength = 0; // fraction or multiplier of clockConfig.step_micros, 1 == 1 step
	bool sendMidi = true;
	bool sendCV = true;
	uint32_t noteonMicros = 0;
	bool unknownLength = false;
	bool noteOff = false; // Set true if note off, corresponding note on should have stepLength of 0
};

#define NUM_DISP_PARAMS 5

extern const float kNoteLengths[];
extern const uint8_t kNumNoteLengths;

extern const uint8_t kArpRates[];
extern const uint8_t kNumArpRates;

extern const int gridh;
extern const int gridw;
extern const int PPQ;

extern const char *mfxOffMsg;
extern const char *mfxArpEditMsg;
extern const char *exitMsg;

extern const char *modes[];
extern const char *macromodes[];
extern const int nummacromodes;
extern const char *infoDialogText[];

extern String tempString;
extern String tempStrings[];

enum multDiv
{
	MD_QUART = 0,
	MD_HALF,
	MD_ONE,
	MD_TWO,
	MD_FOUR,
	MD_EIGHT,
	MD_SIXTEEN,

	NUM_MULTDIVS
};

extern float multValues[];
extern const char *mdivs[];

enum Dialogs
{
	COPY = 0,
	PASTE,
	CLEAR,
	RESET,
	FWD,
	REV,
	SAVED,
	SAVE,

	NUM_DIALOGS
};
struct InfoDialogs
{
	const char *text;
	bool state;
};

extern InfoDialogs infoDialog[NUM_DIALOGS];

enum SubModes
{
	SUBMODE_MIDI = 0,
	SUBMODE_SEQ,
	SUBMODE_SEQ2,
	SUBMODE_NOTESEL,
	SUBMODE_NOTESEL2,
	SUBMODE_NOTESEL3,
	SUBMODE_PATTPARAMS,
	SUBMODE_PATTPARAMS2,
	SUBMODE_PATTPARAMS3,
	SUBMODE_STEPREC,
	SUBMODE_MIDI2,
	SUBMODE_MIDI3,
	SUBMODE_MIDI4,

	SUBMODES_COUNT
};

// KEY SWITCH ROWS/COLS
#define ROWS 5 // five rows
#define COLS 6 // six columns

// Map the keys
extern char keys[ROWS][COLS];
extern byte rowPins[ROWS]; // row pins for key switches
extern byte colPins[COLS]; // column pins for key switches

// KEYBOARD MIDI NOTE LAYOUT
extern const int notes[];
extern const int steps[];
extern const int midiKeyMap[];

#endif
