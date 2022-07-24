#pragma once
#include "config.h"
#include "ClearUI_Input.h"
#include "omx_keypad.h"
#include "param_manager.h"

namespace midifx
{
    

    // void(*outNoteptr)(midifxnote); // out pointer type

    // typedef void (*MidiFXNoteFunction)(midifxnote);

    class MidiFXInterface
    {
    public:
        MidiFXInterface() {}
        virtual ~MidiFXInterface() {}

        virtual int getFXType() = 0;

        // Display name
        virtual String getName() = 0;

        virtual void setEnabled(bool newEnabled);
        virtual bool getEnabled();

        virtual void loopUpdate() {}

        virtual void onEncoderChanged(Encoder::Update enc);
        virtual void onEncoderButtonDown();

        virtual void onDisplayUpdate() = 0;

        // Static glue to link a pointer to a member function
        static void onNoteInputForwarder(void *context, MidiNoteGroup note)
        {
            static_cast<MidiFXInterface *>(context)->noteInput(note);
        }

        virtual void noteInput(MidiNoteGroup note) = 0;
        // virtual MidiFXNoteFunction getInputFunc() = 0;
        virtual void setNoteOutput(void (*fptr)(void *, MidiNoteGroup), void *context);

        // // the function using the function pointers:
        // void somefunction(void (*fptr)(void *, int, int), void *context)
        // {
        //     fptr(context, 17, 42);
        // }

    protected:
        bool enabled_;
        bool encoderSelect_;
        ParamManager params_;

        std::vector<MidiNoteGroup> triggeredNotes; 

        void* outFunctionContext_;
        void (*outFunctionPtr_)(void *, MidiNoteGroup);

        virtual void onEnabled() {} // Called whenever entering mode
        virtual void onDisabled() {} // Called whenever entering mode

        virtual void onEncoderChangedSelectParam(Encoder::Update enc);
        virtual void onEncoderChangedEditParam(Encoder::Update enc) = 0;

        virtual void sendNoteOut(MidiNoteGroup note);

        virtual void processNoteOn(uint8_t origNoteNumber, MidiNoteGroup note);
        virtual void processNoteOff(MidiNoteGroup note);

    };
}