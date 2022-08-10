#pragma once
#include "config.h"
#include "ClearUI_Input.h"
#include "omx_keypad.h"
#include "param_manager.h"

namespace midimacro
{
    class MidiMacroInterface
    {
    public:
        MidiMacroInterface() {}
        virtual ~MidiMacroInterface();

        // Return true if consumes pots
        virtual bool consumesPots() = 0;

        // Return true if consumes display / encoder
        virtual bool consumesDisplay() = 0;

        // Display name
        virtual String getName() = 0;

        virtual void setEnabled(bool newEnabled);
        virtual bool getEnabled();

        virtual void loopUpdate() {}

        virtual void onEncoderChanged(Encoder::Update enc);
        virtual void onEncoderButtonDown();

        virtual void onDisplayUpdate() = 0;

        virtual void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) = 0;
        virtual void onKeyUpdate(OMXKeypadEvent e) = 0;
        virtual void drawLEDs() = 0;

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