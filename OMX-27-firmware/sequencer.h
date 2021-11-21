#pragma once

#include <Arduino.h>

#define NUM_PATTERNS 8
#define NUM_STEPS 64
#define NUM_STEPKEYS 16

using Micros = unsigned long;    // for tracking time per pattern

struct TimePerPattern {
  Micros lastProcessTimeP : 32;
  Micros nextStepTimeP : 32;
  Micros lastStepTimeP : 32;
  int lastPosP : 16;
};

enum TrigType {
  TRIGTYPE_MUTE = 0,
  TRIGTYPE_PLAY
};

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

const char *stepTypes[STEPTYPE_COUNT] = {"--", "1", ">>", "<<", "<>", "#?", "?"};
// int stepTypeNumber[STEPTYPE_COUNT] = {STEPTYPE_NONE,STEPTYPE_RESTART,STEPTYPE_FWD,STEPTYPE_REV,STEPTYPE_RANDSTEP,STEPTYPE_RAND};

struct StepNote {           // ?? bytes
  uint8_t note : 7;         // 0 - 127
  // uint8_t unused : 1;    // not hooked up. example of how to sneak a bool into the first byte in the structure
  uint8_t vel : 7;          // 0 - 127
  uint8_t len : 4;          // 0 - 15
  TrigType trig : 1;        // 0 - 1
  int8_t params[5];         // -128 -> 127 // 40 bits
  uint8_t prob : 7;         // 0 - 100
  uint8_t condition : 6;    // 0 - 36
  StepType stepType : 3;    // can be 2 bits as long as StepType has 4 values or fewer
}; // {note, vel, len, TRIG_TYPE, {params0, params1, params2, params3}, prob, cond, STEP_TYPE}


struct Pattern {              // ?? bytes
  uint8_t len : 6;            // 0 - 63, maps to 1 - 64
  uint8_t channel : 4;        // 0 - 15 , maps to channels 1 - 16
  uint8_t startstep : 4;      // step to begin pattern. must be < patternlength-1
  uint8_t autoresetstep : 4;  // step to reset on / 0 = off
  uint8_t autoresetfreq : 4;  // tracking reset iteration if enabled / ie Freq of autoreset. should be renamed
  uint8_t current_cycle : 4;  // tracking current cycle of autoreset counter / start it at 1
  uint8_t rndstep : 4;        // for random autostep functionality
  uint8_t clockDivMultP : 4;
  uint8_t autoresetprob : 7;  // probability of autoreset - 1 is always and totally random if autoreset is 0
  uint8_t swing : 7;
  bool reverse : 1;
  bool mute : 1;
  bool autoreset : 1;        // whether autoreset is enabled
  bool solo : 1;
  bool sendCV : 1;

  StepNote steps[NUM_STEPS]; // note data
};                           // ? bytes

// holds state for sequencer
class SequencerState {

public:
  int ticks;                      // A tick of the clock
  bool clockSource;               // Internal clock (0), external clock (1)
  bool playing;                   // Are we playing?
  bool paused;                    // Are we paused?
  bool stopped;                   // Are we stopped? (Must init to 1)
  byte songPosition;              // A place to store the current MIDI song position
  int playingPattern;             // The currently playing pattern, 0-7
  bool seqResetFlag;              // for autoreset functionality
  int clockDivMult;               // TODO: per pattern setting
  word stepCV;
  int seq_velocity;
  int seq_acc_velocity;

  // TODO: move into Pattern?
  int seqPos[NUM_PATTERNS]; // What position in the sequence are we in?

  int patternDefaultNoteMap[NUM_PATTERNS]; // default to GM Drum Map for now
    int patternPage[NUM_PATTERNS];
  Pattern patterns[NUM_PATTERNS];

  // TODO: move into Pattern?
  TimePerPattern timePerPattern[NUM_PATTERNS];

  Pattern* getPattern(int pattern) {
    return &this->patterns[pattern];
  }

  Pattern* getCurrentPattern() {
    return getPattern(this->playingPattern);
  }

    // Helpers to deal with 1-16 values for pattern length and channel when they're stored as 0-15
  uint8_t getPatternLength(int pattern) {
    return this->patterns[pattern].len + 1;
  }

  void setPatternLength(int pattern, int len) {
    this->patterns[pattern].len = len - 1;
  }

  uint8_t getPatternChannel(int pattern) {
    return this->patterns[pattern].channel + 1;
  }
};

// TODO: this should probably just be a constructor?
SequencerState defaultSequencer() {
  auto nextStepTime = micros();
  auto lastStepTime = micros();

  auto state = SequencerState{
    ticks: 0,
    clockSource: 0,
    playing: 0,
    paused: 0,
    stopped: 1,
    songPosition: 0,
    playingPattern: 0,
    seqResetFlag: 1,
    clockDivMult: 0,
    stepCV: 0,
    seq_velocity: 100,
    seq_acc_velocity: 127,
    seqPos: {0, 0, 0, 0, 0, 0, 0, 0},
    patternDefaultNoteMap: {36, 38, 37, 39, 42, 46, 49, 51},    // default to GM Drum Map for now
    patternPage: {0, 0, 0, 0, 0, 0, 0, 0},
    patterns: {
        {15, 0, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
        {15, 1, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
        {15, 2, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
        {15, 3, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
        {15, 4, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
        {15, 5, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
        {15, 6, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
        {15, 7, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false}},
    timePerPattern: {
        {0, nextStepTime, lastStepTime, 0},
        {0, nextStepTime, lastStepTime, 0},
        {0, nextStepTime, lastStepTime, 0},
        {0, nextStepTime, lastStepTime, 0},
        {0, nextStepTime, lastStepTime, 0},
        {0, nextStepTime, lastStepTime, 0},
        {0, nextStepTime, lastStepTime, 0},
        {0, nextStepTime, lastStepTime, 0}},
    };

  return state;
}

uint8_t lastNote[NUM_PATTERNS][NUM_STEPS] = {
    {0},{0},{0},{0},{0},{0},{0},{0}
};

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

const char* trigConditions[36] = {
    "1:1",
    "1:2","2:2",
    "1:3","2:3","3:3",
    "1:4","2:4","3:4","4:4",
    "1:5","2:5","3:5","4:5","5:5",
    "1:6","2:6","3:6","4:6","5:6","6:6",
    "1:7","2:7","3:7","4:7","5:7","6:7","7:7",
    "1:8","2:8","3:8","4:8","5:8","6:8","7:8","8:8"};
int trigConditionsAB[36][2] = {
    {1,1},
    {1,2}, {2,2},
    {1,3}, {2,3}, {3,3},
    {1,4}, {2,4}, {3,4}, {4,4},
    {1,5}, {2,5}, {3,5}, {4,5}, {5,5},
    {1,6}, {2,6}, {3,6}, {4,6}, {5,6}, {6,6},
    {1,7}, {2,7}, {3,7}, {4,7}, {5,7}, {6,7}, {7,7},
    {1,8}, {2,8}, {3,8}, {4,8}, {5,8}, {6,8}, {7,8}, {8,8}
};
