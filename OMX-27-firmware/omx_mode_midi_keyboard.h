#pragma once

#include "omx_mode_interface.h"

class OmxModeMidiKeyboard : public OmxModeInterface
{
public:
    OmxModeMidiKeyboard() {}
    ~OmxModeMidiKeyboard() {}

    void OnPotChanged(int potIndex, int potValue) override;

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

    void setOrganelleMode()
    {
        organelleMotherMode = true;
    }

private:
    bool organelleMotherMode = false; // TODO make separate class for this
};