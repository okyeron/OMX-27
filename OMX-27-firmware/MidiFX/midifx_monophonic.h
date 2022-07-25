#pragma once

#include "midifx_interface.h"

namespace midifx
{

    // Forces Monophonic output, one note at a time
    class MidiFXMonophonic : public MidiFXInterface
    {
    public:
        MidiFXMonophonic();
        ~MidiFXMonophonic() {}

        int getFXType() override;
        String getName() override;

        void loopUpdate() override;

        void onDisplayUpdate() override;

        void noteInput(MidiNoteGroup note) override;
        // MidiFXNoteFunction getInputFunc() override;

    protected:
        void onEnabled() override;
        void onDisabled() override;

        void onEncoderChangedEditParam(Encoder::Update enc) override;

    private:
        uint8_t chancePerc_ = 100;

        // 16 midi channels
        MidiNoteGroupCache prevNoteOn[16];
    };
}
