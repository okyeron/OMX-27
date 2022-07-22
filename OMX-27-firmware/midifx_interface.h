#pragma once
#include "ClearUI_Input.h"
#include "omx_keypad.h"
#include "param_manager.h"

namespace midifx
{
    struct midifxnote
    {
        uint8_t noteNumber;
        uint8_t velocity;
    };

    class MidiFXInterface
    {
    public:
        MidiFXInterface() {}
        virtual ~MidiFXInterface() {}

        virtual void onActivate() {} // Called whenever entering mode

        virtual void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) {}
        virtual void loopUpdate() {}
        virtual void updateLEDs() {}
        virtual void onEncoderChanged(Encoder::Update enc);
        virtual void onEncoderButtonDown();

        virtual bool shouldBlockEncEdit() { return false; }

        virtual void onKeyUpdate(OMXKeypadEvent e) {}

        virtual void onDisplayUpdate(){};

        virtual void noteInput(midifxnote note) = 0;

    protected:
        bool encoderSelect_;
        ParamManager params_;

        void onEncoderChangedSelectParam(Encoder::Update enc);
    };
}