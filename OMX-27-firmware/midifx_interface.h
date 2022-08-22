#pragma once
#include "config.h"
#include "ClearUI_Input.h"
#include "omx_keypad.h"
#include "param_manager.h"
#include "storage.h"

namespace midifx
{
    // Lighter version of MidiNoteGroup for tracking note offs
    struct MidiNoteGroupCache
    {
        uint8_t prevNoteNumber = 0;
        uint8_t channel = 1;
        uint8_t noteNumber = 0;
        bool sendMidi = true;
        bool sendCV = true;
        bool unknownLength = false;

        MidiNoteGroupCache()
        {
        }

        MidiNoteGroupCache(MidiNoteGroup noteGroup)
        {
            setFromNoteGroup(noteGroup);
        }

        void setFromNoteGroup(MidiNoteGroup noteGroup)
        {
            prevNoteNumber = noteGroup.prevNoteNumber;
            channel = noteGroup.channel;
            noteNumber = noteGroup.noteNumber;
            sendMidi = noteGroup.sendMidi;
            sendCV = noteGroup.sendCV;
            unknownLength = noteGroup.unknownLength;
        }

        MidiNoteGroup toMidiNoteGroup()
        {
            MidiNoteGroup noteGroup;
            noteGroup.channel = channel;
            noteGroup.prevNoteNumber = prevNoteNumber;
            noteGroup.noteNumber = noteNumber;
            noteGroup.sendCV = sendCV;
            noteGroup.sendMidi = sendMidi;
            noteGroup.unknownLength = unknownLength;
            return noteGroup;
        }
    };

    // void(*outNoteptr)(midifxnote); // out pointer type

    // typedef void (*MidiFXNoteFunction)(midifxnote);

    class MidiFXInterface
    {
    public:
        MidiFXInterface() {}
        virtual ~MidiFXInterface();

        virtual int getFXType() = 0;

        // Display name
        virtual const char* getName() = 0;

        // Short Display Name
        virtual const char* getDispName() = 0;

        virtual MidiFXInterface* getClone() { return nullptr;}

        // If returns true, midifx will use the keys
        // Recommend only using keys on specific pages
        virtual bool usesKeys() { return false; }
        virtual void onKeyUpdate(OMXKeypadEvent e, uint8_t funcKeyMode) {}
        virtual void onKeyHeldUpdate(OMXKeypadEvent e, uint8_t funcKeyMode) {}
        virtual void updateLEDs(uint8_t funcKeyMode) {}


        virtual void onModeChanged(){};

        virtual void setSelected(bool selected);

        virtual void setEnabled(bool newEnabled);
        virtual bool getEnabled();

        virtual void loopUpdate() {}

        virtual void onEncoderChanged(Encoder::Update enc);
        virtual void onEncoderButtonDown();

        virtual void onDisplayUpdate(uint8_t funcKeyMode) = 0;

        // Static glue to link a pointer to a member function
        static void onNoteInputForwarder(void *context, MidiNoteGroup note)
        {
            static_cast<MidiFXInterface *>(context)->noteInput(note);
        }

        virtual void noteInput(MidiNoteGroup note) = 0;
        // virtual MidiFXNoteFunction getInputFunc() = 0;
        virtual void setNoteOutput(void (*fptr)(void *, MidiNoteGroup), void *context);

        virtual int saveToDisk(int startingAddress, Storage *storage);
        virtual int loadFromDisk(int startingAddress, Storage *storage);

        // // the function using the function pointers:
        // void somefunction(void (*fptr)(void *, int, int), void *context)
        // {
        //     fptr(context, 17, 42);
        // }

    protected:
        bool enabled_;
        bool selected_;

        bool encoderSelect_;
        ParamManager params_;

        // std::vector<MidiNoteGroup> triggeredNotes; 

        void* outFunctionContext_;
        void (*outFunctionPtr_)(void *, MidiNoteGroup);

        virtual void onEnabled() {} // Called whenever entering mode
        virtual void onDisabled() {} // Called whenever entering mode

        virtual void onSelected() {} // Called whenever MidiFX group containing this MidiFX is selected
        virtual void onDeselected() {} // Called whenever MidiFX group containing this MidiFX is deselected

        virtual void onEncoderChangedSelectParam(Encoder::Update enc);
        virtual void onEncoderChangedEditParam(Encoder::Update enc) = 0;

        virtual void sendNoteOut(MidiNoteGroup note);

        virtual void sendNoteOff(MidiNoteGroupCache noteCache);
        virtual void sendNoteOff(MidiNoteGroup note);


        virtual void processNoteOn(uint8_t origNoteNumber, MidiNoteGroup note);
        virtual void processNoteOff(MidiNoteGroup note);

    };
}