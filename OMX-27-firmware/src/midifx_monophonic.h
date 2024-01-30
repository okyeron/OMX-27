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
        const char* getName() override;
        const char* getDispName() override;
        uint32_t getColor() override;

        MidiFXInterface* getClone() override;

        void loopUpdate() override;

        void onDisplayUpdate(uint8_t funcKeyMode) override;

        void noteInput(MidiNoteGroup note) override;
        // MidiFXNoteFunction getInputFunc() override;

        int saveToDisk(int startingAddress, Storage *storage) override;
        int loadFromDisk(int startingAddress, Storage *storage) override;

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
