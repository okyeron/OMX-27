
#define LED_PIN    14
#define LED_COUNT 27


// VARIABLES
float step_delay;
bool dirtyPixels = false;
bool dirtyDisplay = false;
bool blinkState = false;
bool noteSelect = false;
bool noteSelection = false;
bool patternParams = false;
int selectedNote = 0;
int selectedStep = 0;
bool stepSelect = false;
bool midiAUX = false;
bool enc_edit = false;
int noteon_velocity = 100;
int octave = 0; // default C4 is 0 - range is -4 to +5
int newoctave = octave;

float clockbpm = 120;
float newtempo = clockbpm;
unsigned long tempoStartTime, tempoEndTime;

unsigned long blinkInterval = clockbpm * 2;


// MODES
int mode = 0;
int newmode = 0;
const char* modes[] = {"M","S1","S2"};


// POTS/ANALOG INPUTS					// CCS mapped to Organelle Defaults
int pots[] = {21,22,23,24,7};			// the MIDI CC (continuous controller) for each analog input
int analogPins[] = {23,22,21,20,16};	// teensy pins for analog inputs
int analogValues[] = {0,0,0,0,0};		// default values
int potCC = pots[0];
int potVal = analogValues[0];
//int previous[] = {-1,-1,-1,-1,-1};	// ???


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
byte rowPins[ROWS] = {6, 4, 3, 5, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 8, 10, 9, 15, 17}; //connect to the column pinouts of the keypad

// KEYBOARD NOTE LAYOUT
int notes[] = {0,
     61,63,   66,68,70,   73,75,   78,80,82,
59,60,62,64,65,67,69,71,72,74,76,77,79,81,83,84};

int steps[] = {0,
     1,2,   3,4,5,   6,7,   8,9,10,
11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};


// CV 
word pitchCV;
uint8_t RES;
uint16_t AMAX;
word V_scale;


// COLOR PRESETS
const auto RED = 0xFF0000;
const auto ORANGE = 0xFF8C00;
const auto YELLOW = 0xFFFF00;
const auto GREEN = 0x00FF00;
const auto BLUE = 0x0000FF;
const auto INDIGO = 0x4B0082;
const auto VIOLET = 0xEE82EE;
const auto HALFGREEN = 0x008000;
const auto HALFRED = 0x800000;
const auto DKORANGE = 0x663300;
const auto LBLUE = 0xADD8E6;
const auto DKBLUE = 0x000080;
const auto CYAN = 0x00FFFF;
const auto LTCYAN = 0xE0FFFF;
const auto DKCYAN = 0x008080;
const auto MAGENTA = 0xFF00FF;
const auto DKMAGENTA = 0x330033;
const auto PURPLE = 0x3B0F85;
const auto AMBER = 0x999900;
const auto BEIGE = 0xFFCC33;
const auto HALFWHITE = 0x808080;
const auto LOWWHITE = 0x202020;
const auto LEDOFF = 0x000000;
const uint32_t seqColors[] = {ORANGE,YELLOW,HALFGREEN,MAGENTA,VIOLET,DKCYAN,DKBLUE,PURPLE};

