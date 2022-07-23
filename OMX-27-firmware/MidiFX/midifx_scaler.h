#pragma once

#include "midifx_interface.h"

namespace midifx
{

    class MidiFXScaler : public MidiFXInterface
    {
    public:
        MidiFXScaler();
        ~MidiFXScaler() {}

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
        uint8_t chancePerc_ = 255;
    };
}
