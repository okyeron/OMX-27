#pragma once

#include "midifx_interface.h"

namespace midifx
{

	class MidiFXTranspose : public MidiFXInterface
	{
	public:
		MidiFXTranspose();
		~MidiFXTranspose() {}

		int getFXType() override;
		const char *getName() override;
		const char *getDispName() override;

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
		struct TransposeSave
		{
			int8_t transpose : 6;
			int8_t octave : 4;
			uint8_t chancePerc_ = 100;
		};

		int8_t transpose_ : 6;
		int8_t octave_ : 4;

		uint8_t chancePerc_ = 100;
	};
}
