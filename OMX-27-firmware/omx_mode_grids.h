#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"
#include "retro_grids.h"

class OmxModeGrids : public OmxModeInterface
{
public:
    OmxModeGrids() {}
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
    bool initSetup = false;
    grids::GridsWrapper grids_;
};