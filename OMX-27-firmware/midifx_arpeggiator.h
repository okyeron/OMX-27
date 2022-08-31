#pragma once

#include "midifx_interface.h"

namespace midifx
{
    enum ArpMode
    {
        ARPMODE_OFF,
        ARPMODE_ON,
        ARPMODE_ONESHOT,
        ARPMODE_ONCE,
        ARPMODE_HOLD
    };
    
    // in: C,E,G,B
    enum ArpPattern
    {
        ARPPAT_UP,                  // Plays notes from lowest to highest: CEGB-CEGB
        ARPPAT_DOWN,                // Plays notes from highest to loweest: BGEC-BGEC
        ARPPAT_UP_DOWN,             // Plays notes up then down: CEGBGE-CEGBGE
        ARPPAT_DOWN_UP,             // Plays notes down then up: BGECEG-BGECEG
        ARPPAT_UP_AND_DOWN,         // Plays notes up then down, end notes repeat: CEGBBGEC-CEGBBGEC
        ARPPAT_DOWN_AND_UP,         // Down then up, ends repeat: BGECCEGB
        ARPPAT_CONVERGE,            // Converges notes to center point: CBEG-CBEG
        ARPPAT_DIVERGE,             // Diverges notes from center: GEBC-GEBC
        ARPPAT_CONVERGE_DIVERGE,    // Converges then diverges: CBEGEB-CBEGEB
        ARPPAT_HI_UP,               // Alternates between highest note: BGBEBC-BGBEBC
        ARPPAT_HI_UP_DOWN,          // BGBEBCBE-BGBEBCBE
        ARPPAT_LOW_UP,              // Alternates between lowest note: CECGCB-CECGCB
        ARPPAT_LOW_UP_DOWN,         // CECGCBCG-CECGCBCG
        ARPPAT_RAND,                // Plays notes randomly, same note could get played twice: GGEGCBB-
        ARPPAT_RAND_OTHER,          // Plays notes randomly, but won't play same note in a row: EGEBCEB
        ARPPAT_RAND_ONCE,           // Plays notes randomly only once, so all notes get played: GCBE
        ARPPAT_AS_PLAYED,           // Plays notes in the order they are played
        ARPPAT_NUM_OF_PATS
    };

    enum ModPattern
    {
        MODPAT_ARPNOTE,             // Plays note as generated by arp
        MODPAT_REST,                // Skips note
        MODPAT_TIE,                 // Increases length of previous note
        MODPAT_REPEAT,              // Repeats the last note played
        MODPAT_LOWPITCH_OCTAVE,     // Lowest pitch minus 1 octave
        MODPAT_HIGHPITCH_OCTAVE,    // Highest pitch plus 1 octave
        MODPAT_PWRCHORD,            // Plays a power chord of lowest and highest note
        MODPAT_CHORD,               // Plays a chord of all the notes being played
        MODPAT_NOTE1,               // Plays 1st note as played
        MODPAT_NOTE2,               // 2nd note as played
        MODPAT_NOTE3,
        MODPAT_NOTE4,
        MODPAT_NOTE5,
        MODPAT_NOTE6,
        MODPAT_NUM_OF_MODS
    };

    enum ArpResetMode
    {
        ARPRESET_NORMAL,            // Resets after reaching end of arp pattern and octave range
        ARPRESET_NOTE,              // Resets whenever a new note is added to arp
        ARPRESET_MODPAT,            // Resets after mod pattern is completed
        ARPRESET_TRANSPOSEPAT       // Resets after the transpose pattern is completed
    };

    class MidiFXArpeggiator : public MidiFXInterface
    {
    public:
        MidiFXArpeggiator();
        ~MidiFXArpeggiator() {}

        int getFXType() override;
        const char* getName() override;
        const char* getDispName() override;
        uint32_t getColor() override;

        MidiFXInterface* getClone() override;

        void onModeChanged() override;

        void loopUpdate() override;
        void onClockTick() override;
        void resync() override;

        bool usesKeys() override;
        void onKeyUpdate(OMXKeypadEvent e, uint8_t funcKeyMode) override;
        void onKeyHeldUpdate(OMXKeypadEvent e, uint8_t funcKeyMode) override;
        void updateLEDs(uint8_t funcKeyMode) override;

        void onDisplayUpdate(uint8_t funcKeyMode) override;

        void noteInput(MidiNoteGroup note) override;
        // MidiFXNoteFunction getInputFunc() override;

        int saveToDisk(int startingAddress, Storage *storage) override;
        int loadFromDisk(int startingAddress, Storage *storage) override;

        // Toggles between off and previous mode
        void toggleArp();
        void toggleHold();
        void nextArpPattern();
        void nextOctRange();

        bool isOn();
        bool isHoldOn();

        uint8_t getOctaveRange();

    protected:
        void onEnabled() override;
        void onDisabled() override;

        void onSelected() override;
        void onDeselected() override;

        void onEncoderChangedEditParam(Encoder::Update enc) override;

    private:
        struct ArpNote
        {
            // bool inUse = false;
            uint8_t noteNumber;
            // uint8_t velocity : 7;
            // bool sendMidi = false;
            // bool sendCV = false;

            ArpNote()
            {
                noteNumber = 255;
            }

            ArpNote(int noteNumber)
            {
                if (noteNumber < 0 || noteNumber > 127)
                {
                    noteNumber = 255;
                }
                this->noteNumber = noteNumber;
            }

            ArpNote(MidiNoteGroup noteGroup)
            {
                noteNumber = noteGroup.noteNumber;
                // velocity = noteGroup.velocity;
                // sendMidi = noteGroup.sendMidi;
                // sendCV = noteGroup.sendCV;
            }
        };

        struct PendingArpNote
        {
            MidiNoteGroupCache noteCache;
            Micros offTime;
        };

        // In struct to limit bits
        struct ArpMod
        {
            uint8_t mod : 4;

            ArpMod()
            {
                mod = 0;
            }
        };

        struct ArpSave
        {
            uint8_t chancePerc : 7;
            uint8_t arpMode : 3;
            uint8_t arpPattern : 5;
            uint8_t resetMode : 3;
            uint8_t midiChannel : 4; // 0-15, Add 1 when using
            uint8_t swing : 7;       // max 100
            uint8_t rateIndex : 4;   // max 15
            uint8_t octaveRange : 4; // max 7, 0 = 1 octave
            int8_t octDistance_ : 6; // -24 to 24
            uint8_t gate : 7;       // 0 - 200

            uint8_t modPatternLength : 4; // Max 15
            ArpMod modPattern[16];

            uint8_t transpPatternLength : 4; // Max 15
            int8_t transpPattern[16];
        };

        static inline bool
        compareArpNote(ArpNote a1, ArpNote a2)
        {
            return (a1.noteNumber < a2.noteNumber);
        }

        uint8_t chancePerc_ : 7;

        uint8_t arpMode_ : 3;

        uint8_t arpPattern_ : 5;

        uint8_t resetMode_ : 3;

        // bool holdNotes_;

        uint8_t midiChannel_ : 4; // 0-15, Add 1 when using

        uint8_t swing_ : 7; // max 100

        uint8_t rateIndex_ : 4; // max 15

        uint8_t octaveRange_ : 4; // max 7, 0 = 1 octave
        int8_t octDistance_ : 6; // -24 to 24


        uint8_t gate = 90; // 0 - 200

        // int arpSize = sizeof(ArpNote);


        uint8_t velocity_ : 7;
        bool sendMidi_ = false;
        bool sendCV_ = false;

        uint8_t randPrevNote_;

        bool pendingStart_ = false;
        bool pendingStop_ = false;
        Micros pendingStartTime_;
        uint8_t pendingStopCount_ = 0;

        bool arpRunning_ = false;

        static const int queueSize = 8;

        std::vector<ArpNote> playedNoteQueue; // Keeps track of which notes are being played
        std::vector<ArpNote> holdNoteQueue; // Holds notes
        std::vector<ArpNote> sortedNoteQueue; // Notes that are used in arp
        std::vector<ArpNote> tempNoteQueue; // Notes that are used in arp

        std::vector<ArpNote> prevSortedNoteQueue;

        std::vector<PendingArpNote> pendingNotes; // Notes that are used in arp

        uint8_t modPatternLength_ : 4; // Max 15
        ArpMod modPattern_[16];

        uint8_t transpPatternLength_ : 4; // Max 15
        int8_t transpPattern_[16];

        uint8_t modPos_ : 5;
        uint8_t transpPos_ : 5;
        int8_t notePos_;
        uint8_t octavePos_ : 4;
        uint8_t syncPos_ : 5;

        uint8_t lowestPitch_;
        uint8_t highestPitch_;
        uint8_t stepLength_ = 1; // length of note in arp steps

        // ArpNote notePat_[256];
        // int notePatLength_ = 0;
        int patPos_;
        bool goingUp_;

        int8_t heldKey16_ = -1; // Key that is held

        int8_t modCopyBuffer_;
        int8_t transpCopyBuffer_;

        int16_t lastPlayedNoteNumber_;
        int8_t lastPlayedMod_;

        Micros nextStepTimeP_ = 32;
        Micros lastStepTimeP_ = 32;
        uint32_t stepMicroDelta_ = 0;

        

        float multiplier_ = 1;

        String tempString_;
        String tempString2_;
        String tempString3_;

        String headerMessage_;

        int messageTextTimer = 0;

        // Used for toggling arp
        uint8_t prevArpMode_ : 3;

        int8_t prevNotePos_;
        int8_t nextNotePos_;
        int8_t prevQLength_;

        bool resetNextTrigger_;
        bool sortOrderChanged_;

        MidiNoteGroup trackingNoteGroups[8];

        bool insertMidiNoteQueue(MidiNoteGroup note);
        bool removeMidiNoteQueue(MidiNoteGroup note);

        void findIndexOfNextNotePos();

        void sortNotes();
        // void generatePattern();

        bool hasMidiNotes();

        void trackNoteInput(MidiNoteGroup note);
        void processNoteInput(MidiNoteGroup note);

        void arpNoteOn(MidiNoteGroup note);
        void arpNoteOff(MidiNoteGroup note);

        void startArp();
        void doPendingStart();
        void stopArp();
        void doPendingStop();
        void resetArpSeq();

        void arpNoteTrigger();
        int16_t applyModPattern(int16_t note);
        uint8_t findStepLength();
        int16_t applyTranspPattern(int16_t note);

        void playNote(uint32_t noteOnMicros, int16_t noteNumber, uint8_t velocity);

        void showMessage();

        bool isModeHold(uint8_t arpMode);

        void changeArpMode(uint8_t newArpMode);
    };
}
