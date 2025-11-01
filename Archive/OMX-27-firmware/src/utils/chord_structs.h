#pragma once
#include "../config.h"

#define NUM_CHORD_PATTERNS 37

extern const uint8_t kNumChordPatterns;
extern const uint8_t kCustomChordPattern;

// Last pattern is custom
extern const int8_t chordPatterns[NUM_CHORD_PATTERNS - 1][3];

extern const char *kChordMsg[NUM_CHORD_PATTERNS];

#define NUM_CHORD_BALANCE 23

extern const uint8_t kNumChordBalance;

extern const int8_t chordBalance[NUM_CHORD_BALANCE][3];

// extern int balSize;
// extern int patSize;

extern const char *kChordTypeDisp[2];
extern const char *kVoicingNames[8];

enum ChordVoicing
{
	CHRDVOICE_NONE,
	CHRDVOICE_POWER,
	CHRDVOICE_SUS2,
	CHRDVOICE_SUS4,
	CHRDVOICE_SUS24,
	CHRDVOICE_ADD6,
	CHRDVOICE_ADD69,
	CHRDVOICE_KB11
};

enum ChordsModeParams
{
	CPARAM_UIMODE,
	CPARAM_MAN_STRUM,
	CPARAM_CHORD_TYPE,
	CPARAM_CHORD_MFX,
	CPARAM_CHORD_VEL,
	CPARAM_CHORD_MCHAN,
	CPARAM_BAS_NOTE,
	CPARAM_BAS_OCT,
	CPARAM_BAS_CHORD,
	CPARAM_BAS_BALANCE,
	CPARAM_INT_NUMNOTES,
	CPARAM_INT_DEGREE,
	CPARAM_INT_OCTAVE,
	CPARAM_INT_TRANSPOSE,
	CPARAM_INT_SPREAD,
	CPARAM_INT_ROTATE,
	CPARAM_INT_VOICING,
	CPARAM_INT_SPRDUPDOWN,
	CPARAM_INT_QUARTVOICE
};

struct CustomChordNote
{
	int8_t note : 7; // Root NoteNumber Offset or degree
};

struct ChordSettings
{
public:
	int color = 0xFFFFFF;
	uint8_t type : 1;
	int8_t midiFx : 4;
	uint8_t mchan : 4;
	uint8_t velocity : 7;

	// Basic Type:
	uint8_t note : 4;
	int8_t basicOct : 4;
	uint8_t chord : 6;
	uint8_t balance : 8; // 0 - 23 * 10

	CustomChordNote customNotes[6];
	// CustomChordDegree customDegrees[6];

	// Interval Type:
	uint8_t numNotes : 3;
	uint8_t degree : 3;	  // degree from root note of scale, if scale is cmaj, degree of 0 = c, degree of 3 = e
	int8_t octave : 4;	  // transposes note by octave
	int8_t transpose : 5; // transposes note by semitone, can bump off scale
	int8_t spread : 4;	  // spreads chord notes over octave
	// spread 0 =   C3,E3,G3        C3,E3,G3,B3
	// spread -1 =  C2,E3,G2        C2,E3,G2,B3     -1,*,-1     -1,*,-1,*
	// spread -2 =  C1,E3,G1        C1,E3,G1,B3     -2,*,-2     -2,*,-2,*
	// spread 1 =   C3,E4,G3        C3,E4,G3,B4     *,+1,*      *,+1,*,+1
	// spread 2 =   C3,E5,G3        C3,E5,G3,B5     *,+2,*      *,+2,*,+2
	uint8_t rotate : 4; // Rotates the chord notes
	// rotate 0 =   C3,E3,G3        C3,E3,G3,B3
	// rotate 1 =   E3,G3,C4        E3,G3,B3,C4
	// rotate 2 =   G3,C4,E4        G3,B3,C4,E4
	// rotate 3 =   C3,E3,G3        B3,C4,E4,G4
	// rotate 4 =   E3,G3,C4        C3,E3,G3,B3
	bool spreadUpDown = false; // spreads notes in both directions
	// false =      C3,E3,G3        C3,E3,G3,B3
	// true =       C2,E4,G2        C2,E4,G2,B4
	// Spead -1 =   C1,E4,G1        C1,E4,G1,E4
	// bool widerInterDown = false; // Eh, not sure about this one. Could get with a rotate spread combo
	// false =      C3,E3,G3        C3,E3,G3,B3
	// true =       G2,C3,E3        C3,E3,G3,B3
	bool quartalVoicing = false;
	// false =      C3,E3,G3        C3,E3,G3,B3
	// true =       C5,E3,G4        C5,E3,G4,B2
	uint8_t voicing : 3;
	// 0 = none - based off numNotes
	// 1 = powerChord
	//  C3,G3       C3,G3,C4        C3,G3,C4
	// 2 = sus2
	//  Shifts 2nd note down one degree
	//  C3,D3       C3,D3,G3        C3,D3,G3,B3
	// 3 = sus4
	//  Shifts 2nd note up one degree
	//  C3,F3       C3,F3,G3        C3,F3,G3,B3
	// 4 = sus2+4
	//  Shifts 2nd note down one degree and 3rd note down one degree
	//  C3,D3       C3,D3,F3        C3,D3,F3,B3
	// 5 = add 6
	//  C3,D3,A3    C3,E3,G3,A3     C3,E3,G3,A3
	// 6 = add 6 + 9
	//  C3,E3,A3,D4  C3,E3,G3,A3,D4  C3,E3,G3,A3,D4
	// 7 = kennyBarron11
	//  Two hand jazz voicing
	//  1,5,9,  10, 7th+oct,11+Oct
	//  C3,G3,D4,E4,B4,F5

	ChordSettings()
	{
		type = 0;
		midiFx = 0;
		mchan = 0;
		velocity = 100;

		note = 0;
		basicOct = 0;
		chord = 0;
		balance = 40; // Four note chord

		numNotes = 3;
		degree = 0;
		octave = 0;
		transpose = 0;
		spread = 0;
		rotate = 0;
		spreadUpDown = false;
		quartalVoicing = false;
		voicing = 0;

        for(uint8_t i = 0; i < 6; i++)
        {
            customNotes[i].note = 0;
        }
	}

	void CopySettingsFrom(ChordSettings *other)
	{
		this->type = other->type;
		this->midiFx = other->midiFx;
		this->mchan = other->mchan;
		this->velocity = other->velocity;

		// Basic Type:
		this->note = other->note;
		this->basicOct = other->basicOct;
		this->chord = other->chord;
		this->balance = other->balance;

		this->numNotes = other->numNotes;
		this->degree = other->degree;
		this->octave = other->octave;
		this->transpose = other->transpose;
		this->spread = other->spread;
		this->rotate = other->rotate;
		this->spreadUpDown = other->spreadUpDown;
		this->quartalVoicing = other->quartalVoicing;
		this->voicing = other->voicing;

        for(uint8_t i = 0; i < 6; i++)
        {
            this->customNotes[i].note = other->customNotes[i].note;
        }
	}
};

struct ChordNotes
{
	bool active = false;
	uint8_t channel = 0;
	// uint8_t velocity = 100;
	int notes[6] = {-1, -1, -1, -1, -1, -1};
	uint8_t velocities[6] = {100, 100, 100, 100, 100, 100};
	int8_t strumPos = 0;
	int8_t encDelta = 0;
	int8_t octIncrement = 0;
	uint8_t midifx;
	int rootNote;

	void CopyFrom(ChordNotes *other)
	{
		active = other->active;
		channel = other->channel;
		for (uint8_t i = 0; i < 6; i++)
		{
			notes[i] = other->notes[i];
			velocities[i] = other->velocities[i];
		}
		strumPos = other->strumPos;
		encDelta = other->encDelta;
		octIncrement = other->octIncrement;
		midifx = other->midifx;
		rootNote = other->rootNote;
	}
};

struct ChordBalanceDetails
{
	int8_t type[4];
	float velMult[4];

    void Clear()
    {
        for(uint8_t i = 0; i < 4; i++)
        {
            type[i] = 0;
            velMult[i] = 0;
        }
    }
};

enum ChordType
{
	CTYPE_BASIC, // Chords are copied from the Syntakt Chord machine, has a root, octave, scale, and ghosts. The ghosts determine number of notes in chord and notes will either be brought down or up and octave
	CTYPE_INTERVAL, // Advanced chord config using intervals, can be locked to a the global scale. 
	CTYPE_BYOC, // Build your own chord however you'd like. 
};
