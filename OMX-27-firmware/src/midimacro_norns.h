#pragma once
#include "midimacro_interface.h"

namespace midimacro
{
    class MidiMacroNorns : public MidiMacroInterface
    {
    public:
        MidiMacroNorns();
        ~MidiMacroNorns() {}

        bool consumesPots() override {return true;}
        bool consumesDisplay() override {return true;}

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
        bool m8mutesolo_[16];

        // Control key mappings
        uint8_t keyUp_ = 1;
        uint8_t keyDown_ = 12;
        uint8_t keyLeft_ = 11;
        uint8_t keyRight_ = 13;

        uint8_t but1_ = 3;
        uint8_t but2_ = 14;
        uint8_t but3_ = 15;

        uint8_t enc1_ = 5;
        uint8_t enc2_ = 16;
        uint8_t enc3_ = 17;

        uint8_t ccBut1_ = 85;
        uint8_t ccBut2_ = 87;
        uint8_t ccBut3_ = 88;

        uint8_t ccEnc1_ = 58;
        uint8_t ccEnc2_ = 62;
        uint8_t ccEnc3_ = 63;
    };
}
