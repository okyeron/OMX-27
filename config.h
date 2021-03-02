
const int DEFAULT_MODE = 0;

// DEFINE CC NUMBERS FOR POTS // CCS mapped to Organelle Defaults
const int CC1 = 21;
const int CC2 = 22; 
const int CC3 = 23;
const int CC4 = 24;
const int CC5 = 7;

const int CC_AUX = 25; // Mother mode - AUX key
const int CC_OM1 = 26; // Mother mode - enc switch 
const int CC_OM2 = 28; // Mother mode - enc turn

// DONT CHANGE ANYTHING BELOW HERE

const int LED_PIN  = 14;
const int LED_COUNT = 27;

// POTS/ANALOG INPUTS					
const int analogPins[] = {23,22,21,20,16};	// teensy pins for analog inputs 
						// {23,22,21,20,16} on beta
						// {23,A10,21,20,16} on test
						// {A10,22,21,20,16} on 1.0

const int pots[] = {CC1,CC2,CC3,CC4,CC5};			// the MIDI CC (continuous controller) for each analog input

const int gridh = 32;
const int gridw = 128;
const int PPQ = 24;

const char* modes[] = {"MI","S1","S2","OM"};

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
const int notes[] = {0,
     61,63,   66,68,70,   73,75,   78,80,82,
59,60,62,64,65,67,69,71,72,74,76,77,79,81,83,84};

const int steps[] = {0,
     1,2,   3,4,5,   6,7,   8,9,10,
11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};
