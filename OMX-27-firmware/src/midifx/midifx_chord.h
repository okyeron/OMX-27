#pragma once

#include "midifx_interface.h"

namespace midifx
{

	class MidiFXChord : public MidiFXInterface
	{
	public:
		MidiFXChord();
		~MidiFXChord() {}

		int getFXType() override;
		const char *getName() override;
		const char *getDispName() override;
		uint32_t getColor() override;

		MidiFXInterface *getClone() override;

		void loopUpdate() override;

		void onDisplayUpdate(uint8_t funcKeyMode) override;

		void noteInput(MidiNoteGroup note) override;

		int saveToDisk(int startingAddress, Storage *storage) override;
		int loadFromDisk(int startingAddress, Storage *storage) override;

	protected:
		void onEnabled() override;
		void onDisabled() override;

		void onEncoderChangedEditParam(Encoder::Update enc) override;

	private:
		uint8_t chancePerc_ = 255;
	};
}
