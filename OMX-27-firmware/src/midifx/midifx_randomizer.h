#pragma once

#include "midifx_interface.h"

namespace midifx
{

	class MidiFXRandomizer : public MidiFXInterface
	{
	public:
		MidiFXRandomizer();
		~MidiFXRandomizer() {}

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

		void processNoteOff(MidiNoteGroup note) override;


	private:
		struct RandTrackedNote
		{
			uint8_t prevNoteNumber = 0;
			uint8_t channel = 1;
			uint8_t origChannel = 1;
			uint8_t noteNumber = 0;

			RandTrackedNote()
			{
			}

			void setFromNoteGroup(MidiNoteGroup *noteGroup)
			{
				prevNoteNumber = noteGroup->prevNoteNumber;
				channel = noteGroup->channel;
				origChannel = noteGroup->channel;
				noteNumber = noteGroup->noteNumber;
			}
		};

		// std::vector<MidiNoteGroup> triggeredNotes;
		struct RandomSave
		{
			uint8_t noteMinus : 4;
			uint8_t notePlus : 4;
			uint8_t octMinus : 4;
			uint8_t octPlus : 4;
			uint8_t velMinus : 7;
			uint8_t velPlus : 7;
			uint8_t lengthPerc : 7;
			uint8_t midiChan : 5;
			uint8_t delayMin : 5;
			uint8_t delayMax : 5;
			uint8_t chancePerc : 7;
		};

		uint8_t noteMinus_ : 4;	 // 0 to 12
		uint8_t notePlus_ : 4;	 // 0 to 12
		uint8_t octMinus_ : 4;	 // 0 to 12
		uint8_t octPlus_ : 4;	 // 0 to 12
		uint8_t velMinus_ : 7;	 // 0 to 127
		uint8_t velPlus_ : 7;	 // 0 to 127
		uint8_t lengthPerc_ : 7; // 0 to 100
		uint8_t midiChan_ : 5; // 0 to 16, Midi channel random range between incoming channel and channel + this value
		uint8_t delayMin_ : 5; // Maps to kArpRates, except 0 = off
		uint8_t delayMax_ : 5;
		uint8_t chancePerc_ : 7; // 0 to 100

		static const int queueSize = 16;
		std::vector<MidiNoteGroup> delayedNoteQueue;	  // notes pending for quantization

		std::vector<RandTrackedNote> trackedNotes;	  // notes that are tracked because midi chan changed


		static uint8_t getDelayLength(uint8_t delayIndex);

		static uint8_t getRand(uint8_t v, uint8_t minus, uint8_t plus);

		void removeFromDelayQueue(MidiNoteGroup *note);
		void processDelayedNote(MidiNoteGroup *note);

		void processNoteOn(MidiNoteGroup note);
	};
}
