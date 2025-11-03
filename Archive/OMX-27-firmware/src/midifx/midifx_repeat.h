#pragma once

#include "midifx_interface.h"
#include "midifx_notemaster.h"

namespace midifx
{

	class MidiFXRepeat : public MidiFXInterface
	{
	public:
		MidiFXRepeat();
		~MidiFXRepeat();

		int getFXType() override;
		const char *getName() override;
		const char *getDispName() override;

		MidiFXInterface *getClone() override;

		void loopUpdate() override;
        void onClockTick() override;
		void resync() override;

		void onDisplayUpdate(uint8_t funcKeyMode) override;

		void noteInput(MidiNoteGroup note) override;

		int saveToDisk(int startingAddress, Storage *storage) override;
		int loadFromDisk(int startingAddress, Storage *storage) override;

    protected:
		void onEnabled() override;
		void onDisabled() override;

		void onEncoderChangedEditParam(Encoder::Update enc) override;

	private:
        struct FixedLengthNote
		{
			MidiNoteGroupCache noteCache;
			Micros offTime;
		};

        struct RepeatNote
        {
            // bool playing; // Not needed now with two queues
            // bool inUse = false;
            uint8_t noteNumber;
            uint8_t channel : 4;
            uint8_t velocity : 7;
            uint8_t velocityStart : 7;
            uint8_t velocityEnd : 7;
            uint8_t repeatCounter : 7;

            // bool sendMidi = false;
            // bool sendCV = false;

            Micros nextTriggerDelta = 0; // Delta time to next trigger
            Micros nextTriggerTime = 0; // Time in global microseconds when note should trigger next

            RepeatNote()
            {
                noteNumber = 255;
                velocity = 100;
                channel = 0;
            }

            RepeatNote(int noteNumber, uint8_t velocity, uint8_t channel)
            {
                if (noteNumber < 0 || noteNumber > 127)
                {
                    noteNumber = 255;
                }
                else
                {
                    this->noteNumber = noteNumber;
                    this->velocity = velocity;
                    this->channel = channel - 1;
                }
            }

            RepeatNote(MidiNoteGroup *noteGroup)
            {
                if (noteGroup->noteNumber < 0 || noteGroup->noteNumber > 127)
                {
                    noteNumber = 255;
                    return;
                }
                this->noteNumber = noteGroup->noteNumber;
                this->velocity = noteGroup->velocity;
                this->channel = noteGroup->channel - 1;
            }
        };

        struct RepeatSave
        {
            uint8_t chancePerc : 7;
            uint8_t numOfRepeats : 6;
            uint8_t mode : 3;
            int8_t rateIndex : 5;
            int8_t quantizedRateIndex_ : 5;
            uint8_t rateHz;
            uint8_t gate : 8;
            bool fadeVel_;
            uint8_t velStart : 7;
            uint8_t velEnd : 7;
            bool fadeRate_;
            uint8_t rateStart_;
            uint8_t rateEnd_; 
            uint8_t rateStartHz_;
            uint8_t rateEndHz_;
        };
        // Saved variables
        uint8_t chancePerc_ = 100;
		uint8_t numOfRepeats_ : 6; // 1 to 64, stored as 0 - 63
        uint8_t mode_ : 3; // Off, 1-Shot - Repeats for numOfRepeats_ restarts on new note on, On - Repeats indefinitely while key is hold, Hold - Endlessly repeats, 
        int8_t rateIndex_ : 5; // max 15 or -1 for hz
        int8_t quantizedRateIndex_ : 5; // max 15 or -1 for hz
        uint8_t rateHz_; // 0-255, gets remapped to a hertz float value
        uint8_t gate_ : 8; // 0-200
        bool fadeVel_; // Fade the velocity if true
        uint8_t velStart_ : 7; // 0-127
        uint8_t velEnd_ : 7; // 0-127
        bool fadeRate_; // Fade the rate if true
        uint8_t rateStart_ : 4; // 0-15
        uint8_t rateEnd_ : 4; // 0-15
        uint8_t rateStartHz_; // 0-255, gets remapped to a hertz float value
        uint8_t rateEndHz_; // 0-255

        // Consts
		static const int queueSize = 16;

        bool seqRunning_;

        // Timing stuff
        bool multiplierCalculated_ = false; // Used to prevent recalculating multiplier
		float multiplier_ = 1;
		uint8_t stepLength_ = 1; // length of note in arp steps

        // Micros nextStepTimeP_ = 32;
		// Micros lastStepTimeP_ = 32;
		// uint32_t stepMicroDelta_ = 0;

		// Micros last16thTime_ = 0;
		// Micros next16thTime_ = 0;

        // Calculated
        bool quantizeSync_ = true;

        float velStartPerc_ = 1.0f;
        float velEndPerc_ = 1.0f;

		float rateStartMult_ = 1;
		float rateEndMult_ = 1;

        // Rate in hertz
        float rateInHz_;
        float rateStartInHz_;
        float rateEndInHz_;

        MidiFXNoteMaster noteMaster;

        static void processNoteForwarder(void *context, MidiNoteGroup *note)
        {
            static_cast<MidiFXRepeat *>(context)->processNoteInput(note);
        }

        static void sendNoteOutForwarder(void *context, MidiNoteGroup *note)
        {
            static_cast<MidiFXRepeat *>(context)->sendNoteOut(*note);
        }

        std::vector<RepeatNote> playedNoteQueue; // Keeps track of which notes are being played
		std::vector<RepeatNote> activeNoteQueue;	  // Holds notes
		std::vector<RepeatNote> pendingNoteQueue;	  // notes pending for quantization

		std::vector<RepeatNote> tempNoteQueue;	  // Notes that are used in arp

		std::vector<FixedLengthNote> fixedLengthNotes; // Tracking of fixed length notes

		// MidiNoteGroup trackingNoteGroups[8];
		// MidiNoteGroup trackingNoteGroupsPassthrough[8];

        void trackNoteInputPassthrough(MidiNoteGroup *note, bool ignoreNoteOns);
        void trackNoteInput(MidiNoteGroup *note);
        void processNoteInput(MidiNoteGroup *note);

		bool hasMidiNotes();
        bool useRateHz();
        void updateMultiplier();
        bool insertMidiNoteQueue(MidiNoteGroup *note);
		bool removeMidiNoteQueue(MidiNoteGroup *note);

        static bool removeFromQueue(std::vector<RepeatNote> *queue, MidiNoteGroup *note);
        static float rateToHz(uint8_t rateHz);

        void changeRepeatMode(uint8_t newMode);
        void repeatNoteOn(MidiNoteGroup *note);
        void repeatNoteOff(MidiNoteGroup *note);
        void startSeq();
		void stopSeq();
        void resetArpSeq();

        void recalcVariables();

        void triggerNote(RepeatNote note);
        // void repeatNoteTrigger();
		void playNote(uint32_t noteOnMicros, uint32_t lengthDelta, int16_t noteNumber, uint8_t velocity, uint8_t channel);
	};
}
