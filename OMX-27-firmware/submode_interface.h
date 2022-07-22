#pragma once
#include "ClearUI_Input.h"
#include "omx_keypad.h"
#include "param_manager.h"

// defines interface for a submode, a mode within a mode
class SubmodeInterface
{
public:
    SubmodeInterface() {}
    virtual ~SubmodeInterface() {}

    virtual void setEnabled(bool newEnabled);
    virtual bool isEnabled();

    virtual void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) {}
    virtual void loopUpdate() {}
    virtual void updateLEDs() {}
    virtual void onEncoderChanged(Encoder::Update enc);
    virtual void onEncoderButtonDown();

    virtual bool shouldBlockEncEdit() { return false; }

    virtual void onKeyUpdate(OMXKeypadEvent e) {}

    virtual void onDisplayUpdate() = 0;

    virtual bool usesPots() { return false; } // return true if submode uses pots

protected:
    bool enabled_;
    bool encoderSelect_;
    ParamManager params_;

    virtual void onEnabled() {}   // Called whenever entering mode
    virtual void onDisabled() {} // Called whenever exiting mode

    virtual void onEncoderChangedSelectParam(Encoder::Update enc);
    virtual void onEncoderChangedEditParam(Encoder::Update enc) = 0;
};