#define NUM_PATTERNS 8
#define NUM_STEPS 16

// the MIDI channel number to send messages
int midiChannel = 1;

int ticks = 0;            // A tick of the clock
bool clockSource = 0;     // Internal clock (0), external clock (1)
bool playing = 0;         // Are we playing?
bool paused = 0;          // Are we paused?
bool stopped = 1;         // Are we stopped? (Must init to 1)
byte songPosition = 0;    // A place to store the current MIDI song position
int playingPattern = 0;  // The currently playing pattern, 0-7
bool seqResetFlag = 1;

word stepCV;
int seq_velocity = 100;
int seq_acc_velocity = 127;

int seqPos[NUM_PATTERNS] = {0, 0, 0, 0, 0, 0, 0, 0};				// What position in the sequence are we in?
bool patternMute[NUM_PATTERNS] = {false, false, false, false, false, false, false, false};     
uint8_t patternLength[NUM_PATTERNS];    // loaded from EEPROM or manually initialized

int patternStart[NUM_PATTERNS] = {0, 0, 0, 0, 0, 0, 0, 0};
int patternDirection[NUM_PATTERNS] = {0, 0, 0, 0, 0, 0, 0, 0}; // 0 = forward, 1 = reverse

int patternChannel[NUM_PATTERNS] = {1, 2, 3, 4, 5, 6, 7, 8};

int patternDefaultNoteMap[NUM_PATTERNS] = {36, 38, 37, 39, 42, 46, 49, 51}; // default to GM Drum Map for now

enum StepType {
  STEPTYPE_MUTE = 0,
  STEPTYPE_PLAY,
  STEPTYPE_ACCENT,
  STEPTYPE_RESTART
};

struct StepNote {           // 8 bytes
  uint8_t note : 7;        // 0 - 127
  // uint8_t unused : 1;       // not hooked up. example of how to sneak a bool into the first byte in the structure

  uint8_t vel : 7;         // 0 - 127
  // uint8_t unused : 1;

  uint8_t len : 4;         // 0 - 15
  StepType stepType : 2;    // can be 2 bits as long as StepType has 4 values or fewer
  // uint8_t unused : 2;

  int8_t params[5];       // -128 -> 127
};

// default to GM Drum Map for now
StepNote stepNoteP[NUM_PATTERNS][NUM_STEPS];

uint8_t lastNote[NUM_PATTERNS][NUM_STEPS] = {
  {0},
  {0},
  {0},
  {0},
  {0},
  {0},
  {0},
  {0}
};

StepNote copyPatternBuffer[NUM_STEPS] = { 
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 1, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} } };



