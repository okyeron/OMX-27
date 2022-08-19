#pragma once

#include "midifx_interface.h"

namespace midifx
{
    class MidiFXArpeggiator : public MidiFXInterface
    {
    public:
        MidiFXArpeggiator();
        ~MidiFXArpeggiator() {}

        int getFXType() override;
        const char* getName() override;
        const char* getDispName() override;

        MidiFXInterface* getClone() override;

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

        uint8_t midiChannel_ : 4; // 0-15, Add 1 when using

        uint8_t chancePerc_ : 7; // max 100
        
        uint8_t swing_ : 7; // max 100

        uint8_t rateIndex_ : 4; // max 15

        uint8_t octaveRange_ : 4; // max 7, 0 = 1 octave

        uint8_t gate = 100; // 0 - 200

        int arpSize = sizeof(ArpNote);


        uint8_t velocity_ : 7;
        bool sendMidi_ = false;
        bool sendCV_ = false;


        bool arpRunning_ = false;

        static const int queueSize = 8;

        std::vector<ArpNote> playedNoteQueue;
        std::vector<ArpNote> sortedNoteQueue;

        uint8_t notePos_;
        uint8_t octavePos_;

        ArpNote notePat_[256];
        int notePatLength_ = 0;
        int patPos_;

        Micros nextStepTimeP_ = 32;
        Micros lastStepTimeP_ = 32;
        uint32_t stepMicroDelta_ = 0;


        float multiplier_ = 1;

        

        bool insertMidiNoteQueue(MidiNoteGroup note);
        bool removeMidiNoteQueue(MidiNoteGroup note);

        void sortNotes();
        void generatePattern();

        bool hasMidiNotes();

        void arpNoteOn(MidiNoteGroup note);
        void arpNoteOff(MidiNoteGroup note);

        void startArp();
        void stopArp();

        void arpNoteTrigger();
        void playNote(uint32_t noteOnMicros, ArpNote note);
    };
}
