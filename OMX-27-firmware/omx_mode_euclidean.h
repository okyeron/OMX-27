#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"
#include "colors.h"
#include "config.h"
#include "omx_mode_midi_keyboard.h"
#include "euclidean_sequencer.h"

class OmxModeEuclidean : public OmxModeInterface
{
public:
    OmxModeEuclidean();
    ~OmxModeEuclidean() {}

    void InitSetup() override;

    void onModeActivated() override;

    void onClockTick() override;

    void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) override;

    void loopUpdate() override;

    void updateLEDs() override;

    void onEncoderChanged(Encoder::Update enc) override;
    void onEncoderButtonDown() override;
    void onEncoderButtonDownLong() override;

    bool shouldBlockEncEdit() override;

    void onKeyUpdate(OMXKeypadEvent e) override;
    void onKeyHeldUpdate(OMXKeypadEvent e) override;

    void onDisplayUpdate() override;
    void SetScale(MusicScales* scale);

    static const u_int8_t kNumEuclids = 4;

    // static int serializedPatternSize(bool eeprom);
    // static inline int getNumPatterns() { return 8; }
    // grids::SnapShotSettings* getPattern(uint8_t patternIndex);
    // void setPattern(uint8_t patternIndex, grids::SnapShotSettings snapShot);
private:
    void setParam(uint8_t pageIndex, uint8_t paramPosition);
    void setParam(uint8_t paramIndex);
    void setupPageLegends();

    void updateLEDsFNone();
    void updateLEDsF1();
    void updateLEDsPatterns();

    void updateLEDsChannelView();
    void onKeyUpdateChanLock(OMXKeypadEvent e);

    void saveActivePattern(uint8_t pattIndex);
    void loadActivePattern(uint8_t pattIndex);

    void selectEuclid(uint8_t euclidIndex);

    bool initSetup = false;

    static const uint8_t kNumPages = 4;
    static const uint8_t kNumParams = kNumPages * NUM_DISP_PARAMS;
    static const uint8_t kNumGrids = 4;

    uint32_t paramSelColors[4] = {MAGENTA, ORANGE, RED, RBLUE};

    const char * rateNames[3] = {"1 / 2", "1", "2"};

    int page = 0;
    int param = 0;

    uint8_t selectedEuclid_ = 0;

    bool gridsSelected[4] = {false,false,false,false};

    bool aux_ = false;

    bool f1_;
    bool f2_;
    bool f3_;
    bool fNone_;

    bool midiModeception = false;
    OmxModeMidiKeyboard midiKeyboard; // Mode inside a mode. For science!


    bool euclidPattern[32];
    bool polyRhythmMode = false;

    // u_int8_t rotation;
    // u_int8_t events;
    // u_int8_t steps;

    // void drawEuclidPattern(bool* pattern, uint8_t steps);

    // void printEuclidPattern(bool* pattern, uint8_t steps);

    void startSequencers();
    void stopSequencers();

    euclidean::EuclideanSequencer euclids[4];

};