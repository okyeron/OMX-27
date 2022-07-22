#pragma once

#include "midifx_interface.h"

namespace midifx
{

    class MidiFXRandomNote : public MidiFXInterface
    {
    public:
        MidiFXRandomNote();
        ~MidiFXRandomNote() {}

        int getFXType() override;
        String getName() override;

        void loopUpdate() override;

        void onDisplayUpdate() override;

        void noteInput(midifxnote note) override;

    protected:
        void onEnabled() override;
        void onDisabled() override;

        void onEncoderChangedEditParam(Encoder::Update enc) override;
    };
}
