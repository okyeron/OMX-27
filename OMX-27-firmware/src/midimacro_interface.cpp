#include "midimacro_interface.h"
#include "omx_disp.h"

namespace midimacro
{
    MidiMacroInterface::~MidiMacroInterface()
    {
        // std::vector<MidiNoteGroup>().swap(triggeredNotes);
        // Serial.println("Deleted vector");
    }

    void MidiMacroInterface::setEnabled(bool newEnabled)
    {
        enabled_ = newEnabled;
        if (enabled_)
        {
            onEnabled();
        }
        else
        {
            onDisabled();
        }
    }

    bool MidiMacroInterface::getEnabled()
    {
        return enabled_;
    }

    void MidiMacroInterface::onEncoderChanged(Encoder::Update enc)
    {
        if (encoderSelect_)
        {
            onEncoderChangedSelectParam(enc);
        }
        else
        {
            onEncoderChangedEditParam(enc);
        }
    }

    // Handles selecting params using encoder
    void MidiMacroInterface::onEncoderChangedSelectParam(Encoder::Update enc)
    {
        params_.changeParam(enc.dir());
        omxDisp.setDirty();
    }

    void MidiMacroInterface::onEncoderButtonDown()
    {
        encoderSelect_ = !encoderSelect_;
        omxDisp.setDirty();
    }

    void MidiMacroInterface::setScale(MusicScales* scale)
    {
        scale_ = scale;
    }

    void MidiMacroInterface::setDoNoteOn(void (*fptr)(void *, uint8_t), void *context)
    {
        doNoteOnFptrContext_ = context;
        doNoteOnFptr_ = fptr;
    }

    void MidiMacroInterface::setDoNoteOff(void (*fptr)(void *, uint8_t), void *context)
    {
        doNoteOffFptrContext_ = context;
        doNoteOffFptr_ = fptr;
    }

    void MidiMacroInterface::DoNoteOn(uint8_t keyIndex)
    {
        if (doNoteOnFptrContext_ != nullptr)
        {
            doNoteOnFptr_(doNoteOnFptrContext_, keyIndex);
        }
    }

    void MidiMacroInterface::DoNoteOff(uint8_t keyIndex)
    {
        if (doNoteOffFptrContext_ != nullptr)
        {
            doNoteOffFptr_(doNoteOffFptrContext_, keyIndex);
        }
    }
}