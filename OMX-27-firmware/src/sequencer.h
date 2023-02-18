#pragma once

#include <Arduino.h>
#include "config.h"

#define NUM_SEQ_PATTERNS_EEPROM 6
#define NUM_SEQ_PATTERNS 8
#define NUM_STEPS 64
#define NUM_STEPKEYS 16

const uint8_t defaultNoteLength = 3; // index from kNoteLengths[] = {0.10, 0.25, 0.5, 0.75, 1, 1.5, 2, 4, 8, 16};
// see config.cpp 

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

extern const char *stepTypes[STEPTYPE_COUNT];
// int stepTypeNumber[STEPTYPE_COUNT] = {STEPTYPE_NONE,STEPTYPE_RESTART,STEPTYPE_FWD,STEPTYPE_REV,STEPTYPE_RANDSTEP,STEPTYPE_RAND};

struct StepNote {           // ?? bytes
	uint8_t note : 7;         // 0 - 127
	// uint8_t unused : 1;    // not hooked up. example of how to sneak a bool into the first byte in the structure
	uint8_t vel : 7;          // 0 - 127
	uint8_t len : 4;          // Plugs into kNoteLengths
	TrigType trig : 1;        // 0 - 1
	int8_t params[5];         // -128 -> 127 // 40 bits
	uint8_t prob : 7;         // 0 - 100
	uint8_t condition : 6;    // 0 - 36
	StepType stepType : 3;    // can be 2 bits as long as StepType has 4 values or fewer

	void CopyFrom(StepNote* other)
	{
		note = other->note;
		vel = other->vel;
		len = other->len;
		trig = other->trig;

		for(uint8_t i = 0; i < 5; i++)
		{
			params[i] = other->params[i];
		}
		prob = other->prob;
		condition = other->condition;
		stepType = other->stepType;
	}
}; // {note, vel, len, TRIG_TYPE, {params0, params1, params2, params3}, prob, cond, STEP_TYPE}


struct Pattern {              // ?? bytes
	uint8_t len : 6;            // 0 - 63, maps to 1 - 64
	uint8_t channel : 4;        // 0 - 15 , maps to channels 1 - 16
	uint8_t startstep : 6;      // step to begin pattern. must be < patternlength-1
	uint8_t autoresetstep : 6;  // step to reset on / 0 = off
	uint8_t autoresetfreq : 6;  // tracking reset iteration if enabled / ie Freq of autoreset. should be renamed
	uint8_t current_cycle : 6;  // tracking current cycle of autoreset counter / start it at 1
	uint8_t rndstep : 6;        // for random autostep functionality
	uint8_t clockDivMultP : 4;
	uint8_t autoresetprob : 7;  // probability of autoreset - 1 is always and totally random if autoreset is 0
	uint8_t swing : 7;
	bool reverse : 1;
	bool mute : 1;
	bool autoreset : 1;        // whether autoreset is enabled
	bool solo : 1;
	bool sendCV : 1;

	// this has to stay as the last property to ensure save/load works correctly
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
	int lastSeqPos[NUM_SEQ_PATTERNS]; // What position in the sequence are we in? ZERO BASED
	int seqPos[NUM_SEQ_PATTERNS]; // What position in the sequence are we in? ZERO BASED

	int patternDefaultNoteMap[NUM_SEQ_PATTERNS]; // default to GM Drum Map for now
		int patternPage[NUM_SEQ_PATTERNS];
	Pattern patterns[NUM_SEQ_PATTERNS];

	// TODO: move into Pattern?
	TimePerPattern timePerPattern[NUM_SEQ_PATTERNS];

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

extern uint8_t lastNote[NUM_SEQ_PATTERNS][NUM_STEPS];
extern const char* trigConditions[36];

// forward declarations
SequencerState defaultSequencer();
int serializedPatternSize(bool eeprom);

StepNote* getSelectedStep();
void doStepS1();
void doStepS2();

void transposeSeq(int patternNum, int amt);
int getPatternPage(int position);
void rotatePattern(int patternNum, int rot);

void step_ahead();
void step_back();
void auto_reset(int p);
bool probResult(int probSetting);

void playNote(int patternNum);
void seqNoteOn(int notenum, int velocity, int patternNum);
void seqNoteOff(int notenum, int patternNum);
void allNotesOff();
void allNotesOffPanic(); // TODO us this used?

void seqStart();
void seqStop();
void seqContinue();
void seqReset();

void changeStepType(int amount);
void resetPatternDefaults(int patternNum);

void copyPattern(int patternNum);
void pastePattern(int patternNum);
void clearPattern(int patternNum);

// global sequencer shared state
extern SequencerState sequencer;