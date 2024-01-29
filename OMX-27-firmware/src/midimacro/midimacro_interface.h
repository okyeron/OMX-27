#pragma once
#include "../config.h"
#include "../ClearUI/ClearUI_Input.h"
#include "../hardware/omx_keypad.h"
#include "../utils/param_manager.h"
#include "../utils/music_scales.h"

namespace midimacro
{
	class MidiMacroInterface
	{
	public:
		MidiMacroInterface() {}
		virtual ~MidiMacroInterface();

		// Return true if consumes pots
		virtual bool consumesPots() = 0;

		// Return true if consumes display / encoder
		virtual bool consumesDisplay() = 0;

		// Display name
		virtual String getName() = 0;

		virtual void setEnabled(bool newEnabled);
		virtual bool getEnabled();

		virtual void loopUpdate() {}

		virtual void onEncoderChanged(Encoder::Update enc);
		virtual void onEncoderButtonDown();

		virtual void onDisplayUpdate() = 0;

		virtual void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) = 0;
		virtual void onKeyUpdate(OMXKeypadEvent e) = 0;
		virtual void drawLEDs() = 0;

		virtual void setScale(MusicScales *scale);

		virtual void setDoNoteOn(void (*fptr)(void *, uint8_t), void *context);
		virtual void setDoNoteOff(void (*fptr)(void *, uint8_t), void *context);

	protected:
		bool enabled_;
		bool encoderSelect_;
		ParamManager params_;

		MusicScales *scale_;

		void *doNoteOnFptrContext_;
		void (*doNoteOnFptr_)(void *, uint8_t);

		void *doNoteOffFptrContext_;
		void (*doNoteOffFptr_)(void *, uint8_t);

		virtual void onEnabled() {}	 // Called whenever entering mode
		virtual void onDisabled() {} // Called whenever entering mode

		virtual void onEncoderChangedSelectParam(Encoder::Update enc);
		virtual void onEncoderChangedEditParam(Encoder::Update enc) = 0;

<<<<<<< HEAD:OMX-27-firmware/src/midimacro_interface.h
		virtual void DoNoteOn(uint8_t keyIndex);
		virtual void DoNoteOff(uint8_t keyIndex);
	};
=======
        virtual void DoNoteOn(uint8_t keyIndex);
        virtual void DoNoteOff(uint8_t keyIndex);
    };
>>>>>>> 5fe2be8 (File organization and includePath updates):OMX-27-firmware/src/midimacro/midimacro_interface.h
}
