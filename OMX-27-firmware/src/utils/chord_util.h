#pragma once
#include "../config.h"
#include "../utils/chord_structs.h"
#include "../utils/music_scales.h"
#include "../ClearUI/ClearUI_Input.h"
// #include "../modes/omx_mode_interface.h"
// #include "../modes/submodes/submode_clearstorage.h"

// Singleton class for making chords
class ChordUtil
{
public:
	ChordUtil();

	bool constructChord(ChordSettings *chord, ChordNotes *chordNotes, int scaleRoot, int scalePattern);
	bool constructChordBasic(ChordSettings * chord, ChordNotes * chordNotes);
	bool constructChordInterval(ChordSettings *chord, ChordNotes *chordNotes, int scaleRoot, int scalePattern);

	ChordBalanceDetails getChordBalance(uint8_t balance);
	MusicScales* getMusicScale();

	void onEncoderChangedEditParam(Encoder::Update *enc, ChordSettings *chord, uint8_t selectedParmIndex, uint8_t targetParamIndex, uint8_t paramType);
	void setupPageLegend(ChordSettings *chord, uint8_t index, uint8_t paramType);
private:
	MusicScales musicScale_;
	ChordBalanceDetails chordBalanceDetails;

	int AddOctave(int note, int8_t octave);
	int TransposeNote(int note, int8_t semitones);

	void updateChordBalance(uint8_t balance);
};

extern ChordUtil chordUtil;
