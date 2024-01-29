#include "submode_interface.h"
#include "../../hardware/omx_disp.h"

void SubmodeInterface::setEnabled(bool newEnabled)
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
bool SubmodeInterface::isEnabled()
{
	return enabled_;
}

bool SubmodeInterface::getEncoderSelect()
{
	return encoderSelect_;
}

void SubmodeInterface::onEncoderChanged(Encoder::Update enc)
{
	if (getEncoderSelect())
	{
		onEncoderChangedSelectParam(enc);
	}
	else
	{
		onEncoderChangedEditParam(enc);
	}
}

// Handles selecting params using encoder
void SubmodeInterface::onEncoderChangedSelectParam(Encoder::Update enc)
{
	params_.changeParam(enc.dir());
	omxDisp.setDirty();
}

void SubmodeInterface::onEncoderButtonDown()
{
<<<<<<< HEAD:OMX-27-firmware/src/submode_interface.cpp
	encoderSelect_ = !encoderSelect_;
	omxDisp.setDirty();
=======
    encoderSelect_ = !encoderSelect_;
    omxDisp.setDirty();
>>>>>>> 5fe2be8 (File organization and includePath updates):OMX-27-firmware/src/modes/submodes/submode_interface.cpp
}
