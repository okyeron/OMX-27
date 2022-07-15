#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"
#include "retro_grids.h"
#include "colors.h"
#include "config.h"

class OmxModeGrids : public OmxModeInterface
{
public:
    OmxModeGrids();
    ~OmxModeGrids() {}

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

private:
    void setParam(int pageIndex, int paramPosition);
    void setParam(int paramIndex);
    void setupPageLegends();

    void updateLEDsFNone();
    void updateLEDsF1();
    void updateLEDsPatterns();

    void updateLEDsChannelView();
    void onKeyUpdateChanLock(OMXKeypadEvent e);

    void saveActivePattern(int pattIndex);
    void loadActivePattern(int pattIndex);



    bool initSetup = false;
    grids::GridsWrapper grids_;

    static const int kNumPages = 3;
    static const int kNumParams = kNumPages * NUM_DISP_PARAMS;
    static const int kNumGrids = 4;

    // static const int kParamGridX = 2;
    // static const int kParamGridY = 3;

    uint32_t paramSelColors[4] = {MAGENTA, ORANGE, RED, RBLUE};




    int page = 0;
    int param = 0;

    // int gridXKeyChannel = 0; // Gets set by holding first 0-3 keys on bottom
    // int gridYKeyChannel = 0; // Gets set by holding keys 4-7 on bottom

    // int gridsXY[4][2];

    bool gridsSelected[4] = {false,false,false,false};

    bool gridsAUX = false;

    bool f1_;
    bool f2_;
    bool f3_;
    bool fNone_;

    bool channelLockView_ = false;
    bool justLocked_ = false;
    int lockedChannel_ = 0;
    uint16_t chanLockHues_[4] = {300, 30, 0, 210};


    int prevResolution_ = 0;
};