#pragma once

#include "midimacro_interface.h"

namespace midimacro
{

    class MidiMacroNorns : public MidiMacroInterface
    {
    public:
        MidiMacroNorns();
        ~MidiMacroNorns() {}

        bool consumesPots() override {return false;}
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
