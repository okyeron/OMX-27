#pragma once

#include "midifx_interface.h"

namespace midifx
{
    enum ArpMode
    {
        ARPMODE_OFF,
        ARPMODE_ON,
        ARPMODE_ONESHOT,
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
        ARPPAT_HI_UP,            // Alternates between highest note: BGBEBC-BGBEBC
        ARPPAT_HI_UP_DOWN,       // BGBEBCBE-BGBEBCBE
        ARPPAT_LOW_UP,            // Alternates between lowest note: CECGCB-CECGCB
        ARPPAT_LOW_UP_DOWN,       // CECGCBCG-CECGCBCG
        ARPPAT_AS_PLAYED,           // Plays notes in the order they are played
        ARPPAT_RAND,                // Plays notes randomly, same note could get played twice: GGEGCBB-
        ARPPAT_RAND_OTHER,          // Plays notes randomly, but won't play same note in a row: EGEBCEB
        ARPPAT_RAND_ONCE,           // Plays notes randomly only once, so all notes get played: GCBE
        ARPPAT_NUM_OF_PATS
    };

    class MidiFXArpeggiator : public MidiFXInterface
    {
    public:
        MidiFXArpeggiator();
        ~MidiFXArpeggiator() {}

        int getFXType() override;
        const char* getName() override;
        const char* getDispName() override;

        MidiFXInterface* getClone() override;

        void onModeChanged() override;

        void loopUpdate() override;

        void onDisplayUpdate() override;

        void noteInput(MidiNoteGroup note) override;
        // MidiFXNoteFunction getInputFunc() override;

        int saveToDisk(int startingAddress, Storage *storage) override;
        int loadFromDisk(int startingAddress, Storage *storage) override;

    protected:
        void onEnabled() override;
        void onDisabled() override;

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
                if(noteNumber < 0 || noteNumber > 127)
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

        static inline bool compareArpNote(ArpNote a1, ArpNote a2)
        {
            return (a1.noteNumber < a2.noteNumber);
        }

        uint8_t arpMode_ : 3;

        uint8_t arpPattern_ : 5;

        // bool holdNotes_;

        uint8_t midiChannel_ : 4; // 0-15, Add 1 when using

        uint8_t swing_ : 7; // max 100

        uint8_t rateIndex_ : 4; // max 15

        uint8_t octaveRange_ : 4; // max 7, 0 = 1 octave

        uint8_t gate = 100; // 0 - 200

        int arpSize = sizeof(ArpNote);


        uint8_t velocity_ : 7;
        bool sendMidi_ = false;
        bool sendCV_ = false;

        uint8_t randPrevNote_;


        bool arpRunning_ = false;

        static const int queueSize = 8;

        std::vector<ArpNote> playedNoteQueue; // Keeps track of which notes are being played
        std::vector<ArpNote> holdNoteQueue; // Holds notes
        std::vector<ArpNote> sortedNoteQueue; // Notes that are used in arp
        std::vector<ArpNote> tempNoteQueue; // Notes that are used in arp

        int8_t notePos_;
        uint8_t octavePos_;

        ArpNote notePat_[256];
        int notePatLength_ = 0;
        int patPos_;
        bool goingUp_;

        Micros nextStepTimeP_ = 32;
        Micros lastStepTimeP_ = 32;
        uint32_t stepMicroDelta_ = 0;


        float multiplier_ = 1;

        String tempString_;

        bool insertMidiNoteQueue(MidiNoteGroup note);
        bool removeMidiNoteQueue(MidiNoteGroup note);

        void sortNotes();
        void generatePattern();

        bool hasMidiNotes();

        void arpNoteOn(MidiNoteGroup note);
        void arpNoteOff(MidiNoteGroup note);

        void startArp();
        void stopArp();
        void resetArpSeq();

        void arpNoteTrigger();
        void playNote(uint32_t noteOnMicros, ArpNote note);
    };
}
