#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"
#include "retro_grids.h"

class OmxModeGrids : public OmxModeInterface
{
public:
    OmxModeGrids();
    ~OmxModeGrids() {}

    void InitSetup() override;

    void onModeActivated() override;

    void onClockTick() override;

    void onPotChanged(int potIndex, int potValue) override;

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
    void setupPageLegends();


    bool initSetup = false;
    grids::GridsWrapper grids_;

    const int kNumPages = 2;
    const int kNumParams = 10;

    int page = 0;
    int param = 0;

    // int gridXKeyChannel = 0; // Gets set by holding first 0-3 keys on bottom
    // int gridYKeyChannel = 0; // Gets set by holding keys 4-7 on bottom

    int gridsXY[4][2];

    bool gridsSelected[4] = {false,false,false,false};

    bool gridsAUX = false;
};