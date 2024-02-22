#pragma once
#include "../config.h"
#include "../utils/chord_structs.h"
#include "../utils/music_scales.h"
// #include "../modes/omx_mode_interface.h"
// #include "../modes/submodes/submode_clearstorage.h"

// Singleton class for making chords
class ChordUtil
{
public:
	ChordUtil();

	bool constructChord(ChordSettings *chord, ChordNotes *chordNotes, int scaleRoot, int scalePattern);
	bool constructChordBasic(ChordSettings * chord, ChordNotes * chordNotes);
private:
	MusicScales musicScale_;
	ChordBalanceDetails chordBalanceDetails;

	int AddOctave(int note, int8_t octave);
	int TransposeNote(int note, int8_t semitones);

	void updateChordBalance(uint8_t balance);
};

extern ChordUtil chordUtil;
