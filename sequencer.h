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
bool seqResetFlag = 1;    // for autoreset functionality
using Micros = unsigned long; // for tracking time per pattern
int clockDivMult = 0;  // TODO: per pattern setting

word stepCV;
int seq_velocity = 100;
int seq_acc_velocity = 127;

int seqPos[NUM_PATTERNS] = {0, 0, 0, 0, 0, 0, 0, 0};				// What position in the sequence are we in?

// int patternStart[NUM_PATTERNS] = {0, 0, 0, 0, 0, 0, 0, 0};

int patternDefaultNoteMap[NUM_PATTERNS] = {36, 38, 37, 39, 42, 46, 49, 51}; // default to GM Drum Map for now

enum StepType {
  STEPTYPE_MUTE = 0,
  STEPTYPE_PLAY,
  STEPTYPE_ACCENT,
  STEPTYPE_RESTART
};

struct PatternSettings {  // 5 bytes
  uint8_t len : 4;    // 0 - 15, maps to 1 - 16
  uint8_t channel : 4;    // 0 - 15 , maps to channels 1 - 16
  uint8_t startstep : 4; // step to begin pattern. must be < patternlength-1
  uint8_t autoresetstep : 4;  // step to reset on / 0 = off
  uint8_t autoresetfreq : 4; // tracking reset iteration if enabled / ie Freq of autoreset. should be renamed
  uint8_t autoresetprob : 4; // probability of autoreset - 1 is always and totally random if autoreset is 0
  uint8_t current_cycle : 4; // tracking current cycle of autoreset counter / start it at 1
  uint8_t rndstep : 4; // for random autostep functionality
  bool reverse : 1;
  bool mute : 1;
  bool autoreset : 1; // whether autoreset is enabled
};

PatternSettings patternSettings[NUM_PATTERNS] = { 
  { 15, 0, 0, 0, 0, 0, 1, 3, false, false, false },
  { 15, 1, 0, 0, 0, 0, 1, 3, false, false, false },
  { 15, 2, 0, 0, 0, 0, 1, 3, false, false, false },
  { 15, 3, 0, 0, 0, 0, 1, 3, false, false, false },
  { 15, 4, 0, 0, 0, 0, 1, 3, false, false, false },
  { 15, 5, 0, 0, 0, 0, 1, 3, false, false, false },
  { 15, 6, 0, 0, 0, 0, 1, 3, false, false, false },
  { 15, 7, 0, 0, 0, 0, 1, 3, false, false, false }
};

struct TimePerPattern {
  Micros lastProcessTimeP : 32;
  Micros nextStepTimeP : 32;
  Micros lastStepTimeP : 32;
};

TimePerPattern timePerPattern[NUM_PATTERNS] = {
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 }
};

// Helpers to deal with 1-16 values for pattern length and channel when they're stored as 0-15
uint8_t PatternLength( int pattern ) {
  return patternSettings[pattern].len + 1;
}

void SetPatternLength( int pattern, int len ) {
  patternSettings[pattern].len = len - 1;
}

uint8_t PatternChannel( int pattern ) {
  return patternSettings[pattern].channel + 1;
}

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

uint8_t midiLastNote = 0;

StepNote copyPatternBuffer[NUM_STEPS] = { 
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} },
  {0, 0, 0, STEPTYPE_MUTE, { -1, -1, -1, -1, -1} } };
