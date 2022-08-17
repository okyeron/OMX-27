#include "config.h"
#include "consts.h"

const OMXMode DEFAULT_MODE = MODE_MIDI;
const uint8_t EEPROM_VERSION = 23;

// DEFINE CC NUMBERS FOR POTS // CCS mapped to Organelle Defaults
const int CC1 = 21;
const int CC2 = 22;
const int CC3 = 23;
const int CC4 = 24;
const int CC5 = 7;      // change to 25 for EYESY Knob 5

const int CC_AUX = 25; // Mother mode - AUX key
const int CC_OM1 = 26; // Mother mode - enc switch
const int CC_OM2 = 28; // Mother mode - enc turn

const int LED_BRIGHTNESS = 50;

// DONT CHANGE ANYTHING BELOW HERE

const int LED_PIN  = 14;
const int LED_COUNT = 27;

#if DEV
	const int analogPins[] = {23,22,21,20,16};  // DEV/beta boards
#elif MIDIONLY
	const int analogPins[] = {23,22,21,20,16};  // on MIDI only boards - {23,A10,21,20,16} on Bodged MIDI boards
#else
	const int analogPins[] = {34,22,21,20,16}; // on 1.0
#endif

const int potCount = NUM_CC_POTS;

int pots[NUM_CC_BANKS][NUM_CC_POTS] = {
	{CC1,CC2,CC3,CC4,CC5},
	{29,30,31,32,33},
	{34,35,36,37,38},
	{39,40,41,42,43},
	{91,93,103,104,7}
};          // the MIDI CC (continuous controller) for each analog input

const int gridh = 32;
const int gridw = 128;
const int PPQ = 96;

const char* modes[] = {"MI","CHORDS", "S1","S2","OM","GR", "EL"};
const char* macromodes[] = {"Off", "M8", "NRN"};
const int nummacromodes = 2;

float multValues[] = {.25, .5, 1, 2, 4, 8, 16};
const char* mdivs[] = {"1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "W"};

const float kNoteLengths[] = {0.10, 0.25, 0.5, 0.75, 1, 1.5, 2, 4, 8, 16};
const uint8_t kNumNoteLengths = 10;


// KEY SWITCH ROWS/COLS

// Map the keys
char keys[ROWS][COLS] = {
	{0, 1, 2, 3, 4, 5},
	{6, 7, 8, 9, 10,26},
	{11,12,13,14,15,24},
	{16,17,18,19,20,25},
	{22,23,21}
};
byte rowPins[ROWS] = {6, 4, 3, 5, 2}; // row pins for key switches
byte colPins[COLS] = {7, 8, 10, 9, 15, 17}; // column pins for key switches

// KEYBOARD MIDI NOTE LAYOUT
const int notes[] = {0,
	 61,63,   66,68,70,   73,75,   78,80,82,
59,60,62,64,65,67,69,71,72,74,76,77,79,81,83,84};

const int steps[] = {0,
	 1,2,   3,4,5,   6,7,   8,9,10,
11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};

const int midiKeyMap[] = {12,1,13,2,14,15,3,16,4,17,5,18,19,6,20,7,21,22,8,23,9,24,10,25,26};



SysSettings sysSettings;
PotSettings potSettings;
MidiConfig midiSettings;
MidiMacroConfig midiMacroConfig;
EncoderConfig encoderConfig;
ClockConfig clockConfig;
SequencerConfig seqConfig;
ColorConfig colorConfig;
ScaleConfig scaleConfig;

// MidiPage midiPageParams;
// SequencerPage seqPageParams;

