#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"
#include "param_manager.h"

class OmxModeSequencer : public OmxModeInterface
{
public:
    OmxModeSequencer();
    ~OmxModeSequencer() {}

    void InitSetup() override;

    void initPatterns(); // Initializes all patterns

    void onModeActivated() override;

    void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) override;

    void loopUpdate(Micros elapsedTime) override;

    // Should be part of LED update, intertangled with the sequencer class which is calling it in main FW code.
    void showCurrentStepLEDs(int patternNum);

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

    // If true, encoder selects param rather than modifies value
    bool encoderSelect_ = false;

    bool patternParams_ = false;
    bool seqPages_ = false; // True when we can change page selection

    bool noteSelect_ = false;
    // bool noteSelection_ = false; // noteSelection_ is never set false when in noteSelect_ mode, so see no reason for it. seems to be remnant of some other feature. 

    // bool stepSelect_ = false; // Only used in noteSelection after selecting a key, it is set false, value never checked, see no reason for it.
    bool stepRecord_ = false;
    bool stepDirty_ = false;


    ParamManager seqParams; // seq params, 2 pages
    ParamManager noteSelParams; // note select params, 3 pages
    ParamManager patParams; // pattern params, 3 pages
    ParamManager sRecParams; // step record params, 2 pages

    // void setParam(uint8_t pageIndex, uint8_t paramPosition);
    // void setParam(uint8_t paramIndex);

    void onEncoderChangedNorm(Encoder::Update enc);
    void onEncoderChangedStep(Encoder::Update enc);

    void onEncoderChangedSelectParam(Encoder::Update enc);

    uint8_t getAdjustedNote(uint8_t keyNumber);

    void changeSequencerMode(uint8_t newMode);
    uint8_t getSequencerMode(); // based on enum SequencerMode in cpp file

    void pasteStep(uint8_t stepKey);

};