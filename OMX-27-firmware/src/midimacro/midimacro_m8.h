#pragma once
#include "midimacro_interface.h"

namespace midimacro
{
	class MidiMacroM8 : public MidiMacroInterface
	{
	public:
		MidiMacroM8();
		~MidiMacroM8() {}

		bool consumesPots() override { return true; }
		bool consumesDisplay() override { return true; }

		String getName() override;

		void loopUpdate() override;

		void onDisplayUpdate() override;

		void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) override;
		void onEncoderButtonDown() override;
		void onKeyUpdate(OMXKeypadEvent e) override;
		void drawLEDs() override;

	protected:
		void onEnabled() override;
		void onDisabled() override;

		void onEncoderChangedEditParam(Encoder::Update enc) override;

	private:
		bool m8mutesolo_[16];

		// M8PAGE_CONTROL key mappings
		uint8_t keyUp_ = 1;
		uint8_t keyDown_ = 12;
		uint8_t keyLeft_ = 11;
		uint8_t keyRight_ = 13;

		uint8_t keyOption_ = 4;
		uint8_t keyEdit_ = 5;
		uint8_t keyShift_ = 16;
		uint8_t keyPlay_ = 17;
	};
}
