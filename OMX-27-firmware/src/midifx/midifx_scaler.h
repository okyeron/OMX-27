#pragma once

#include "midifx_interface.h"

namespace midifx
{

	class MidiFXScaler : public MidiFXInterface
	{
	public:
		MidiFXScaler();
		~MidiFXScaler() {}

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

<<<<<<< HEAD:OMX-27-firmware/src/midifx_scaler.h
	private:
		// std::vector<MidiNoteGroup> triggeredNotes;
=======
    private:
        // std::vector<MidiNoteGroup> triggeredNotes;
>>>>>>> 5fe2be8 (File organization and includePath updates):OMX-27-firmware/src/midifx/midifx_scaler.h

		uint8_t chancePerc_ = 100;

		bool useGlobalScale_ = true;

		int8_t rootNote_ = 0;
		int8_t scaleIndex_ = 0;

		int8_t scaleRemapper[12];

		void calculateRemap();

		// MidiNoteGroup findTriggeredNote(uint8_t noteNumber);
	};
}
