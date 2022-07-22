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

        virtual int getFXType() = 0;

        // Display name
        virtual String getName() = 0;

        virtual void setEnabled(bool newEnabled);
        virtual bool getEnabled();

        virtual void loopUpdate() {}

        virtual void onEncoderChanged(Encoder::Update enc);
        virtual void onEncoderButtonDown();

        virtual void onDisplayUpdate() = 0;

        virtual void noteInput(midifxnote note) = 0;

    protected:
        bool enabled_;
        bool encoderSelect_;
        ParamManager params_;

        virtual void onEnabled() {} // Called whenever entering mode
        virtual void onDisabled() {} // Called whenever entering mode

        virtual void onEncoderChangedSelectParam(Encoder::Update enc);
        virtual void onEncoderChangedEditParam(Encoder::Update enc) = 0;
    };
}