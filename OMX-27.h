// OMX-27 config

#define CVGATE_PIN	13		// 13 on beta boards, A10 on v1
#define CVPITCH_PIN	A14
#define LED_PIN    14
#define LED_COUNT 27

#define DEFAULT_MODE 1

#define gridh 32
#define gridw 128

#define PPQ 24

#define FONT_LABELS u8g2_font_5x8_tf
#define FONT_VALUES u8g2_font_7x14B_tf
#define FONT_SYMB u8g2_font_9x15_m_symbols
#define FONT_SYMB_BIG u8g2_font_cu12_h_symbols
#define FONT_TENFAT u8g2_font_tenfatguys_tf
#define FONT_BIG u8g2_font_helvB18_tr

// DEFINE CCS FOR POTS
const int CC1 = 21;
const int CC2 = 22;
const int CC3 = 23;
const int CC4 = 24;
const int CC5 = 7;

const int CC_AUX = 25; // Mother mode - AUX key
const int CC_OM1 = 26; // Mother mode - enc switch 
const int CC_OM2 = 28; // Mother mode - enc turn


// POTS/ANALOG INPUTS					// CCS mapped to Organelle Defaults
int pots[] = {CC1,CC2,CC3,CC4,CC5};			// the MIDI CC (continuous controller) for each analog input
int potValues[] = {0,0,0,0,0};			
int analogPins[] = {23,22,21,20,16};	// teensy pins for analog inputs
int analogValues[] = {0,0,0,0,0};		// default values
int potCC = pots[0];
int potVal = analogValues[0];
int potNum = 0;
bool plockDirty[] = {false,false,false,false,false};
int prevPlock[] = {0,0,0,0,0};

// MODES
const char* modes[] = {"MI","S1","S2","OM"};
int mode = DEFAULT_MODE;
int newmode = DEFAULT_MODE;
#define numModes (sizeof(modes)/sizeof(char *)) //array size  
int nsmode = 6;
int ptmode = 3;

// VARIABLES
float step_delay;
bool dirtyPixels = false;
bool dirtyDisplay = false;
bool blinkState = false;
bool noteSelect = false;
bool noteSelection = false;
bool patternParams = false;
int noteSelectPage = 0;
int selectedNote = 0;
int selectedStep = 0;
bool stepSelect = false;
bool midiAUX = false;
bool enc_edit = false;
int noteon_velocity = 100;
int octave = 0; // default C4 is 0 - range is -4 to +5
int newoctave = octave;
int rotationAmt = 0;
int hline = 8;

float clockbpm = 120;
float newtempo = clockbpm;
unsigned long tempoStartTime, tempoEndTime;

unsigned long blinkInterval = clockbpm * 2;
unsigned long longPressInterval = 1500;

bool keyState[27] = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};

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
// https://www.rapidtables.com/web/color/color-wheel.html
// hsl(xxx, 100%, 50%)
const auto RED = 0xFF0000;
const auto ORANGE = 0xFF8000;
const auto YELLOW = 0xFFFF00;
const auto LIME = 0x80FF00;
const auto GREEN = 0x00FF00;
const auto MINT = 0x00FF80;
const auto CYAN = 0x00FFFF;
const auto RBLUE = 0x007FFF;
const auto BLUE = 0x0000FF;
const auto PURPLE = 0x7F00FF;
const auto MAGENTA = 0xFF00FF;
const auto ROSE = 0xFF0080;

// hsl(xxx, 50%, 50%)
const auto MEDRED = 0xBF4040;
const auto MEDBLUE = 0x4040BF;
const auto MEDYELLOW = 0xBFBF40;

// hsl(xxx, 100%, 75%)
const auto LTCYAN = 0x80FFFF;
const auto LTPURPLE = 0xBF80FF;
const auto SALMON = 0xFF8080;
const auto PINK = 0xFF80D4;
const auto LTYELLOW = 0xFFFF80;

// hsl(xxx, 100%, 15%)
const auto DKRED = 0x800000;
const auto DKORANGE = 0x4D2600;
const auto DKYELLOW = 0x4C4D00;
const auto DKLIME = 0x408000;
const auto DKGREEN = 0x264D00;
const auto DKCYAN = 0x004C4D;
const auto DKBLUE = 0x00004D;
const auto DKPURPLE = 0x26004D;
const auto DKMAGENTA = 0x4D004C;
const auto INDIGO = 0x4B0082;

// hsl(xxx, 50%, 75%)
const auto LBLUE = 0x9FCFDF;
const auto VIOLET = 0xDF9FDF;

// hsl(xxx, 25%, 50%)
const auto DIMORANGE = 0x9F8060;
const auto DIMYELLOW = 0x9F9F60;
const auto DIMLIME = 0x809F60;
const auto DIMGREEN = 0x609F60;
const auto DIMMAGENTA = 0x9F609F;
const auto DIMCYAN = 0x609F9F;
const auto DIMBLUE = 0x60609F;
const auto DIMPURPLE = 0x7F609F;


// other
const auto AMBER = 0x999900;
const auto BEIGE = 0xFFCC33;

// no color
const auto HALFWHITE = 0x808080;
const auto LOWWHITE = 0x202020;
const auto LEDOFF = 0x000000;

uint32_t stepColor = 0x000000;

// sequencer pattern colors
const uint32_t seqColors[] = {ORANGE,YELLOW,GREEN,MAGENTA,CYAN,BLUE,LIME,LTPURPLE};
const uint32_t muteColors[] = {DKORANGE,DKYELLOW,DKGREEN,DKMAGENTA,DKCYAN,DKBLUE,DKLIME,DKPURPLE};


#define MIDINOTEON HALFWHITE
#define SEQCHASE DKRED 
#define SEQMARKER LOWWHITE 
#define SEQSTEP ORANGE 

#define NOTESEL DKCYAN 
#define PATTSEL LIME 

#define FUNKONE LTCYAN 
#define FUNKTWO MINT 
