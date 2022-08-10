#pragma once
#include "config.h"
// #include "ClearUI_Input.h"
#include "omx_keypad.h"
#include "param_manager.h"

namespace midifx
{
    int b = sizeof(MidiNoteGroup);
    class MidiFXNoteTracker
    {
    public:
        MidiFXNoteTracker() {}
        virtual ~MidiFXNoteTracker();

    private:

    };
}