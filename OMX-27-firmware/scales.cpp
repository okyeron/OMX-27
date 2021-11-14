#include "scales.h"
#include "util.h"

int scaleOffsets[12];
int scaleDegrees[12];
int scaleLength = 0;

const int scalePatterns[][7] = {
	// major / ionian
	{0, 2, 4, 5, 7, 9, 11},
	// dorian
	{0, 2, 3, 5, 7, 9, 10},
	// phrygian
	{0, 1, 3, 5, 7, 8, 10},
	// lydian
	{0, 2, 4, 6, 7, 9, 11},
	// mixolydian
	{0, 2, 4, 5, 7, 9, 10},
	// minor / aeolian
	{0, 2, 3, 5, 7, 8, 10},
	// locrian
	{0, 1, 3, 5, 6, 8, 10},

	// melodic minor
	{0, 2, 3, 5, 7, 9, 11},
	// dorian b2
	{0, 1, 3, 5, 7, 9, 10},
	// lydian #5
	{0, 2, 4, 6, 8, 9, 11},
	// lydian b7
	{0, 2, 4, 6, 7, 9, 10},
	// mixolydian b6
	{0, 2, 4, 5, 7, 8, 10},
	// half-diminished (locrian natural 2)
	{0, 2, 3, 5, 6, 8, 10},
	// altered (super locrian)
	{0, 1, 3, 4, 6, 8, 10},

	// harmonic minor
	{0, 2, 3, 5, 7, 8, 11},
	// locrian 6
	{0, 1, 3, 5, 6, 9, 10},
	// ionian #5
	{0, 2, 4, 5, 8, 9, 11},
	// dorian #4
	{0, 2, 3, 6, 7, 9, 10},
	// phrygian dominant
	{0, 1, 4, 5, 7, 8, 10},
	// lydian #2
	{0, 3, 4, 6, 7, 9, 11},
	// super locrian bb7
	{0, 1, 3, 4, 6, 8, 9},

	// double harmonic
	{0, 1, 4, 5, 7, 8, 11},
	// lydian #2#6
	{0, 3, 4, 6, 7, 10, 11},
	// ultraphrygian
	{0, 1, 3, 4, 7, 8, 9},
	// hungarian
	{0, 2, 3, 6, 7, 8, 11},
	// oriental
	{0, 1, 4, 5, 6, 9, 10},
	// ionian #2#5
	{0, 3, 4, 5, 8, 9, 11},
	// locrian bb3bb7
	{0, 2, 3, 5, 6, 8, 9},

	// pentatonic scales
	// major pentatonic
	{0, 2, 4, 7, 9, -1, -1},
	// minor pentatonic
	{0, 3, 5, 7, 9, -1, -1},
	// in sen (japanese)
	{0, 1, 5, 7, 10, -1, -1},
	// iwato
	{0, 1, 5, 6, 10, -1, -1},
	// yo
	{0, 2, 5, 7, 9, -1, -1},
	// hirajoshi
	{0, 2, 3, 7, 8, -1, -1},
	// egyptian
	{0, 2, 5, 7, 10, -1 , -1},
};

const char* scaleNames[] = {
	"major",
	"dorian",
	"phrygian",
	"lydian",
	"mixolydian",
	"minor",
	"locrian",

	"mel. minor",
	"dorian b2",
	"lydian #5",
	"lydian b7",
	"mixo b6",
	"half-dim",
	"altered",

	"harm. minor",
	"locrian 6",
	"ionian #5",
	"dorian #4",
	"phrygian dom.",
	"lydian #2",
	"sup loc bb7",

	"dbl harm.maj",
	"lydian #2#6",
	"ultraphrygian",
	"hungarian",
	"oriental",
	"ionian #2#5",
	"loc. bb3bb7",

	"penta maj",
	"penta min",
	"in sen",
	"iwato",
	"yo",
	"hirajoshi",
	"egyptian",
};

const char* noteNames[] = {
	"C",
	"C#",
	"D",
	"D#",
	"E",
	"F",
	"F#",
	"G",
	"G#",
	"A",
	"A#",
	"B",
};



void setScale(int scaleRoot, int scalePattern) {
	if(scaleRoot == -1) {
		// disabled
		for(int n = 0; n < 12; n++) {
			scaleOffsets[n] = -1;
			scaleDegrees[n] = -1;
		}
	} else {
		for(int n = 0; n < 12; n++) {
			int offset = -1;
			int degree = -1;
			for(int j = 0; j < 7; j++) {
				int v = scalePatterns[scalePattern][j];
				if(v == -1) {
					continue;
				}
				if((scaleRoot + v) % 12 == n) {
					offset = v;
					degree = j;
					break;
				}
			}
			scaleOffsets[n] = offset;
			scaleDegrees[n] = degree;
		}
	}
	scaleLength = 0;
	for(int j = 0; j < 7; j++) {
		int v = scalePatterns[scalePattern][j];
		if(v != -1) {
			scaleLength++;
		}
	}
}

int getNumScales() {
	return ARRAYLEN(scalePatterns);
}
