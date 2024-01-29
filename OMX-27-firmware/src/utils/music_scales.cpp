#include <cstdint>
#include "music_scales.h"
#include "../consts/colors.h"
#include "logic_util.h"
// #include "config.h"
// #include "omx_leds.h"
#include <Arduino.h>

const uint8_t rainbowSaturation = 127;
const uint8_t scaleBrightness = 200;

const auto ROOTNOTECOLOR = 0xA2A2FF;
const auto INSCALECOLOR = 0x000090;

String tempFullNoteName;

#include <Adafruit_NeoPixel.h>
extern Adafruit_NeoPixel strip;

const int8_t scalePatterns[][7] = {
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
	{0, 2, 5, 7, 10, -1, -1},
};

const char *scaleNames[] = {
	"major",
	"dorian",
	"phrygian",
	"lydian",
	"mixolydian",
	"minor",
	"locrian",

	"mel minor",
	"dorian b2",
	"lydian #5",
	"lydian b7",
	"mixo b6",
	"half-dim",
	"altered",

	"harm minor",
	"locrian 6",
	"ionian #5",
	"dorian #4",
	"phrygian dom",
	"lydian #2",
	"sup loc bb7",

	"dbl harm.maj",
	"lydian #2#6",
	"ultraphrygian",
	"hungarian",
	"oriental",
	"ionian #2#5",
	"loc bb3bb7",

	"penta maj",
	"penta min",
	"in sen",
	"iwato",
	"yo",
	"hirajoshi",
	"egyptian",
};

const char *noteNames[] = {
	"C ",
	"C#",
	"D ",
	"D#",
	"E ",
	"F ",
	"F#",
	"G ",
	"G#",
	"A ",
	"A#",
	"B ",
};

const char *noteNamesNoFormat[] = {
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

void MusicScales::calculateScaleIfModified(uint8_t scaleRoot, uint8_t scalePattern)
{
	if (scaleRoot == rootNote && scalePattern == scaleIndex)
		return;

	calculateScale(scaleRoot, scalePattern);
}

void MusicScales::calculateScale(uint8_t scaleRoot, uint8_t scalePattern)
{
<<<<<<< HEAD:OMX-27-firmware/src/music_scales.cpp
=======

>>>>>>> 5fe2be8 (File organization and includePath updates):OMX-27-firmware/src/utils/music_scales.cpp

	rootNote = scaleRoot;
	scaleIndex = scalePattern;
	auto pattern = getScalePattern(scalePattern);

	// auto sPattern2 = scalePatterns[scalePattern];

	if (scalePattern == -1)
	{
		// disabled
		for (int n = 0; n < 12; n++)
		{
			scaleOffsets[n] = -1;
			scaleDegrees[n] = -1;
			scaleColors[n] = LEDOFF;
		}
	}
	else
	{
		for (int n = 0; n < 12; n++)
		{
			int offset = -1;
			int degree = -1;

			for (int j = 0; j < 7; j++)
			{
				// int v = scalePatterns[scalePattern][j];
				int v = pattern[j];

				if (v == -1)
				{
					continue;
				}
				if ((scaleRoot + v) % 12 == n)
				{
					offset = v;
					degree = j;
					break;
				}
			}
			scaleOffsets[n] = offset;
			scaleDegrees[n] = degree;
			if (degree == -1)
			{
				scaleColors[n] = LEDOFF;
			}
			else
			{
				if (degree == 0)
				{
					scaleColors[n] = ROOTNOTECOLOR;
				}
				else
				{
					scaleColors[n] = INSCALECOLOR;
				}

				// Use for rainbow scale
				// scaleColors[n] = strip.gamma32(strip.ColorHSV((65535 / 12) * offset, rainbowSaturation, scaleBrightness));
			}
		}

		int k = 0;
		int octave = 0;

		// Populate offsets for group16 mode
		for (int i = 0; i < 16; i++)
		{
			int offset = pattern[k];

			if (offset == -1)
			{
				k = 0;
				offset = pattern[k];
				octave++;
			}
			k++;

			group16Offsets[i] = offset + 12 * octave;

			if (k >= 7)
			{
				k = 0;
				octave++;
			}
		}
	}
	scaleLength = 0;
	for (int j = 0; j < 7; j++)
	{
		int v = pattern[j];
		if (v != -1)
		{
			scaleLength++;
		}
	}

	scaleCalculated = true;
}

uint8_t MusicScales::getNumScales()
{
	return ARRAYLEN(scalePatterns);
}

bool MusicScales::isNoteInScale(int8_t noteNum)
{
<<<<<<< HEAD:OMX-27-firmware/src/music_scales.cpp
	// Serial.println((String)"isNoteInScale: " + noteNum );
	if (!scaleCalculated || noteNum < 0 || noteNum > 127)
	{
		return false;
	}
=======
    // Serial.println((String)"isNoteInScale: " + noteNum );
    if (!scaleCalculated || noteNum < 0 || noteNum > 127)
    {
        return false;
    }
>>>>>>> 5fe2be8 (File organization and includePath updates):OMX-27-firmware/src/utils/music_scales.cpp

	int noteIndex = noteNum % 12;
	bool inScale = scaleColors[noteIndex] != LEDOFF;

	// Serial.println((String)"noteIndex: " + noteNum + " inScale: " + (inScale ? "true" : "false"));

	return inScale;
}

int MusicScales::getGroup16Note(uint8_t keyNum, int8_t octave)
{
	//     1,2,   3,4,5,   6,7,   8,9,10,
	// 11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26

	if (keyNum < 11 || keyNum > 26 || scaleIndex < 0)
		return -1;

	int stepIndex = keyNum - 12; // C will be root note

	int adjnote;

	if (keyNum == 11) // edge case to make line up with C note
	{
		int offset = -1;

		for (int j = 0; j < 7; j++) // find last valid offset of scale
		{
			int o = scalePatterns[scaleIndex][j];
			if (o != -1)
			{
				offset = o;
			}
		}

		if (offset == -1)
			return -1;

		int firstNote = group16Offsets[0] + rootNote + 60 + (octave * 12);
		adjnote = firstNote + offset - 12; // lower by 1 octave
	}
	else
	{
		adjnote = group16Offsets[stepIndex] + rootNote + 60 + (octave * 12);
	}

	// adjnote = constrain(adjnote, -1, 127);

	return adjnote;
}

int8_t MusicScales::getNoteByDegree(uint8_t degree, int8_t octave)
{
	// degree should be less than 16
	if (degree >= 16)
		return -1;

	int adjnote;

<<<<<<< HEAD:OMX-27-firmware/src/music_scales.cpp
	if (scaleIndex < 0)
	{
		// Chromatically offset
		adjnote = 60 + rootNote + degree + (octave * 12);
		// Serial.println("Chromatic note: " + String(adjnote));
	}
	else
	{
		adjnote = group16Offsets[degree] + rootNote + 60 + (octave * 12);
	}
	if (adjnote > 127 || adjnote < -1)
		adjnote = -1;
	adjnote = constrain(adjnote, -1, 127);
=======
    if(scaleIndex < 0)
    {
        // Chromatically offset
        adjnote = 60 + rootNote + degree + (octave * 12);
        // Serial.println("Chromatic note: " + String(adjnote));
    }
    else
    {
        adjnote = group16Offsets[degree] + rootNote + 60 + (octave * 12);
    }
    if(adjnote > 127 || adjnote < -1) adjnote = -1;
    adjnote = constrain(adjnote,-1,127);
>>>>>>> 5fe2be8 (File organization and includePath updates):OMX-27-firmware/src/utils/music_scales.cpp

	return (int8_t)adjnote;
}

int MusicScales::getScaleColor(uint8_t noteIndex)
{
	if (!scaleCalculated)
		return LEDOFF;
	return scaleColors[noteIndex];
}

int MusicScales::getGroup16Color(uint8_t keyNum)
{
	if (!scaleCalculated || keyNum < 11 || keyNum > 26 || scaleIndex < 0)
		return LEDOFF;

	int note = getGroup16Note(keyNum, 4);

	if (note < 0)
		return LEDOFF;

	note = note % 12;

	return scaleColors[note];
}

const char *MusicScales::getNoteName(uint8_t noteIndex, bool removeSpaces)
{
	// noteIndex = constrain(noteIndex, 0, 11);
	if (removeSpaces)
	{
		return noteNamesNoFormat[noteIndex % 12];
	}
	return noteNames[noteIndex % 12];
}

const char *MusicScales::getFullNoteName(uint8_t noteNumber)
{
<<<<<<< HEAD:OMX-27-firmware/src/music_scales.cpp
	int8_t octave = (noteNumber / 12) - 2;
	tempFullNoteName = String(noteNamesNoFormat[noteNumber % 12] + String(octave));
=======
    int8_t octave = (noteNumber / 12) - 2;
    tempFullNoteName = String(noteNamesNoFormat[noteNumber % 12] + String(octave));

    // strcpy(fullNoteNameBuf, noteNamesNoFormat[noteNumber % 12]);
    // strcat(fullNoteNameBuf, itoa(octave,fullNoteNameBuf,10));
>>>>>>> 5fe2be8 (File organization and includePath updates):OMX-27-firmware/src/utils/music_scales.cpp

	// strcpy(fullNoteNameBuf, noteNamesNoFormat[noteNumber % 12]);
	// strcat(fullNoteNameBuf, itoa(octave,fullNoteNameBuf,10));

	// return fullNoteNameBuf;
	// int8_t octave = (noteNumber / 12) - 2;

	// String newString = noteNamesNoFormat[noteNumber % 12] + String(octave);

	// tempFullNoteName = newString;

	return tempFullNoteName.c_str();
}

const char *MusicScales::getScaleName(uint8_t scaleIndex)
{
	if (scaleIndex < 0 || scaleIndex >= getNumScales())
		return "off";
	return scaleNames[scaleIndex];
}

int MusicScales::getScaleLength()
{
	return scaleLength;
}

const int8_t *MusicScales::getScalePattern(uint8_t noteIndex)
{
	return scalePatterns[noteIndex];
}
