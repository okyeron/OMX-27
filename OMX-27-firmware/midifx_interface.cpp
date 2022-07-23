#include "midifx_interface.h"
#include "omx_disp.h"

namespace midifx
{
    void MidiFXInterface::setEnabled(bool newEnabled)
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

    bool MidiFXInterface::getEnabled()
    {
        return enabled_;
    }

    void MidiFXInterface::onEncoderChanged(Encoder::Update enc)
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
    void MidiFXInterface::onEncoderChangedSelectParam(Encoder::Update enc)
    {
        params_.changeParam(enc.dir());
        omxDisp.setDirty();
    }

    void MidiFXInterface::onEncoderButtonDown()
    {
        encoderSelect_ = !encoderSelect_;
        omxDisp.setDirty();
    }

    void MidiFXInterface::setNoteOutput(void (*fptr)(void *, MidiNoteGroup), void *context)
    {
        outFunctionContext_ = context;
        outFunctionPtr_ = fptr;
    }

    void MidiFXInterface::sendNoteOut(MidiNoteGroup note)
    {
        if(outFunctionContext_ != nullptr){
            outFunctionPtr_(outFunctionContext_, note);
        }
    }
}