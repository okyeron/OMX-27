#pragma once

#include "midifx_interface.h"
#include "../utils/chord_structs.h"

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
        struct NoteTracker
        {
            int8_t triggerCount;
            uint8_t noteNumber : 7;
            uint8_t midiChannel : 4;
        };

		uint8_t chancePerc_ = 255;

        ParamManager basicParams_;
	    ParamManager intervalParams_;

        ChordSettings chord_;
	    ChordNotes chordNotes_;
	    ChordNotes playedChordNotes_;
	    ChordNotes chordEditNotes_;

	    ChordBalanceDetails activeChordBalance_;

	    int noNotes[6] = {-1, -1, -1, -1, -1, -1};

	    const uint8_t kMaxNoteTrackerSize = 32;

	    std::vector<NoteTracker> noteOffTracker;


		bool useGlobalScale_ = true;
        int8_t rootNote_ = 0;
		int8_t scaleIndex_ = 0;

        void onChordOn(MidiNoteGroup inNote);
	};
}
