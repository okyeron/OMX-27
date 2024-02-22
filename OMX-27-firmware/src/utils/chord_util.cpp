#include <U8g2_for_Adafruit_GFX.h>

#include "chord_util.h"
#include "../consts/consts.h"
#include "../midi/midi.h"
#include "../consts/colors.h"
#include "../hardware/omx_leds.h"
#include "../hardware/omx_disp.h"
#include "../midi/noteoffs.h"
#include "../modes/sequencer.h"

const uint8_t kNumChordPatterns = 37;
const uint8_t kCustomChordPattern = kNumChordPatterns - 1;

// Last pattern is custom
const int8_t chordPatterns[kNumChordPatterns - 1][3] = {
	{4, 7, -1},	 // Major        C E G
	{3, 7, -1},	 // minor        C Eb G
	{2, 7, -1},	 // sus2         C D G
	{5, 7, -1},	 // sus4         C F G
	{3, 6, -1},	 // mb5          C Eb Gb
	{4, 6, -1},	 // Mb5          C E Gb
	{4, 8, -1},	 // M#5          C E G#
	{4, 14, -1}, // M9no5        C E D2  no 5

	{3, 6, 9},	 // dim7         C Eb Gb A
	{3, 6, 10},	 // m7b5         C Eb Gb Bb
	{3, 7, 8},	 // mb6          C Eb G Ab
	{3, 7, 9},	 // m6           C Eb G A
	{3, 7, 10},	 // m7           C Eb G Bb
	{3, 7, 11},	 // mMaj7        C Eb G B
	{3, 7, 14},	 // madd9        C Eb G D
	{3, 8, 10},	 // m7#5         C Eb Ab Bb
	{3, 10, 13}, // m7b9no5      C Eb Bb Db2
	{3, 10, 14}, // m9no5        C Eb Bb D2

	{4, 5, 9},	 // M6add4no5    C E F A
	{4, 6, 10},	 // M7b5         C E Gb Bb
	{4, 6, 11},	 // Maj7b5       C E Gb B
	{4, 6, 14},	 // Madd9b5      C E Gb D2
	{4, 7, 8},	 // Maddb5       C E G Gb
	{4, 7, 9},	 // M6           C E G A
	{4, 7, 10},	 // M7           C E G Bb
	{4, 7, 11},	 // Maj7         C E G B
	{4, 7, 14},	 // Madd9        C E G D2
	{4, 8, 10},	 // M7#5         C E G# Bb
	{4, 10, 13}, // M7b9no5      C E Bb Db2
	{4, 11, 14}, // Maj9no5      C E B D2
	{4, 11, 21}, // Maj7/6no5    C E B A2
	{5, 7, 8},	 // sus4add#5    C F G G#
	{5, 7, 10},	 // 7sus4        C F G Bb
	{5, 8, 13},	 // sus4#5b9     C F G# Db2
	{5, -1, -1}, // Fourth       CF
	{7, -1, -1}	 // Fifth        CG
};

const char *kChordMsg[kNumChordPatterns] = {
	"Major",
	"Minor",
	"sus2",
	"sus4",
	"mb5",
	"Mb5",
	"M#5",
	"M9no5",

	"dim7",
	"m7b5",
	"mb6",
	"m6",
	"m7",
	"mMaj7",
	"madd9",
	"m7#5",
	"m7b9no5",
	"m9no5",

	"M6add4no5",
	"M7b5",
	"Maj7b5",
	"Madd9b5",
	"Maddb5",
	"M6",
	"M7",
	"Maj7",
	"Madd9",
	"M7#5",
	"M7b9no5",
	"Maj9no5",
	"Maj7/6no5",
	"sus4add#5",
	"7sus4",
	"sus4#5b9",

	"Fourths",
	"Fifth",
	"Custom"};

const uint8_t kNumChordBalance = 23;

const int8_t chordBalance[kNumChordBalance][3] = {
	{-10, -10, -10}, // 0 Single Note - 0
	{0, -10, -10},	 // 10 Power Chord - 10
	{0, 0, -10},	 // 20 Triad
	{0, 0, 0},		 // 30 Four notes - Root
	{0, 0, 0},		 // 32 Four notes - Root
	{-10, 0, 0},	 // 37
	{-1, 0, 0},		 // 42
	{-1, -10, 0},	 // 47
	{-1, -1, 0},	 // 52
	{-1, -1, -10},	 // 57
	{-1, -1, -1},	 // 62 - Inv 1
	{-10, -1, -1},	 // 69
	{0, -1, -1},	 // 74 - Inv 2
	{0, -10, -1},	 // 79
	{0, 0, -1},		 // 84 - Inv 3
	{0, 0, -10},	 // 91
	{0, 0, 0},		 // 96
	{-10, 0, 0},	 // 101
	{1, 0, 0},		 // 106
	{1, -10, 0},	 // 111
	{1, 1, 0},		 // 116
	{1, 1, -10},	 // 121
	{1, 1, 1},		 // 127
};

// int balSize = sizeof(chordBalance);
// int patSize = sizeof(chordPatterns);

const char *kChordTypeDisp[2] = {"BASC", "INTV"};
const char *kVoicingNames[8] = {"NONE", "POWR", "SUS2", "SUS4", "SU24", "+6", "+6+9", "KB11"};

// extern const uint8_t kNumChordPatterns = 37;
// extern const uint8_t kCustomChordPattern = kNumChordPatterns - 1;

// // Last pattern is custom
// extern const int8_t chordPatterns[kNumChordPatterns - 1][3] = {
// 	{4, 7, -1},	 // Major        C E G
// 	{3, 7, -1},	 // minor        C Eb G
// 	{2, 7, -1},	 // sus2         C D G
// 	{5, 7, -1},	 // sus4         C F G
// 	{3, 6, -1},	 // mb5          C Eb Gb
// 	{4, 6, -1},	 // Mb5          C E Gb
// 	{4, 8, -1},	 // M#5          C E G#
// 	{4, 14, -1}, // M9no5        C E D2  no 5

// 	{3, 6, 9},	 // dim7         C Eb Gb A
// 	{3, 6, 10},	 // m7b5         C Eb Gb Bb
// 	{3, 7, 8},	 // mb6          C Eb G Ab
// 	{3, 7, 9},	 // m6           C Eb G A
// 	{3, 7, 10},	 // m7           C Eb G Bb
// 	{3, 7, 11},	 // mMaj7        C Eb G B
// 	{3, 7, 14},	 // madd9        C Eb G D
// 	{3, 8, 10},	 // m7#5         C Eb Ab Bb
// 	{3, 10, 13}, // m7b9no5      C Eb Bb Db2
// 	{3, 10, 14}, // m9no5        C Eb Bb D2

// 	{4, 5, 9},	 // M6add4no5    C E F A
// 	{4, 6, 10},	 // M7b5         C E Gb Bb
// 	{4, 6, 11},	 // Maj7b5       C E Gb B
// 	{4, 6, 14},	 // Madd9b5      C E Gb D2
// 	{4, 7, 8},	 // Maddb5       C E G Gb
// 	{4, 7, 9},	 // M6           C E G A
// 	{4, 7, 10},	 // M7           C E G Bb
// 	{4, 7, 11},	 // Maj7         C E G B
// 	{4, 7, 14},	 // Madd9        C E G D2
// 	{4, 8, 10},	 // M7#5         C E G# Bb
// 	{4, 10, 13}, // M7b9no5      C E Bb Db2
// 	{4, 11, 14}, // Maj9no5      C E B D2
// 	{4, 11, 21}, // Maj7/6no5    C E B A2
// 	{5, 7, 8},	 // sus4add#5    C F G G#
// 	{5, 7, 10},	 // 7sus4        C F G Bb
// 	{5, 8, 13},	 // sus4#5b9     C F G# Db2
// 	{5, -1, -1}, // Fourth       CF
// 	{7, -1, -1}	 // Fifth        CG
// };

// extern const char *kChordMsg[kNumChordPatterns] = {
// 	"Major",
// 	"Minor",
// 	"sus2",
// 	"sus4",
// 	"mb5",
// 	"Mb5",
// 	"M#5",
// 	"M9no5",

// 	"dim7",
// 	"m7b5",
// 	"mb6",
// 	"m6",
// 	"m7",
// 	"mMaj7",
// 	"madd9",
// 	"m7#5",
// 	"m7b9no5",
// 	"m9no5",

// 	"M6add4no5",
// 	"M7b5",
// 	"Maj7b5",
// 	"Madd9b5",
// 	"Maddb5",
// 	"M6",
// 	"M7",
// 	"Maj7",
// 	"Madd9",
// 	"M7#5",
// 	"M7b9no5",
// 	"Maj9no5",
// 	"Maj7/6no5",
// 	"sus4add#5",
// 	"7sus4",
// 	"sus4#5b9",

// 	"Fourths",
// 	"Fifth",
// 	"Custom"};

// extern const uint8_t kNumChordBalance = 23;

// extern const int8_t chordBalance[kNumChordBalance][3] = {
// 	{-10, -10, -10}, // 0 Single Note - 0
// 	{0, -10, -10},	 // 10 Power Chord - 10
// 	{0, 0, -10},	 // 20 Triad
// 	{0, 0, 0},		 // 30 Four notes - Root
// 	{0, 0, 0},		 // 32 Four notes - Root
// 	{-10, 0, 0},	 // 37
// 	{-1, 0, 0},		 // 42
// 	{-1, -10, 0},	 // 47
// 	{-1, -1, 0},	 // 52
// 	{-1, -1, -10},	 // 57
// 	{-1, -1, -1},	 // 62 - Inv 1
// 	{-10, -1, -1},	 // 69
// 	{0, -1, -1},	 // 74 - Inv 2
// 	{0, -10, -1},	 // 79
// 	{0, 0, -1},		 // 84 - Inv 3
// 	{0, 0, -10},	 // 91
// 	{0, 0, 0},		 // 96
// 	{-10, 0, 0},	 // 101
// 	{1, 0, 0},		 // 106
// 	{1, -10, 0},	 // 111
// 	{1, 1, 0},		 // 116
// 	{1, 1, -10},	 // 121
// 	{1, 1, 1},		 // 127
// };

// extern int balSize = sizeof(chordBalance);
// extern int patSize = sizeof(chordPatterns);

// extern const char *kChordTypeDisp[8] = {"BASC", "INTV"};
// extern const char *kVoicingNames[8] = {"NONE", "POWR", "SUS2", "SUS4", "SU24", "+6", "+6+9", "KB11"};

ChordUtil::ChordUtil()
{
    musicScale_.calculateScale(0,0);
}

int ChordUtil::AddOctave(int note, int8_t octave)
{
	if (note < 0 || note > 127)
		return -1;

	int newNote = note + (12 * octave);
	if (newNote < 0 || newNote > 127)
		return -1;
	return newNote;
}

int ChordUtil::TransposeNote(int note, int8_t semitones)
{
	if (note < 0 || note > 127)
		return -1;

	int newNote = note + semitones;
	if (newNote < 0 || newNote > 127)
		return -1;
	return newNote;
}

bool ChordUtil::constructChord(ChordSettings *chord, ChordNotes *chordNotes, int scaleRoot, int scalePattern)
{
    musicScale_.calculateScaleIfModified(scaleRoot, scalePattern);

	// Serial.println("Constructing Chord: " + String(chordIndex));
	// auto chord = chords_[chordIndex];

	if (chord->type == CTYPE_BASIC)
	{
		return constructChordBasic(chord, chordNotes);
	}

	int8_t octave = midiSettings.octave + chord->octave;

	uint8_t numNotes = 0;

	for (uint8_t i = 0; i < 6; i++)
	{
		chordNotes->notes[i] = -1;
		chordNotes->velocities[i] = chord->velocity;
	}

	if (chord->numNotes == 0)
	{
		return false;
	}
	else if (chord->numNotes == 1)
	{
		chordNotes->notes[0] = musicScale_.getNoteByDegree(chord->degree, octave);
		numNotes = 1;
	}
	else if (chord->numNotes == 2)
	{
		chordNotes->notes[0] = musicScale_.getNoteByDegree(chord->degree, octave);
		chordNotes->notes[1] = musicScale_.getNoteByDegree(chord->degree + 2, octave);
		numNotes = 2;
	}
	else if (chord->numNotes == 3)
	{
		chordNotes->notes[0] = musicScale_.getNoteByDegree(chord->degree, octave);
		chordNotes->notes[1] = musicScale_.getNoteByDegree(chord->degree + 2, octave);
		chordNotes->notes[2] = musicScale_.getNoteByDegree(chord->degree + 4, octave);
		numNotes = 3;
	}
	else if (chord->numNotes == 4)
	{
		chordNotes->notes[0] = musicScale_.getNoteByDegree(chord->degree, octave);
		chordNotes->notes[1] = musicScale_.getNoteByDegree(chord->degree + 2, octave);
		chordNotes->notes[2] = musicScale_.getNoteByDegree(chord->degree + 4, octave);
		chordNotes->notes[3] = musicScale_.getNoteByDegree(chord->degree + 6, octave);
		numNotes = 4;
	}

	chordNotes->rootNote = chordNotes->notes[0];

	// Serial.println("numNotes: " + String(numNotes));

	switch (chord->voicing)
	{
	case CHRDVOICE_NONE:
	{
	}
	break;
	case CHRDVOICE_POWER:
	{
		if (chord->numNotes > 1)
		{
			chordNotes->notes[1] = musicScale_.getNoteByDegree(chord->degree + 4, octave);
		}
		if (chord->numNotes > 2)
		{
			chordNotes->notes[2] = chordNotes->notes[1] + 12;
			for (uint8_t i = 3; i < 6; i++)
			{
				chordNotes->notes[i] = -1;
			}
			numNotes = 3;
		}
	}
	break;
	case CHRDVOICE_SUS2:
	{
		if (chord->numNotes > 1)
		{
			chordNotes->notes[1] = musicScale_.getNoteByDegree(chord->degree + 1, octave);
		}
	}
	break;
	case CHRDVOICE_SUS4:
	{
		if (chord->numNotes > 1)
		{
			chordNotes->notes[1] = musicScale_.getNoteByDegree(chord->degree + 3, octave);
		}
	}
	break;
	case CHRDVOICE_SUS24:
	{
		if (chord->numNotes > 1)
		{
			chordNotes->notes[1] = musicScale_.getNoteByDegree(chord->degree + 1, octave);
		}
		if (chord->numNotes > 2)
		{
			chordNotes->notes[2] = musicScale_.getNoteByDegree(chord->degree + 3, octave);
		}
	}
	break;
	case CHRDVOICE_ADD6:
	{
		chordNotes->notes[chord->numNotes] = musicScale_.getNoteByDegree(chord->degree + 5, octave);
		numNotes = chord->numNotes + 1;
	}
	break;
	case CHRDVOICE_ADD69:
	{
		chordNotes->notes[chord->numNotes] = musicScale_.getNoteByDegree(chord->degree + 5, octave);
		chordNotes->notes[chord->numNotes + 1] = musicScale_.getNoteByDegree(chord->degree + 8, octave);
		numNotes = chord->numNotes + 2;
	}
	break;
	case CHRDVOICE_KB11:
	{
		if (chord->numNotes > 1)
		{
			chordNotes->notes[0] = musicScale_.getNoteByDegree(chord->degree + 0, octave);
			chordNotes->notes[1] = musicScale_.getNoteByDegree(chord->degree + 4, octave);
			numNotes = 2;
		}
		if (chord->numNotes > 2)
		{
			chordNotes->notes[2] = musicScale_.getNoteByDegree(chord->degree + 8, octave);
			numNotes = 3;
		}
		if (chord->numNotes > 3)
		{
			chordNotes->notes[3] = musicScale_.getNoteByDegree(chord->degree + 9, octave);
			chordNotes->notes[4] = musicScale_.getNoteByDegree(chord->degree + 6, octave + 1);
			chordNotes->notes[5] = musicScale_.getNoteByDegree(chord->degree + 10, octave + 1);
			numNotes = 6;
		}
	}
	break;
	default:
		break;
	}

	// Serial.println("numNotes: " + String(numNotes));

	if (chord->quartalVoicing)
	{
		chordNotes->notes[0] = AddOctave(chordNotes->notes[0], 2);
		chordNotes->notes[1] = AddOctave(chordNotes->notes[1], 0);
		chordNotes->notes[2] = AddOctave(chordNotes->notes[2], 1);
		chordNotes->notes[3] = AddOctave(chordNotes->notes[3], -1);
	}

	if (chord->spreadUpDown)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			if (i % 2 == 0)
			{
				chordNotes->notes[i] = AddOctave(chordNotes->notes[i], -1);
			}
			else
			{
				chordNotes->notes[i] = AddOctave(chordNotes->notes[i], 1);
			}
		}
	}

	if (chord->spread < 0)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			if (i % 2 == 0)
			{
				chordNotes->notes[i] = AddOctave(chordNotes->notes[i], chord->spread);
			}
		}
	}
	else if (chord->spread > 0)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			if (i % 2 != 0)
			{
				chordNotes->notes[i] = AddOctave(chordNotes->notes[i], chord->spread);
			}
		}
	}

	if (chord->rotate != 0 && numNotes > 0)
	{
		int temp[numNotes];

		uint8_t val = numNotes - chord->rotate;

		uint8_t offset = chord->rotate % numNotes;

		for (uint8_t i = 0; i < offset; i++)
		{
			chordNotes->notes[i] = AddOctave(chordNotes->notes[i], 1);
		}

		for (uint8_t i = 0; i < numNotes; i++)
		{
			temp[i] = chordNotes->notes[abs((i + val) % numNotes)];
		}
		for (int i = 0; i < numNotes; i++)
		{
			chordNotes->notes[i] = temp[i];
		}
	}

	for (uint8_t i = 0; i < 6; i++)
	{
		chordNotes->notes[i] = TransposeNote(chordNotes->notes[i], chord->transpose);
	}

	chordNotes->midifx = chord->midiFx;

	return true;
}

bool ChordUtil::constructChordBasic(ChordSettings * chord, ChordNotes * chordNotes)
{
	// auto chord = chords_[chordIndex];

	// int8_t octave = midiSettings.octave + chord->octave;

	// uint8_t numNotes = 0;

	for (uint8_t i = 0; i < 6; i++)
	{
		chordNotes->notes[i] = -1;
        // Note velocity is set below by the chord balance
	}

	// int adjRoot = notes[thisKey] + (midiSettings.octave + 1 * 12);

	int rootNote = chord->note + ((chord->basicOct + 5) * 12);

	if (rootNote < 0 || rootNote > 127)
		return false;

	chordNotes->rootNote = rootNote;

	chordNotes->midifx = chord->midiFx;

	chordNotes->notes[0] = rootNote;

	if (chord->chord == kCustomChordPattern)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			int noteOffset = chord->customNotes[i].note;

			if (noteOffset != 0 || (noteOffset == 0 && i == 0))
			{
				chordNotes->notes[i] = rootNote + noteOffset;
			}
			// else offset is zero, do nothing.
		}
	}
	else
	{
		auto pattern = chordPatterns[chord->chord];

		for (uint8_t i = 0; i < 3; i++)
		{
			if (pattern[i] >= 0)
			{
				chordNotes->notes[i + 1] = rootNote + pattern[i];
			}
		}
	}

	updateChordBalance(chord->balance);

	for (uint8_t i = 0; i < 4; i++)
	{
		int pnote = chordNotes->notes[i];

		if (pnote >= 0 && pnote <= 127)
		{
			int bal = chordBalanceDetails.type[i];

			chordNotes->notes[i] = (bal <= -10 ? -1 : (pnote + (12 * bal)));
			chordNotes->velocities[i] = chord->velocity * chordBalanceDetails.velMult[i];
		}
	}

	return true;
}

void ChordUtil::updateChordBalance(uint8_t balance)
{
	// ChordBalanceDetails bDetails;

	chordBalanceDetails.type[0] = 0;
	chordBalanceDetails.velMult[0] = 1.0f;

	uint8_t balanceIndex = balance / 10;

	auto balancePat = chordBalance[balanceIndex];

	for (uint8_t i = 0; i < 3; i++)
	{
		int8_t bal = balancePat[i];

		chordBalanceDetails.type[i + 1] = bal;

		if (balanceIndex < kNumChordBalance)
		{
			int8_t nextBal = chordBalance[balanceIndex + 1][i];

			if ((balance % 10) != 0)
			{
				if (nextBal > -10)
				{
					chordBalanceDetails.type[i + 1] = nextBal;
				}
			}

			float v1 = bal <= -10 ? 0.0f : 1.0f;
			float v2 = nextBal <= -10 ? 0.0f : 1.0f;

			chordBalanceDetails.velMult[i + 1] = map((float)balance, balanceIndex * 10.0f, (balanceIndex + 1) * 10.0f, v1, v2);
		}
		else
		{
			chordBalanceDetails.velMult[i + 1] = 1.0f;
		}
	}
}

ChordUtil chordUtil;
