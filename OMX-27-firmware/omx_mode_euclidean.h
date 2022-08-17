#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"
#include "colors.h"
#include "config.h"
#include "omx_mode_midi_keyboard.h"
#include "euclidean_sequencer.h"
#include "submode_midifxgroup.h"
#include "param_manager.h"


struct EuclidPatternSave
{
    euclidean::EuclidSave euclids[8];
    bool polyRhythmMode_ = true;
};

class OmxModeEuclidean : public OmxModeInterface
{
public:
    OmxModeEuclidean();
    ~OmxModeEuclidean() {}

    void InitSetup() override;

    void onModeActivated() override;

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

    static const u_int8_t kNumEuclids = 8;
    static const u_int8_t kNumSaves = 16;
    // static const u_int8_t kNumMidiFXGroups = 5;

    int saveToDisk(int startingAddress, Storage *storage);
    int loadFromDisk(int startingAddress, Storage *storage);


    // static int serializedPatternSize(bool eeprom);
    // static inline int getNumPatterns() { return 8; }
    // grids::SnapShotSettings* getPattern(uint8_t patternIndex);
    // void setPattern(uint8_t patternIndex, grids::SnapShotSettings snapShot);
private:
    // void setParam(uint8_t pageIndex, uint8_t paramPosition);
    // void setParam(uint8_t paramIndex);
    void setupPageLegends();

    void updateLEDsFNone();
    void updateLEDsF1();
    void updateLEDsPatterns();

    void updateLEDsChannelView();
    void onKeyUpdateChanLock(OMXKeypadEvent e);

    void saveActivePattern(uint8_t pattIndex, bool showMsg = true);
    void loadActivePattern(uint8_t pattIndex);

    void selectEuclid(uint8_t euclidIndex);

    bool initSetup = false;

    String tempString;

    static const uint8_t kNumPages = 4;
    static const uint8_t kNumParams = kNumPages * NUM_DISP_PARAMS;
    static const uint8_t kNumGrids = 4;

    uint32_t paramSelColors[4] = {MAGENTA, ORANGE, RED, RBLUE};

    const char * rateNames[3] = {"1 / 2", "1", "2"};

    ParamManager* getSelectedParamMode();
    void setParamMode(uint8_t newParamMode);
    void setPageAndParam(uint8_t pageIndex, uint8_t paramPosition, bool editParam);
    void setParam(uint8_t paramIndex);
    void onEncoderChangedSelectParam(Encoder::Update enc);

    bool encoderSelect_ = false;

    // ParamManager selEucParams;

    uint8_t paramMode_ = 0;

    ParamManager params_[3];

    uint8_t selectedEuclid_ = 0;

    EuclidPatternSave saveSlots_[kNumSaves];

    uint8_t selectedSave_ = 0;

    // int sizeSaves = sizeof(saveSlots_);

    // bool gridsSelected[4] = {false,false,false,false};

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

    euclidean::EuclideanSequencer euclids[kNumEuclids];

    // int esize = sizeof(euclids);

    // SubModes
    SubmodeInterface* activeSubmode = nullptr;

    // SubModeMidiFxGroup subModeMidiFx[kNumMidiFXGroups];

    void enableSubmode(SubmodeInterface* subMode);
    void disableSubmode();
    bool isSubmodeEnabled();

    // Static glue to link a pointer to a member function
    static void onPendingNoteOffForwarder(void *context, int note, int channel)
    {
        static_cast<OmxModeEuclidean *>(context)->onPendingNoteOff(note, channel);
    }

    void onPendingNoteOff(int note, int channel);

    // Static glue to link a pointer to a member function
    static void onNoteTriggeredForwarder(void *context, uint8_t euclidIndex, MidiNoteGroup note)
    {
        static_cast<OmxModeEuclidean *>(context)->onNoteTriggered(euclidIndex, note);
    }

    void onNoteTriggered(uint8_t euclidIndex, MidiNoteGroup note);

    // Static glue to link a pointer to a member function
    static void onNotePostFXForwarder(void *context, MidiNoteGroup note)
    {
        static_cast<OmxModeEuclidean *>(context)->onNotePostFX(note);
    }

    void onNotePostFX(MidiNoteGroup note);
};