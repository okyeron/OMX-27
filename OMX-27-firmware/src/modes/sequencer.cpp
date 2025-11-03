#include <Adafruit_NeoPixel.h>

#include "sequencer.h"
#include "../globals.h"
#include "../config.h"
#include "../consts/consts.h"
#include "../consts/colors.h"
#include "../midi/midi.h"
#include "../midi/noteoffs.h"
#include "../hardware/omx_disp.h"
#include "../hardware/omx_leds.h"
#include "../utils/omx_util.h"
#include "../utils/cvNote_util.h"

// globals in main ino
extern SequencerState sequencer;
// extern SysSettings sysSettings;

// extern int midiChannel;
// extern int omxSeqselectedStep;

// extern Adafruit_NeoPixel strip;

// extern volatile unsigned long omxseqstep_micros;
// extern volatile unsigned long seqConfig.noteon_micros;
// extern volatile unsigned long noteoff_micros;
// extern volatile unsigned long ppqInterval;

// extern int octave;			// default C4 is 0 - range is -4 to +5
// extern int midiKeyState[27];
// extern bool dirtyPixels;
// extern bool dirtyDisplay;
// extern PendingNoteOffs pendingNoteOffs; // in noteoffs.h
// extern int potbank;
// extern int potValues[];
// extern int prevPlock[];
// extern int defaultVelocity;

// funcs in main ino
// extern void show_current_step(int patternNum);

// extern StepNote* getSelectedStep();

// globals from sequencer.h
uint8_t lastNote[NUM_SEQ_PATTERNS][NUM_STEPS] = {
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};

StepNote copyPatternBuffer[NUM_STEPS] = {
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE},
	{0, 0, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE}};

int loopCount[NUM_SEQ_PATTERNS][NUM_STEPS] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

const char *trigConditions[36] = {
	"1:1",
	"1:2", "2:2",
	"1:3", "2:3", "3:3",
	"1:4", "2:4", "3:4", "4:4",
	"1:5", "2:5", "3:5", "4:5", "5:5",
	"1:6", "2:6", "3:6", "4:6", "5:6", "6:6",
	"1:7", "2:7", "3:7", "4:7", "5:7", "6:7", "7:7",
	"1:8", "2:8", "3:8", "4:8", "5:8", "6:8", "7:8", "8:8"};

int trigConditionsAB[36][2] = {
	{1, 1},
	{1, 2},
	{2, 2},
	{1, 3},
	{2, 3},
	{3, 3},
	{1, 4},
	{2, 4},
	{3, 4},
	{4, 4},
	{1, 5},
	{2, 5},
	{3, 5},
	{4, 5},
	{5, 5},
	{1, 6},
	{2, 6},
	{3, 6},
	{4, 6},
	{5, 6},
	{6, 6},
	{1, 7},
	{2, 7},
	{3, 7},
	{4, 7},
	{5, 7},
	{6, 7},
	{7, 7},
	{1, 8},
	{2, 8},
	{3, 8},
	{4, 8},
	{5, 8},
	{6, 8},
	{7, 8},
	{8, 8}};

const char *stepTypes[STEPTYPE_COUNT] = {"--", "1", ">>", "<<", "<>", "#?", "?"};

// definitions

SequencerState defaultSequencer()
{
	auto nextStepTime = micros();
	auto lastStepTime = micros();

	auto state = SequencerState{
		ticks : 0,
		clockSource : 0,
		playing : 0,
		paused : 0,
		stopped : 1,
		songPosition : 0,
		playingPattern : 0,
		seqResetFlag : 1,
		clockDivMult : 0,
		stepCV : 0,
		seq_velocity : 100,
		seq_acc_velocity : 127,
		lastSeqPos : {0, 0, 0, 0, 0, 0, 0, 0},					  // ZERO BASED
		seqPos : {0, 0, 0, 0, 0, 0, 0, 0},						  // ZERO BASED
		patternDefaultNoteMap : {36, 38, 37, 39, 42, 46, 49, 51}, // default to GM Drum Map for now
		patternPage : {0, 0, 0, 0, 0, 0, 0, 0},
		patterns : {
			{15, 0, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
			{15, 1, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
			{15, 2, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
			{15, 3, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
			{15, 4, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
			{15, 5, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
			{15, 6, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false},
			{15, 7, 0, 0, 0, 0, 1, 2, 1, 0, false, false, false, false, false}},
		timePerPattern : {
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

int serializedPatternSize(bool eeprom)
{
	int total = sizeof(Pattern);
	int singleStep = sizeof(StepNote);
	int totalWithoutSteps = total - (singleStep * NUM_STEPS);
	int numSteps = NUM_STEPS;

	// 1292
	// 1280
	// totalWithoutSteps = 12
	// EPROM Size = 320 + 12 = 332

	// for EEPROM we only serialize 16 steps
	if (eeprom)
	{
		numSteps = 16;
	}
	int stepSize = numSteps * singleStep;

	return totalWithoutSteps + stepSize;
}

StepNote *getSelectedStep()
{
	return &sequencer.getCurrentPattern()->steps[seqConfig.selectedStep];
}

void step_ahead()
{
	// step ALL patterns ahead one place
	for (int j = 0; j < 8; j++)
	{
		sequencer.lastSeqPos[j] = sequencer.seqPos[j];

		sequencer.seqPos[j]++;
		if (sequencer.seqPos[j] >= sequencer.getPatternLength(j))
			sequencer.seqPos[j] = 0;

		sequencer.patternPage[j] = getPatternPage(sequencer.seqPos[j]);

		//		if (sequencer.getPattern(j)->reverse) {
		//			sequencer.seqPos[j]--;
		////			auto_reset(j); // determine whether to reset or not based on param settings
		//			if (sequencer.seqPos[j] < 0)
		//				sequencer.seqPos[j] = sequencer.getPatternLength(j)-1;
		//		} else {
		//			sequencer.seqPos[j]++;
		////			auto_reset(j); // determine whether to reset or not based on param settings
		//			if (sequencer.seqPos[j] >= sequencer.getPatternLength(j))
		//				sequencer.seqPos[j] = 0;
		//		}
	}
}
void step_back()
{
	// step each pattern ahead one place
	for (int j = 0; j < 8; j++)
	{
		sequencer.lastSeqPos[j] = sequencer.seqPos[j];

		sequencer.seqPos[j]--;
		if (sequencer.seqPos[j] < 0)
			sequencer.seqPos[j] = sequencer.getPatternLength(j) - 1;

		sequencer.patternPage[j] = getPatternPage(sequencer.seqPos[j]);

		//			if (sequencer.getPattern(j)->reverse) {
		//				sequencer.seqPos[j]++;
		//	//			auto_reset(j); // determine whether to reset or not based on param settings
		//				if (sequencer.seqPos[j] >= sequencer.getPatternLength(j))
		//					sequencer.seqPos[j] = 0;
		//			} else {
		//				sequencer.seqPos[j]--;
		//	// 			auto_reset(j);
		//				if (sequencer.seqPos[j] < 0)
		//					sequencer.seqPos[j] = sequencer.getPatternLength(j) - 1;
		//			}
	}
}

void new_step_ahead(int patternNum)
{
	sequencer.lastSeqPos[patternNum] = sequencer.seqPos[patternNum];

	// step ONE pattern ahead one place
	if (sequencer.getPattern(patternNum)->reverse)
	{
		sequencer.seqPos[patternNum]--;
		auto_reset(patternNum); // determine whether to reset or not based on param settings
	}
	else
	{
		sequencer.seqPos[patternNum]++;
		auto_reset(patternNum); // determine whether to reset or not based on param settings
	}
}

void auto_reset(int p)
{
	auto pattern = sequencer.getPattern(p);

	// should be conditioned on whether we're in S2!!
	if (sequencer.seqPos[p] >= sequencer.getPatternLength(p) ||
		(pattern->autoreset && (pattern->autoresetstep > (pattern->startstep)) && (sequencer.seqPos[p] >= pattern->autoresetstep)) ||
		(pattern->autoreset && (pattern->autoresetstep == 0) && (sequencer.seqPos[p] >= pattern->rndstep)) ||
		(pattern->reverse && (sequencer.seqPos[p] < 0)) ||									   // normal reverse reset
		(pattern->reverse && pattern->autoreset && (sequencer.seqPos[p] < pattern->startstep)) // ||
		//(settings->reverse && settings->autoreset && (settings->autoresetstep == 0 ) && (seqPos[p] < settings->rndstep))
	)
	{

		if (pattern->reverse)
		{
			if (pattern->autoreset)
			{
				if (pattern->autoresetstep == 0)
				{
					sequencer.seqPos[p] = pattern->rndstep - 1;
				}
				else
				{
					sequencer.seqPos[p] = pattern->autoresetstep - 1; // resets pattern in REV
				}
			}
			else
			{
				sequencer.seqPos[p] = (sequencer.getPatternLength(p) - pattern->startstep) - 1;
			}
		}
		else
		{
			sequencer.seqPos[p] = (pattern->startstep); // resets pattern in FWD
		}
		if (pattern->autoresetfreq == pattern->current_cycle)
		{ // reset cycle logic
			if (probResult(pattern->autoresetprob))
			{
				// chance of doing autoreset
				pattern->autoreset = true;
			}
			else
			{
				pattern->autoreset = false;
			}
			pattern->current_cycle = 1; // reset cycle to start new iteration
		}
		else
		{
			pattern->autoreset = false;
			pattern->current_cycle++; // advance to next cycle
		}
		pattern->rndstep = (rand() % sequencer.getPatternLength(p)) + 1; // randomly choose step for next cycle
	}
	sequencer.patternPage[p] = getPatternPage(sequencer.seqPos[p]); // FOLLOW MODE FOR SEQ PAGE

	// return ()
}

bool probResult(int probSetting)
{
	//	int tempProb = (rand() % probSetting);
	if (probSetting == 0)
	{
		return false;
	}
	if ((rand() % 100) < probSetting)
	{ // assumes probSetting is a range 0-100
		return true;
	}
	else
	{
		return false;
	}
}

bool evaluate_AB(int condition, int patternNum)
{
	bool shouldTrigger = false;
	;

	loopCount[patternNum][sequencer.seqPos[patternNum]]++;

	int a = trigConditionsAB[condition][0];
	int b = trigConditionsAB[condition][1];

	// Serial.print (patternNum);
	// Serial.print ("/");
	// Serial.print (seqPos[patternNum]);
	// Serial.print (" ");
	// Serial.print (loopCount[patternNum][seqPos[patternNum]]);
	// Serial.print (" ");
	// Serial.print (a);
	// Serial.print (":");
	// Serial.print (b);
	// Serial.print (" ");

	if (loopCount[patternNum][sequencer.seqPos[patternNum]] == a)
	{
		shouldTrigger = true;
	}
	else
	{
		shouldTrigger = false;
	}
	if (loopCount[patternNum][sequencer.seqPos[patternNum]] >= b)
	{
		loopCount[patternNum][sequencer.seqPos[patternNum]] = 0;
	}
	return shouldTrigger;
}

void changeStepType(int amount)
{
	auto tempType = getSelectedStep()->stepType + amount;

	// this is fucking hacky to increment the enum for stepType
	switch (tempType)
	{
	case 0:
		getSelectedStep()->stepType = STEPTYPE_NONE;
		break;
	case 1:
		getSelectedStep()->stepType = STEPTYPE_RESTART;
		break;
	case 2:
		getSelectedStep()->stepType = STEPTYPE_FWD;
		break;
	case 3:
		getSelectedStep()->stepType = STEPTYPE_REV;
		break;
	case 4:
		getSelectedStep()->stepType = STEPTYPE_PONG;
		break;
	case 5:
		getSelectedStep()->stepType = STEPTYPE_RANDSTEP;
		break;
	case 6:
		getSelectedStep()->stepType = STEPTYPE_RAND;
		break;
	default:
		break;
	}
}

void step_on(int patternNum)
{
	//	Serial.print(patternNum);
	//	Serial.println(" step on");
	//	playNote(playingPattern);
}

void step_off(int patternNum, int position)
{
	lastNote[patternNum][position] = 0;

	//	Serial.print(seqPos[patternNum]);
	//	Serial.println(" step off");
	//	analogWrite(CVPITCH_PIN, 0);
	//	digitalWrite(CVGATE_PIN, LOW);
}

void doStepS1()
{
	// // probability test
	bool testProb = probResult(sequencer.getCurrentPattern()->steps[sequencer.seqPos[sequencer.playingPattern]].prob);

	if (sequencer.playing)
	{
		unsigned long playstepmicros = micros();

		for (int j = 0; j < NUM_SEQ_PATTERNS; j++)
		{ // check all patterns for notes to play in time
			// CLOCK PER PATTERN BASED APPROACH
			auto pattern = sequencer.getPattern(j);

			// TODO: refactor timePerPattern stuff into sequencer.h

			if (playstepmicros >= sequencer.timePerPattern[j].nextStepTimeP)
			{

				seqReset(); // check for seqReset
				sequencer.timePerPattern[j].lastStepTimeP = sequencer.timePerPattern[j].nextStepTimeP;
				sequencer.timePerPattern[j].nextStepTimeP += (clockConfig.step_micros) * (multValues[sequencer.getPattern(j)->clockDivMultP]); // calc step based on rate

				sequencer.timePerPattern[j].lastPosP = (sequencer.seqPos[j] + 15) % 16;
				if (lastNote[j][sequencer.timePerPattern[j].lastPosP] > 0)
				{
					step_off(j, sequencer.timePerPattern[j].lastPosP);
				}
				if (testProb)
				{
					if (evaluate_AB(pattern->steps[sequencer.seqPos[j]].condition, j))
					{
						if (j == sequencer.playingPattern)
						{
							playNote(j);
						}
					}
				}
				// No need to have this function call in here.
				// Can put into omx_mode_sequencer and remove extern function
				// if (j == sequencer.playingPattern)
				// { // only show selected pattern
				// 	show_current_step(sequencer.playingPattern);
				// }
				new_step_ahead(j);
			}
		}
	}
	// else
	// {
	// 	// show_current_step(sequencer.playingPattern);
	// }
}

void doStepS2()
{
	// // probability test
	bool testProb = probResult(sequencer.getCurrentPattern()->steps[sequencer.seqPos[sequencer.playingPattern]].prob);

	if (sequencer.playing)
	{
		unsigned long playstepmicros = micros();

		for (int j = 0; j < NUM_SEQ_PATTERNS; j++)
		{ // check all patterns for notes to play in time
			// CLOCK PER PATTERN BASED APPROACH
			auto pattern = sequencer.getPattern(j);

			// TODO: refactor timePerPattern stuff into sequencer.h

			if (playstepmicros >= sequencer.timePerPattern[j].nextStepTimeP)
			{

				seqReset(); // check for seqReset
				sequencer.timePerPattern[j].lastStepTimeP = sequencer.timePerPattern[j].nextStepTimeP;
				sequencer.timePerPattern[j].nextStepTimeP += (clockConfig.step_micros) * (multValues[sequencer.getPattern(j)->clockDivMultP]); // calc step based on rate

				// only play if not muted
				if (!sequencer.getPattern(j)->mute)
				{
					sequencer.timePerPattern[j].lastPosP = (sequencer.seqPos[j] + 15) % 16;
					if (lastNote[j][sequencer.timePerPattern[j].lastPosP] > 0)
					{
						step_off(j, sequencer.timePerPattern[j].lastPosP);
					}
					if (testProb)
					{
						if (evaluate_AB(pattern->steps[sequencer.seqPos[j]].condition, j))
						{
							playNote(j);
						}
					}
				}
				//						show_current_step(playingPattern);
				// if (j == sequencer.playingPattern)
				// { // only show selected pattern
				// 	show_current_step(sequencer.playingPattern);
				// }
				new_step_ahead(j);
			}
		}
	}
	// else
	// {
	// 	show_current_step(sequencer.playingPattern);
	// }
}

// TODO: move up to other sequencer stuff

// #### SEQ Mode note on/off
void seqNoteOn(int notenum, int velocity, int patternNum)
{
	int adjnote = notes[notenum] + (midiSettings.octave * 12); // adjust key for octave range
	if (adjnote >= 0 && adjnote < 128)
	{
		lastNote[patternNum][sequencer.seqPos[patternNum]] = adjnote;
		MM::sendNoteOn(adjnote, velocity, sequencer.getPatternChannel(sequencer.playingPattern));

		// keep track of adjusted note when pressed so that when key is released we send
		// the correct note off message
		midiSettings.midiKeyState[notenum] = adjnote;

		// CV
		if (sequencer.getCurrentPattern()->sendCV)
		{
			cvNoteUtil.cvNoteOn(adjnote);
		}
	}

	strip.setPixelColor(notenum, MIDINOTEON); //  Set pixel's color (in RAM)
	omxDisp.setDirty();
	omxLeds.setDirty();
}

void seqNoteOff(int notenum, int patternNum)
{
	// we use the key state captured at the time we pressed the key to send the correct note off message
	int adjnote = midiSettings.midiKeyState[notenum];
	if (adjnote >= 0 && adjnote < 128)
	{
		MM::sendNoteOff(adjnote, 0, sequencer.getPatternChannel(sequencer.playingPattern));
		// CV off
		if (sequencer.getCurrentPattern()->sendCV)
		{
			cvNoteUtil.cvNoteOff(adjnote);
		}
	}

	strip.setPixelColor(notenum, LEDOFF);
	omxDisp.setDirty();
	omxLeds.setDirty();
}

// Play a note / step (SEQUENCERS)
void playNote(int patternNum)
{
	//	Serial.println(sequencer.stepNoteP[patternNum][seqPos[patternNum]].note); // Debug
	auto pattern = sequencer.getPattern(patternNum);
	auto steps = pattern->steps;

	bool sendnoteCV = false;
	int rnd_swing;
	if (sequencer.getPattern(patternNum)->sendCV)
	{
		sendnoteCV = true;
	}
	StepType playStepType = (StepType)pattern->steps[sequencer.seqPos[patternNum]].stepType;

	if (steps[sequencer.seqPos[patternNum]].stepType == STEPTYPE_RAND)
	{
		auto tempType = random(STEPTYPE_COUNT);

		// this is fucking hacky to increment the enum for stepType
		switch (tempType)
		{
		case 0:
			playStepType = STEPTYPE_NONE;
			break;
		case 1:
			playStepType = STEPTYPE_RESTART;
			break;
		case 2:
			playStepType = STEPTYPE_FWD;
			break;
		case 3:
			playStepType = STEPTYPE_REV;
			break;
		case 4:
			playStepType = STEPTYPE_PONG;
			break;
		case 5:
			playStepType = STEPTYPE_RANDSTEP;
			break;
		}
		//		Serial.println(playStepType);
	}

	switch (playStepType)
	{
	case STEPTYPE_COUNT: // fall through
	case STEPTYPE_RAND:
		break;
	case STEPTYPE_NONE:
		break;
	case STEPTYPE_FWD:
		pattern->reverse = 0;
		break;
	case STEPTYPE_REV:
		pattern->reverse = 1;
		break;
	case STEPTYPE_PONG:
		pattern->reverse = !pattern->reverse;
		break;
	case STEPTYPE_RANDSTEP:
		sequencer.seqPos[patternNum] = (rand() % sequencer.getPatternLength(patternNum)) + 1;
		break;
	case STEPTYPE_RESTART:
		sequencer.seqPos[patternNum] = 0;
		break;
		break;
	}

	// regular note on trigger

	if (steps[sequencer.seqPos[patternNum]].trig == TRIGTYPE_PLAY)
	{
		sequencer.seq_velocity = steps[sequencer.seqPos[patternNum]].vel;

		uint8_t lenIndex = steps[sequencer.seqPos[patternNum]].len;
		float noteLength = kNoteLengths[lenIndex];

		// Delta = 12499.2 for 0.1 length at 120bpm
		// Delta = 3571.2 for 0.1 length at 300bpm
		// Delta = 8928 for 0.25 length at 300bpm

		seqConfig.noteoff_micros = micros() + (uint32_t)(noteLength * clockConfig.step_micros);

		if (sequencer.seqPos[patternNum] % 2 == 0)
		{

			if (pattern->swing < 99)
			{
				seqConfig.noteon_micros = micros() + ((clockConfig.ppqInterval * multValues[pattern->clockDivMultP]) / (PPQ / 24) * pattern->swing); // full range swing
				// 	Serial.println((clockConfig.ppqInterval * multValues[settings->clockDivMultP])/(PPQ / 24) * settings->swing);
				// } else if ((settings->swing > 50) && (settings->swing < 99)){
				//    noteon_micros = micros() + ((step_micros * multValues[settings->clockDivMultP]) * ((settings->swing - 50)* .01) ); // late swing
				//    Serial.println(((step_micros * multValues[settings->clockDivMultP]) * ((settings->swing - 50)* .01) ));
			}
			else if (pattern->swing == 99)
			{								 // random drunken swing
				rnd_swing = rand() % 95 + 1; // rand 1 - 95 // randomly apply swing value
				seqConfig.noteon_micros = micros() + ((clockConfig.ppqInterval * multValues[pattern->clockDivMultP]) / (PPQ / 24) * rnd_swing);
			}
		}
		else
		{
			seqConfig.noteon_micros = micros();
		}

		if (pendingNoteOffs.sendOffIfPresent(steps[sequencer.seqPos[patternNum]].note, sequencer.getPatternChannel(patternNum), sendnoteCV))
		{
			// Delay slightly so noteoff and note on are not on top of each other
			// seqConfig.noteon_micros += 1000;
			// seqConfig.noteoff_micros += 1000;
		}

		// Queue note-on
		pendingNoteOns.insert(steps[sequencer.seqPos[patternNum]].note, sequencer.seq_velocity, sequencer.getPatternChannel(patternNum), seqConfig.noteon_micros, sendnoteCV);

		// Pending Note Offs needs to happen after note-on
		pendingNoteOffs.insert(steps[sequencer.seqPos[patternNum]].note, sequencer.getPatternChannel(patternNum), seqConfig.noteoff_micros, sendnoteCV);

		// {notenum, vel, notelen, step_type, {p1,p2,p3,p4}, prob}
		// send param locks
		for (int q = 0; q < 4; q++)
		{
			int tempCC = steps[sequencer.seqPos[patternNum]].params[q];
			if (tempCC > -1)
			{
				MM::sendControlChange(pots[potSettings.potbank][q], tempCC, sequencer.getPatternChannel(patternNum));
				seqConfig.prevPlock[q] = tempCC;
			}
			else if (seqConfig.prevPlock[q] != potSettings.potValues[q])
			{
				// if (tempCC != seqConfig.prevPlock[q]) {
				MM::sendControlChange(pots[potSettings.potbank][q], potSettings.potValues[q], sequencer.getPatternChannel(patternNum));
				seqConfig.prevPlock[q] = potSettings.potValues[q];
			}
		}
		lastNote[patternNum][sequencer.seqPos[patternNum]] = steps[sequencer.seqPos[patternNum]].note;

		// CV is sent from pendingNoteOns/pendingNoteOffs
	}
}

void allNotesOff()
{
	pendingNoteOffs.allOff();
}

void allNotesOffPanic()
{
#if BOARDTYPE == TEENSY4
	dac.setVoltage(0, false);
#elif BOARDTYPE == OMX2040
	dac.setVoltage(0, false);
#else
	analogWrite(CVPITCH_PIN, 0);
#endif
	digitalWrite(CVGATE_PIN, LOW);
	for (int j = 0; j < 128; j++)
	{
		MM::sendNoteOff(j, 0, sysSettings.midiChannel); // NEEDS FIXING
	}
}

void transposeSeq(int patternNum, int amt)
{
	auto pattern = sequencer.getPattern(patternNum);
	for (int k = 0; k < NUM_STEPS; k++)
	{
		pattern->steps[k].note += amt;
	}
}

void seqReset()
{
	if (sequencer.seqResetFlag)
	{
		for (int k = 0; k < NUM_SEQ_PATTERNS; k++)
		{
			for (int q = 0; q < NUM_STEPS; q++)
			{
				loopCount[k][q] = 0;
			}
			if (sequencer.getPattern(k)->reverse)
			{ // REVERSE
				sequencer.seqPos[k] = sequencer.getPatternLength(k) - 1;
				sequencer.lastSeqPos[k] = sequencer.seqPos[k];
			}
			else
			{
				sequencer.seqPos[k] = 0;
				sequencer.lastSeqPos[k] = sequencer.seqPos[k];
			}
		}
		// omxUtil.stopClocks();
		// omxUtil.startClocks();
		// MM::stopClock();
		// MM::startClock();
		sequencer.seqResetFlag = false;
	}
}

void seqStart()
{
	sequencer.playing = true;

	for (int x = 0; x < NUM_SEQ_PATTERNS; x++)
	{
		sequencer.timePerPattern[x].nextStepTimeP = micros();
		sequencer.timePerPattern[x].lastStepTimeP = micros();
	}

	if (!sequencer.seqResetFlag)
	{
		omxUtil.resumeClocks();
		// MM::continueClock();
		//	} else if (seqPos[sequencer.playingPattern]==0) {
		//		MM::startClock();
	// 	} else if (sequencer.seqPos[sequencer.playingPattern]==0) {
	} else  {
		omxUtil.startClocks();
// 		MM::startClock();
	}
}

void seqStop()
{
	sequencer.ticks = 0;
	sequencer.playing = false;
	omxUtil.stopClocks();
	// MM::stopClock();
	allNotesOff();
}

void seqContinue()
{
	sequencer.playing = true;
	omxUtil.resumeClocks();
}

int getPatternPage(int position)
{
	return position / NUM_STEPKEYS;
}

void rotatePattern(int patternNum, int rot)
{
	if (patternNum < 0 || patternNum >= NUM_SEQ_PATTERNS)
		return;

	auto pattern = sequencer.getPattern(patternNum);
	int size = sequencer.getPatternLength(patternNum);
	StepNote arr[size];
	rot = (rot + size) % size;

	for (int d = rot, s = 0; s < size; d = (d + 1) % size, ++s)
		arr[d] = pattern->steps[s];

	for (int i = 0; i < size; ++i)
		pattern->steps[i] = arr[i];
}

void resetPatternDefaults(int patternNum)
{
	auto pattern = sequencer.getPattern(patternNum);

	for (int i = 0; i < NUM_STEPS; i++)
	{
		// {notenum,vel,len,stepType,{p1,p2,p3,p4,p5}}
		pattern->steps[i].note = sequencer.patternDefaultNoteMap[patternNum];
		pattern->steps[i].len = 3;
	}
}

void clearPattern(int patternNum)
{
	auto steps = sequencer.getPattern(patternNum)->steps;

	for (int i = 0; i < NUM_STEPS; i++)
	{
		// {notenum,vel,len,stepType,{p1,p2,p3,p4,p5}}
		steps[i].note = sequencer.patternDefaultNoteMap[patternNum];
		steps[i].vel = midiSettings.defaultVelocity;
		steps[i].len = 3; // Default 0.75
		steps[i].trig = TRIGTYPE_MUTE;
		steps[i].stepType = STEPTYPE_NONE;
		steps[i].params[0] = -1;
		steps[i].params[1] = -1;
		steps[i].params[2] = -1;
		steps[i].params[3] = -1;
		steps[i].params[4] = -1;
		steps[i].prob = 100;
		steps[i].condition = 0;
	}
}

void copyPattern(int patternNum)
{
	// for( int i = 0 ; i < NUM_STEPS ; ++i ){
	//	copyPatternBuffer[i] = sequencer.stepNoteP[patternNum][i];
	// }
	auto pattern = sequencer.getPattern(patternNum);
	memcpy(&copyPatternBuffer, &pattern->steps, NUM_STEPS * sizeof(StepNote));
}

void pastePattern(int patternNum)
{
	// for( int i = 0 ; i < NUM_STEPS ; ++i ){
	//	sequencer.stepNoteP[patternNum][i] = copyPatternBuffer[i] ;
	// }
	auto pattern = sequencer.getPattern(patternNum);
	memcpy(&pattern->steps, &copyPatternBuffer, NUM_STEPS * sizeof(StepNote));
}

// global sequencer shared state
SequencerState sequencer = defaultSequencer();
