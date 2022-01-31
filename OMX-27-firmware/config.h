#pragma once

#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
// #include <cstdarg>

//const int OMX_VERSION = 1.4.4;

enum OMXMode
{
	MODE_MIDI = 0,
	MODE_S1,
	MODE_S2,
	MODE_OM,

	NUM_OMX_MODES
};

extern const OMXMode DEFAULT_MODE;

// Increment this when data layout in EEPROM changes. May need to write version upgrade readers when this changes.
extern const uint8_t EEPROM_VERSION;

#define EEPROM_HEADER_ADDRESS            0
#define EEPROM_HEADER_SIZE               32
#define EEPROM_PATTERN_ADDRESS           32

// next address 1104 (was 1096 before clock)

// DEFINE CC NUMBERS FOR POTS // CCS mapped to Organelle Defaults
extern const int CC1;
extern const int CC2;
extern const int CC3;
extern const int CC4;
extern const int CC5;       // change to 25 for EYESY Knob 5

extern const int CC_AUX;    // Mother mode - AUX key
extern const int CC_OM1;    // Mother mode - enc switch
extern const int CC_OM2;    // Mother mode - enc turn

extern const int LED_BRIGHTNESS;

// DONT CHANGE ANYTHING BELOW HERE

extern const int LED_PIN;
extern const int LED_COUNT;

// POTS/ANALOG INPUTS - teensy pins for analog inputs
extern const int analogPins[];

#define NUM_CC_BANKS 5
#define NUM_CC_POTS 5
extern int pots[NUM_CC_BANKS][NUM_CC_POTS];         // the MIDI CC (continuous controller) for each analog input

#define NUM_DISP_PARAMS 5

extern const int gridh;
extern const int gridw;
extern const int PPQ;

extern const char* modes[];
extern const char* infoDialogText[];

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
extern const char* mdivs[];

enum Dialogs{
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
struct InfoDialogs {
	const char*  text;
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

	SUBMODES_COUNT
};

// KEY SWITCH ROWS/COLS
#define ROWS 5 //five rows
#define COLS 6 //six columns

// Map the keys
extern char keys[ROWS][COLS];
extern byte rowPins[ROWS]; // row pins for key switches
extern byte colPins[COLS]; // column pins for key switches

// KEYBOARD MIDI NOTE LAYOUT
extern const int notes[];
extern const int steps[];
extern const int midiKeyMap[];
