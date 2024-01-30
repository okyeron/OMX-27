#pragma once

#include <Arduino.h>
#include "../config.h"
// #define NUM_GRIDS 8

namespace euclidean
{
	// extern const float kEuclidNoteLengths[10];
	// extern const uint8_t kNumEuclidNoteLengths;

	struct EuclidSave
	{
		uint8_t rotation_ : 6;
		uint8_t events_ : 6;
		uint8_t steps_ : 6;

		uint8_t noteNumber_ : 7;
		uint8_t midiChannel_ : 4;
		uint8_t velocity_ : 7;
		uint8_t swing_ : 7;

		uint8_t noteLength_ : 4;
		int8_t midifx : 4;

		bool muted = false;

		bool polyRhythmMode_ = true;

		uint8_t clockDivMultP_ : 3;
		uint8_t polyRClockDivMultP_ : 3;

		EuclidSave()
		{
			rotation_ = 0;
			events_ = 0;
			steps_ = 0;

			noteNumber_ = 60;
			midiChannel_ = 0;
			velocity_ = 100;
			swing_ = 0;
			noteLength_ = 1;
			midifx = 0;
			muted = false;
			polyRhythmMode_ = false;
			clockDivMultP_ = 4;
			polyRClockDivMultP_ = 4;
		}
	};

	// #define EUCLID_PAT_SIZE = 32
	// enum Grid_Resolutions
	// {
	//     HALF = 0,
	//     NORMAL,
	//     DOUBLE,
	//     FOUR,
	//     COUNT
	// };

	// struct InstSettings
	// {
	//     uint8_t note = 60;
	//     uint8_t midiChan = 1;
	//     uint8_t density = 0;
	//     uint8_t x = 128;
	//     uint8_t y = 128;
	// };

	// struct SnapShotSettings
	// {
	//     InstSettings instruments[4];
	//     uint8_t chaos = 0;
	//     uint8_t accent = 128;
	//     uint8_t resolution = 1;
	// };

	// constexpr uint8_t kStepsPerPattern = 32;

	// struct ChannelPatternLEDs
	// {
	//     uint8_t levels[kStepsPerPattern];
	// };

	class EuclideanMath
	{
	public:
		static const uint8_t kPatternSize = 32; // All pattern arrays are 32 length
		EuclideanMath();

		// bool array should be of length kPatternSize
		static void generateEuclidPattern(bool *pattern, uint8_t events, uint8_t steps);
		// bool array should be of length kPatternSize
		static void clearPattern(bool *pattern);
		// bool array should be of length kPatternSize
		static void flipPattern(bool *pattern, uint8_t steps);
		// bool array should be of length kPatternSize
		static void rotatePattern(bool *pattern, uint8_t steps, uint8_t rotation);
	};

	class EuclideanSequencer
	{
	public:
		// uint8_t grids_notes[4] = {36, 38, 42, 46};
		// static const uint8_t num_notes = sizeof(grids_notes);
		// uint8_t playingPattern = 0;

		// static const uint8_t kStepsPerPattern = 16;

		uint8_t midiFXGroup = 0;

		// SnapShotSettings snapshots[8];

		EuclideanSequencer();

		void start();
		void stop();
		void proceed();
		void clockTick(uint32_t stepmicros, uint32_t microsperstep);

		// void saveSnapShot(uint8_t snapShotIndex);
		// void loadSnapShot(uint8_t snapShotIndex);
		// SnapShotSettings* getSnapShot(uint8_t snapShotIndex);
		// void setSnapShot(uint8_t snapShotIndex, SnapShotSettings snapShot);

		static uint32_t randomValue(uint32_t init = 0);

		// ChannelPatternLEDs getChannelLEDS(uint8_t channel);

		// uint8_t getSeqPos();

		// bool getChannelTriggered(uint8_t chanIndex);

		// void setMidiChan(uint8_t chanIndex, uint8_t channel);
		// uint8_t getMidiChan(uint8_t chanIndex);

		bool isDirty();

		bool isRunning();

		void setNoteOutputFunc(void (*fptr)(void *, uint8_t, MidiNoteGroup), void *context, u_int8_t euclidIndex);

		void setMute(bool mute);
		bool getMute();

		bool getTriggered();
		bool getClockAdvanced();

		void setClockDivMult(uint8_t m);
		uint8_t getClockDivMult();

		void setPolyRClockDivMult(uint8_t m);
		uint8_t getPolyRClockDivMult();

		void setRotation(uint8_t newRotation);
		uint8_t getRotation();

		void setEvents(uint8_t newEvents);
		uint8_t getEvents();

		void setSteps(uint8_t newSteps);
		uint8_t getSteps();

		void setNoteNumber(uint8_t newNoteNumber);
		uint8_t getNoteNumber();

		void setMidiChannel(uint8_t newMidiChannel);
		uint8_t getMidiChannel();

		void setVelocity(uint8_t newVelocity);
		uint8_t getVelocity();

		void setSwing(uint8_t newSwing);
		uint8_t getSwing();

		void setNoteLength(uint8_t newNoteLength);
		uint8_t getNoteLength();

		void setPolyRhythmMode(bool enable);
		bool getPolyRhythmMode();

		uint8_t getSeqPos();
		uint8_t getLastSeqPos();

		float getSeqPerc();

		bool *getPattern();

		void printEuclidPattern();

		EuclidSave getSave();
		void loadSave(EuclidSave save);

	private:
		// GridsChannel channel_;
		uint32_t divider_;
		float multiplier_ = 1;
		float multiplierPR_ = 1;
		uint32_t tickCount_;
		// uint8_t density_[num_notes];
		// uint8_t perturbations_[num_notes];
		// uint8_t x_[num_notes];
		// uint8_t y_[num_notes];
		// uint8_t midiChannels_[num_notes];
		// bool channelTriggered_[num_notes];
		// uint8_t triggeredNotes_[num_notes]; // Keep track of triggered notes to avoid stuck notes
		// uint8_t resolution_;
		bool running_;
		bool muted_ = false;

		// Note On pointers
		uint8_t euclidIndex_;
		void *onNoteOnFuncPtrContext_;
		void (*onNoteOnFuncPtr_)(void *, uint8_t, MidiNoteGroup);
		void onNoteOn(uint8_t channel, uint8_t noteNumber, uint8_t velocity, float stepLength, bool sendMidi, bool sendCV, uint32_t noteOnMicros);

		// uint8_t defaultMidiChannel_ = 1;

		uint8_t rotation_ = 0;
		uint8_t events_ = 0;
		uint8_t steps_ = 0;

		uint8_t noteNumber_ = 16;
		uint8_t midiChannel_ = 1;
		uint8_t velocity_ = 100;
		uint8_t swing_ = 0;

		uint8_t noteLength_ = 1;

		bool polyRhythmMode_ = true;

		bool patternDirty_ = false;

		bool triggered_ = false;
		bool clockAdvanced_ = false;

		// Clock timings
		Micros lastProcessTimeP_ = 32;
		Micros nextStepTimeP_ = 32;
		Micros lastStepTimeP_ = 32;
		uint8_t lastPosP_ = 16;
		uint8_t clockDivMultP_ = 4;
		uint8_t polyRClockDivMultP_ = 4;

		uint8_t seqPos_ = 0;
		uint8_t lastSeqPos_ = 0;

		float seqPerc_ = 0;
		uint32_t stepMicroDelta_ = 0;
		uint32_t startMicros = 0;
		uint32_t triggerOffMicros_ = 0;

		bool pattern_[EuclideanMath::kPatternSize];
		void regeneratePattern();

		void advanceStep(uint32_t stepmicros);
		void autoReset();
		void playNote();
	};

}
