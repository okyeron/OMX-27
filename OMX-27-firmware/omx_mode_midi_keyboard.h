#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"

class OmxModeMidiKeyboard : public OmxModeInterface
{
public:
    OmxModeMidiKeyboard() {}
    ~OmxModeMidiKeyboard() {}

    void InitSetup() override;
    void onModeActivated() override;

    void setOrganelleMode()
    {
        organelleMotherMode = true;
    }

    void setMidiMode()
    {
        organelleMotherMode = false;
    }

    void onPotChanged(int potIndex, int potValue) override;

    void updateLEDs() override;

    void onEncoderChanged(Encoder::Update enc) override;
    void onEncoderButtonDown() override;
    void onEncoderButtonUp() override;

    void onEncoderButtonDownLong() override;

    void onKeyUpdate(OMXKeypadEvent e) override;
    void onKeyHeldUpdate(OMXKeypadEvent e){};

    void onDisplayUpdate() override;
    void inMidiNoteOn(byte channel, byte note, byte velocity) override;
    void inMidiNoteOff(byte channel, byte note, byte velocity) override;

    void SetScale(MusicScales* scale);
    
private:
    bool initSetup = false;
    bool organelleMotherMode = false; // TODO make separate class for this

    MusicScales* musicScale;

    void changePage(int amt);
    void setParam(int paramIndex);

};