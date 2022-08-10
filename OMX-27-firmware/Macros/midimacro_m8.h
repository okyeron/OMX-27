#pragma once
#include "midimacro_interface.h"

namespace midimacro
{
    class MidiMacroM8 : public MidiMacroInterface
    {
    public:
        MidiMacroM8();
        ~MidiMacroM8() {}

        bool consumesPots() override {return true;}
        bool consumesDisplay() override {return false;}

        String getName() override;

        void loopUpdate() override;

        void onDisplayUpdate() override;

        void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) override;
        void onKeyUpdate(OMXKeypadEvent e) override;
        void drawLEDs() override;
    protected:
        void onEnabled() override;
        void onDisabled() override;

        void onEncoderChangedEditParam(Encoder::Update enc) override;

    private:
    };
}
