#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"
#include "param_manager.h"
#include "submode_midifxgroup.h"

class OmxModeMidiKeyboard : public OmxModeInterface
{
public:
    OmxModeMidiKeyboard();
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

    void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) override;
    void loopUpdate() override;

    void updateLEDs() override;

    void onEncoderChanged(Encoder::Update enc) override;
    void onEncoderButtonDown() override;
    void onEncoderButtonUp() override;

    void onEncoderButtonDownLong() override;

    bool shouldBlockEncEdit() override;

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

    void onKeyUpdateM8Macro(OMXKeypadEvent e);

    // If true, encoder selects param rather than modifies value
    bool encoderSelect = false;
    void onEncoderChangedSelectParam(Encoder::Update enc);
    ParamManager params;

    // SubModes
    SubmodeInterface* activeSubmode = nullptr;
    SubModeMidiFxGroup subModeMidiFx;

    void enableSubmode(SubmodeInterface* subMode);
    void disableSubmode();
    bool isSubmodeEnabled();

    // // Static glue to link a pointer to a member function
    // static void onNoteTriggeredForwarder(void *context, uint8_t euclidIndex, MidiNoteGroup note)
    // {
    //     static_cast<OmxModeMidiKeyboard *>(context)->onNoteTriggered(euclidIndex, note);
    // }

    void doNoteOn(uint8_t keyIndex);
    void doNoteOff(uint8_t keyIndex);

    // void onNoteTriggered(MidiNoteGroup note);
    // void onNoteOffTriggered(MidiNoteGroup note);

    // Static glue to link a pointer to a member function
    static void onNotePostFXForwarder(void *context, MidiNoteGroup note)
    {
        static_cast<OmxModeMidiKeyboard *>(context)->onNotePostFX(note);
    }

    void onNotePostFX(MidiNoteGroup note);
};