#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"
#include "retro_grids.h"
#include "colors.h"
#include "config.h"
#include "omx_mode_midi_keyboard.h"
#include "param_manager.h"

class OmxModeGrids : public OmxModeInterface
{
public:
    OmxModeGrids();
    ~OmxModeGrids() {}

    void InitSetup() override;

    void onModeActivated() override;
    void onModeDeactivated() override;

    void onClockTick() override;

    void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) override;

    void loopUpdate(Micros elapsedTime) override;

    void updateLEDs() override;

    void onEncoderChanged(Encoder::Update enc) override;
    void onEncoderButtonDown() override;
    void onEncoderButtonDownLong() override;

    bool shouldBlockEncEdit() override;

    void onKeyUpdate(OMXKeypadEvent e) override;
    void onKeyHeldUpdate(OMXKeypadEvent e) override;

    void onDisplayUpdate() override;
    void SetScale(MusicScales* scale);

    static int serializedPatternSize(bool eeprom);
    static inline int getNumPatterns() { return 8; }
    grids::SnapShotSettings* getPattern(uint8_t patternIndex);
    void setPattern(uint8_t patternIndex, grids::SnapShotSettings snapShot);
private:
    void setPageAndParam(uint8_t pageIndex, uint8_t paramPosition);
    void setParam(uint8_t paramIndex);
    void setupPageLegends();

    void updateLEDsFNone();
    void updateLEDsF1();
    void updateLEDsPatterns();

    void updateLEDsChannelView();
    void onKeyUpdateChanLock(OMXKeypadEvent e);

    void saveActivePattern(uint8_t pattIndex);
    void loadActivePattern(uint8_t pattIndex);

    void quickSelectInst(uint8_t instIndex);

    void startPlayback();
    void stopPlayback();

    bool initSetup = false;
    grids::GridsWrapper grids_;

    // Static glue to link a pointer to a member function
    static void onNoteTriggeredForwarder(void *context, uint8_t gridsChannel, MidiNoteGroup note)
    {
        static_cast<OmxModeGrids *>(context)->onNoteTriggered(gridsChannel, note);
    }

    void onNoteTriggered(uint8_t gridsChannel, MidiNoteGroup note);

    // static const uint8_t kNumPages = 4;
    // static const uint8_t kNumParams = kNumPages * NUM_DISP_PARAMS;
    static const uint8_t kNumGrids = 4;

    // static const int kParamGridX = 2;
    // static const int kParamGridY = 3;

    uint32_t paramSelColors[4] = {MAGENTA, ORANGE, RED, RBLUE};

    const char * rateNames[3] = {"0.5x", "1x", "2x"};

    // If true, encoder selects param rather than modifies value
    bool encoderSelect = false;
    void onEncoderChangedSelectParam(Encoder::Update enc);
    ParamManager params;

    // int page = 0;
    // int param = 0;

    // int gridXKeyChannel = 0; // Gets set by holding first 0-3 keys on bottom
    // int gridYKeyChannel = 0; // Gets set by holding keys 4-7 on bottom

    // int gridsXY[4][2];

    bool gridsSelected[4] = {false,false,false,false};

    // Implements threshold post load to prevent pots from changing until modified
    bool potPostLoadThresh[5] = {false,false,false,false, false};

    bool isPlaying_ = false;

    bool gridsAUX = false;

    bool f1_;
    bool f2_;
    bool f3_;
    bool fNone_;

    bool instLockView_ = false;
    bool justLocked_ = false;
    int lockedInst_ = 0;
    uint16_t instLockHues_[4] = {300, 30, 0, 210};


    int prevResolution_ = 0;

    bool midiModeception = false;
    OmxModeMidiKeyboard midiKeyboard; // Mode inside a mode. For science!

    String legendTemp;
    String xTemp;
    String yTemp;
};
