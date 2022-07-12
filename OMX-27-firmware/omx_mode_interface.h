#pragma once
#include "ClearUI_Input.h"
#include "omx_keypad.h"

class OmxModeInterface
{
public:
    OmxModeInterface() {}
    virtual ~OmxModeInterface() {}

    virtual void OnPotChanged(int potIndex, int potValue) = 0;
    virtual void updateLEDs() = 0;
    virtual void onEncoderChanged(Encoder::Update enc) = 0;
    virtual void onEncoderButtonDown() = 0;
    virtual void onEncoderButtonUp() {};
    virtual void onEncoderButtonUpLong() {};

    virtual bool shouldBlockEncEdit() { return false; } // return true if should block encoder mode switch / hold down encoder
    virtual void onEncoderButtonDownLong() = 0; // Will only get called if shouldBlockEncEdit() returns true

    virtual void onKeyUpdate(OMXKeypadEvent e) = 0;

    virtual void onKeyHeldUpdate(OMXKeypadEvent e) {};

    virtual void onDisplayUpdate() {};
};