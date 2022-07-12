#pragma once

#include "omx_mode_interface.h"

class OmxModeSequencer : public OmxModeInterface
{
public:
    OmxModeSequencer() {}
    ~OmxModeSequencer() {}

    void InitSetup() override;

    void OnPotChanged(int potIndex, int potValue) override;

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

    void setSeq2Mode()
    {
        seq2Mode = true;
    }

private:
    bool seq2Mode = false;
    bool patternParams = false;
    bool seqPages = false;

    bool noteSelect = false;
    bool noteSelection = false;

    int selectedNote = 0;
    int selectedStep = 0;
    bool stepSelect = false;
    bool stepRecord = false;
    bool stepDirty = false;

    int pppage = 0;
    int sqpage = 0;
    int srpage = 0;

    int nsparam = 0; // note select params
    int ppparam = 0; // pattern params
    int sqparam = 0; // seq params
    int srparam = 0; // step record params

    // These do not appear to be used
    bool copiedFlag = false;
    bool pastedFlag = false;
    bool clearedFlag = false;

    void onEncoderChangedNorm(Encoder::Update enc);
    void onEncoderChangedStep(Encoder::Update enc);

    void initPatterns();
};