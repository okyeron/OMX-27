//const int OMX_VERSION = 1.4.0b1;

enum OMXMode
{
     MODE_MIDI = 0,
     MODE_S1,
     MODE_S2,
     MODE_OM,

     NUM_OMX_MODES
};

const OMXMode DEFAULT_MODE = MODE_S2;

// Increment this when data layout in EEPROM changes. May need to write version upgrade readers when this changes.
const uint8_t EEPROM_VERSION = 9;

#define EEPROM_HEADER_ADDRESS	          0
#define EEPROM_HEADER_SIZE		     32
#define EEPROM_PATTERN_ADDRESS 	     32
#define EEPROM_PATTERN_SIZE		     1024      // 8 * 16 * sizeof(StepNote))
#define EEPROM_PATTERN_SETTINGS_ADDRESS 1056
#define EEPROM_PATTERN_SETTINGS_SIZE      56      // 8 * sizeof(PatternSettings)
// next address 1104 (was 1096 before clock)

// DEFINE CC NUMBERS FOR POTS // CCS mapped to Organelle Defaults
const int CC1 = 21;
const int CC2 = 22; 
const int CC3 = 23;
const int CC4 = 24;
const int CC5 = 7;		// change to 25 for EYESY Knob 5

const int CC_AUX = 25; // Mother mode - AUX key
const int CC_OM1 = 26; // Mother mode - enc switch 
const int CC_OM2 = 28; // Mother mode - enc turn

const int LED_BRIGHTNESS = 50;

// DONT CHANGE ANYTHING BELOW HERE

const int LED_PIN  = 14;
const int LED_COUNT = 27;

// POTS/ANALOG INPUTS
// teensy pins for analog inputs 
#if DEV			
	const int analogPins[] = {23,22,21,20,16};	// DEV/beta boards
#elif MIDIONLY
	const int analogPins[] = {23,22,21,20,16};  // on MIDI only boards - {23,A10,21,20,16} on Bodged MIDI boards
#else
	const int analogPins[] = {A10,22,21,20,16}; // on 1.0
#endif

#define NUM_CC_POTS 5
int pots[NUM_CC_POTS] = {CC1,CC2,CC3,CC4,CC5};			// the MIDI CC (continuous controller) for each analog input

const int gridh = 32;
const int gridw = 128;
const int PPQ = 96;

const char* modes[] = {"MI","S1","S2","OM"};
const char* infoDialogText[] = {"COPIED","PASTED","CLEARED","RESET","FWD >>","<< REV","SAVED","SAVE?"};

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

float multValues[] = {.25, .5, 1, 2, 4, 8, 16};
const char* mdivs[] = {"1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "W"};

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
InfoDialogs infoDialog[NUM_DIALOGS] = {
  {"COPIED", false},
  {"PASTED", false},
  {"CLEARED", false},
  {"RESET", false},
  {"FWD >>", false},
  {"<< REV", false},
  {"SAVED", false},
  {"SAVE?", false}
};

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

	SUBMODES_COUNT
};

// KEY SWITCH ROWS/COLS
const byte ROWS = 5; //five rows
const byte COLS = 6; //six columns

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