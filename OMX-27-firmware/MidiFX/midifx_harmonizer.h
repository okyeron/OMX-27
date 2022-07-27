#pragma once

#include "midifx_interface.h"

namespace midifx
{

    class MidiFXHarmonizer : public MidiFXInterface
    {
    public:
        MidiFXHarmonizer();
        ~MidiFXHarmonizer() {}

        int getFXType() override;
        String getName() override;

        void loopUpdate() override;

        void onDisplayUpdate() override;

        void noteInput(MidiNoteGroup note) override;

    protected:
        void onEnabled() override;
        void onDisabled() override;

        void onEncoderChangedEditParam(Encoder::Update enc) override;
    private:
        // std::vector<MidiNoteGroup> triggeredNotes; 
        bool playOrigin_ = true;

        int8_t notes_[7];

        uint8_t chancePerc_ = 100;

        String tempString_ = "12345";
        // String tempStringVal_ = "12345";

    };
}
