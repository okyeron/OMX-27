#define NUM_PATTERNS 8
#define NUM_STEPS 32
#define NUM_STEPKEYS 16

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
bool cvPattern[NUM_PATTERNS] = {1, 0, 0, 0, 0, 0, 0, 0};

// int patternStart[NUM_PATTERNS] = {0, 0, 0, 0, 0, 0, 0, 0};

int patternDefaultNoteMap[NUM_PATTERNS] = {36, 38, 37, 39, 42, 46, 49, 51}; // default to GM Drum Map for now

enum StepType {
  STEPTYPE_NONE = 0,
  STEPTYPE_RESTART,
  STEPTYPE_FWD,
  STEPTYPE_REV,
  STEPTYPE_PONG,
  STEPTYPE_RANDSTEP,
  STEPTYPE_RAND,

  STEPTYPE_COUNT
};
const char* stepTypes[STEPTYPE_COUNT] = {"--", "1", ">>", "<<", "<>", "#?", "?"};
// int stepTypeNumber[STEPTYPE_COUNT] = {STEPTYPE_NONE,STEPTYPE_RESTART,STEPTYPE_FWD,STEPTYPE_REV,STEPTYPE_RANDSTEP,STEPTYPE_RAND};

enum TrigType {
  TRIGTYPE_MUTE = 0,
  TRIGTYPE_PLAY
};

struct PatternSettings {  // ?? bytes
  uint8_t len : 6;    // 0 - ?, maps to 1 to (NUM_STEPS - 1)
  uint8_t channel : 4;    // 0 - 15 , maps to channels 1 - 16
  uint8_t startstep : 4; // step to begin pattern. must be < patternlength-1
  uint8_t autoresetstep : 4;  // step to reset on / 0 = off
  uint8_t autoresetfreq : 4; // tracking reset iteration if enabled / ie Freq of autoreset. should be renamed
  uint8_t current_cycle : 4; // tracking current cycle of autoreset counter / start it at 1
  uint8_t rndstep : 4; // for random autostep functionality
  uint8_t clockDivMultP : 4;
  uint8_t autoresetprob : 7; // probability of autoreset - 1 is always and totally random if autoreset is 0
  uint8_t swing : 7;
  bool reverse : 1;
  bool mute : 1;
  bool autoreset : 1; // whether autoreset is enabled
  bool solo : 1;
}; // ? bytes

PatternSettings patternSettings[NUM_PATTERNS] = { 
  { 15, 0, 0, 0, 0, 0, 1, 3, 1, 0, false, false, false, false },
  { 15, 1, 0, 0, 0, 0, 1, 3, 1, 0, false, false, false, false },
  { 15, 2, 0, 0, 0, 0, 1, 3, 1, 0, false, false, false, false },
  { 15, 3, 0, 0, 0, 0, 1, 3, 1, 0, false, false, false, false },
  { 15, 4, 0, 0, 0, 0, 1, 3, 1, 0, false, false, false, false },
  { 15, 5, 0, 0, 0, 0, 1, 3, 1, 0, false, false, false, false },
  { 15, 6, 0, 0, 0, 0, 1, 3, 1, 0, false, false, false, false },
  { 15, 7, 0, 0, 0, 0, 1, 3, 1, 0, false, false, false, false }
};

struct TimePerPattern {
  Micros lastProcessTimeP : 32;
  Micros nextStepTimeP : 32;
  Micros lastStepTimeP : 32;
  int lastPosP : 16;
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

struct StepNote {           // ?? bytes
  uint8_t note : 7;        // 0 - 127
  // uint8_t unused : 1;       // not hooked up. example of how to sneak a bool into the first byte in the structure
  uint8_t vel : 7;			// 0 - 127
  uint8_t len : 4;			// 0 - 15
  TrigType trig : 1;	// 0 - 1
  int8_t params[5];			// -128 -> 127 // 40 bits    
  uint8_t prob : 7;			// 0 - 100
  uint8_t condition : 6;			// 0 - 36
  StepType stepType : 3;	// can be 2 bits as long as StepType has 4 values or fewer
}; // {note, vel, len, TRIG_TYPE, {params0, params1, params2, params3}, prob, cond, STEP_TYPE}

// default to GM Drum Map for now
StepNote stepNoteP[NUM_PATTERNS][NUM_STEPS];

uint8_t lastNote[NUM_PATTERNS][NUM_STEPS] = {
	{0},{0},{0},{0},{0},{0},{0},{0}
};

uint8_t midiLastNote = 0;

StepNote copyPatternBuffer[NUM_STEPS] = { 
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE },
  {0, 0, 0, TRIGTYPE_MUTE, { -1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE } 
};

int loopCount[NUM_PATTERNS][NUM_STEPS] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const char* trigConditions[36] = {"1:1","1:2","2:2","1:3","2:3","3:3","1:4","2:4","3:4","4:4","1:5","2:5","3:5","4:5","5:5","1:6","2:6","3:6","4:6","5:6","6:6","1:7","2:7","3:7","4:7","5:7","6:7","7:7","1:8","2:8","3:8","4:8","5:8","6:8","7:8","8:8"};
int ABcondition = 0;
int trigConditionsAB[36][2] ={
	{1,1}, 
    {1,2}, {2,2},
    {1,3}, {2,3}, {3,3},
    {1,4}, {2,4}, {3,4}, {4,4},
    {1,5}, {2,5}, {3,5}, {4,5}, {5,5},
    {1,6}, {2,6}, {3,6}, {4,6}, {5,6}, {6,6},
    {1,7}, {2,7}, {3,7}, {4,7}, {5,7}, {6,7}, {7,7},
    {1,8}, {2,8}, {3,8}, {4,8}, {5,8}, {6,8}, {7,8}, {8,8}
};
