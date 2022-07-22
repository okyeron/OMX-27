#include "midifx_interface.h"
#include "omx_disp.h"

namespace midifx
{
    void MidiFXInterface::onEncoderChanged(Encoder::Update enc)
    {
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
        omxDisp.isDirty();
    }
}