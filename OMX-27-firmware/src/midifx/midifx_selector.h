#pragma once

#include "midifx_interface.h"

namespace midifx
{

	class MidiFXSelector : public MidiFXInterface
	{
	public:
		MidiFXSelector();
		~MidiFXSelector() {}

		int getFXType() override;
		const char *getName() override;
		const char *getDispName() override;

		MidiFXInterface *getClone() override;

		void loopUpdate() override;

		void onDisplayUpdate(uint8_t funcKeyMode) override;

		void noteInput(MidiNoteGroup note) override;

		int saveToDisk(int startingAddress, Storage *storage) override;
		int loadFromDisk(int startingAddress, Storage *storage) override;

		void handleNoteOff(MidiNoteGroup note);

        bool chanceShouldSkip();

		uint8_t getLength();

		bool didLengthChange();

        // The next mfx index after going through the selector
        // If selector is in slot 1, length is 2, then the next mfx slot index
        // would be slot 4
        // if the index returned is greater than hte number of mfx slots(8)
        // then it will go to the group's output. 
        uint8_t getFinalMidiFXIndex(uint8_t thisMFXIndex);

        // This will return which MidiFX Index to go to
        // If selector is in slot 1, has a length of 2
        // then the note can go to either slot 2 or slot 3. 
        // If the is no mfx at slot 2 or slot 3, the note will go to
        // slot 4. 
        // If there is no midifx at slot 4, it will go to next slot with midifx
        // or the midifx group output if there are none
        uint8_t getSelectedMidiFXIndex(uint8_t thisMFXIndex);

        void setNoteInputFunc(uint8_t slotIndex, void (*fptr)(void *, midifx::MidiFXSelector *, uint8_t, MidiNoteGroup), void *context);

    protected:
		void onEnabled() override;
		void onDisabled() override;

		void onEncoderChangedEditParam(Encoder::Update enc) override;

	private:
		struct SelectorSave
		{
			uint8_t mode : 2;
            uint8_t length : 3; // 2-7
			uint8_t chancePerc : 7;
		};

		uint8_t mode_ : 2;
        uint8_t length_ : 3; // 2-7

		uint8_t chancePerc_ = 100;

        uint8_t selPos_ = 0;

		bool lengthChanged_;

        midifx::MidiFXInterface *getMidiFX(uint8_t index);

        void *noteInputContext_;
		void (*noteInputFunctionPtr_)(void *, midifx::MidiFXSelector *, uint8_t, MidiNoteGroup);
	};
}
