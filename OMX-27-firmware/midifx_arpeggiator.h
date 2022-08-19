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
            uint8_t noteNumber : 7; // 7 Bytes, max 127;
            uint8_t velocity : 7;
            bool sendMidi = false;
            bool sendCV = false;

            ArpNote(MidiNoteGroup noteGroup)
            {
                noteNumber = noteGroup.noteNumber;
                velocity = noteGroup.velocity;
                sendMidi = noteGroup.sendMidi;
                sendCV = noteGroup.sendCV;
            }
        };

        static inline bool compareArpNote(ArpNote a1, ArpNote a2)
        {
            return (a1.noteNumber < a2.noteNumber);
        }

        uint8_t midiChannel : 4; // 0-15, Add 1 when using

        uint8_t chancePerc_ = 255;

        bool arpRunning_ = false;

        static const int queueSize = 8;

        std::vector<ArpNote> playedNoteQueue;
        std::vector<ArpNote> sortedNoteQueue;

        Micros nextStepTimeP_ = 32;
        Micros lastStepTimeP_ = 32;
        uint32_t stepMicroDelta_ = 0;

        uint8_t rateIndex = 0;

        float multiplier_ = 1;

        bool insertMidiNoteQueue(MidiNoteGroup note);
        bool removeMidiNoteQueue(MidiNoteGroup note);

        void sortNotes();

        bool hasMidiNotes();

        void arpNoteOn(MidiNoteGroup note);
        void arpNoteOff(MidiNoteGroup note);

        void startArp();
        void stopArp();

        void arpNoteTrigger();

    };
}
