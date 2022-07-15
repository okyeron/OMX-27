#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"

class OmxModeSequencer : public OmxModeInterface
{
public:
    OmxModeSequencer() {}
    ~OmxModeSequencer() {}

    void InitSetup() override;

    void initPatterns(); // Initializes all patterns

    void onModeActivated() override;

    void onPotChanged(int potIndex, int prevValue, int newValue) override;

    void loopUpdate() override;

    // Should be part of LED update, intertangled with the sequencer class which is calling it in main FW code.
    void showCurrentStep(int patternNum);

    void updateLEDs() override;

    void onEncoderChanged(Encoder::Update enc) override;
    void onEncoderButtonDown() override;
    void onEncoderButtonDownLong() override;

    bool shouldBlockEncEdit() override;

    void onKeyUpdate(OMXKeypadEvent e) override;
    void onKeyHeldUpdate(OMXKeypadEvent e) override;

    void onDisplayUpdate() override;

    void setSeq1Mode()
    {
        seq2Mode = false;
    }

    void setSeq2Mode()
    {
        seq2Mode = true;
    }

    void SetScale(MusicScales* scale);

private:
    bool initSetup = false;
    bool seq2Mode = false;

    MusicScales* musicScale;

    // These do not appear to be used
    // bool copiedFlag = false;
    // bool pastedFlag = false;
    // bool clearedFlag = false;

    void onEncoderChangedNorm(Encoder::Update enc);
    void onEncoderChangedStep(Encoder::Update enc);

};