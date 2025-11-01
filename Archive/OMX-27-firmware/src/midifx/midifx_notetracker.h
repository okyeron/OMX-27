#pragma once
#include "../config.h"
// #include "../ClearUI/ClearUI_Input.h"
#include "../hardware/omx_keypad.h"
#include "../utils/param_manager.h"

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
