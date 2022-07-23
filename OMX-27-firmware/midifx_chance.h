#pragma once

#include "midifx_interface.h"

namespace midifx
{

    class MidiFXChance : public MidiFXInterface
    {
    public:
        MidiFXChance();
        ~MidiFXChance() {}

        int getFXType() override;
        String getName() override;

        void loopUpdate() override;

        void onDisplayUpdate() override;

        void noteInput(midifxnote note) override;
        // MidiFXNoteFunction getInputFunc() override;

    protected:
        void onEnabled() override;
        void onDisabled() override;

        void onEncoderChangedEditParam(Encoder::Update enc) override;

    private:
        uint8_t chancePerc_ = 255;
    };
}
