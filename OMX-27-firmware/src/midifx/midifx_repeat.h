#pragma once

#include "midifx_interface.h"

namespace midifx
{

	class MidiFXRepeat : public MidiFXInterface
	{
	public:
		MidiFXRepeat();
		~MidiFXRepeat() {}

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
		struct RepeatSave
		{
			uint8_t chancePerc : 7;
            uint8_t numOfRepeats : 4; 
            uint8_t mode : 3;
            int8_t rateIndex : 5;
            uint8_t rateHz;	
            uint8_t gate : 8;
            uint8_t velStart : 7;
            uint8_t velEnd : 7;
        };
		uint8_t chancePerc_ = 100;

		uint8_t numOfRepeats_ : 4; // 1 to 16, stored as 0 - 15
        uint8_t mode_ : 3; // Off, 1-Shot - Repeats for numOfRepeats_ restarts on new note on, On - Repeats indefinitely while key is hold, Hold - Endlessly repeats, 
        int8_t rateIndex_ : 5; // max 15 or -1 for hz
        uint8_t rateHz_;	
        uint8_t gate_ : 8; // 0-200
        uint8_t velStart_ : 7; // 0-127
        uint8_t velEnd_ : 7; // 0-127
	};
}
