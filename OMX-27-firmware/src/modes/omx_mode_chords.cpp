#include "omx_mode_chords.h"
#include "../config.h"
#include "../consts/colors.h"
#include "../utils/omx_util.h"
#include "../hardware/omx_disp.h"
#include "../hardware/omx_leds.h"
// #include "../sequencer.h"
#include "../midi/midi.h"
#include "../midi/noteoffs.h"

enum ChordsModePage
{
	CHRDPAGE_NOTES,
	CHRDPAGE_GBL1, // UI Mode
	// CHRDPAGE_GBL2, // Manual Strum, M-Chan,
	CHRDPAGE_OUTMIDI, // Oct, CH, Vel
    CHRDPAGE_POTSANDMACROS, // PotBank, Thru, Macro, Macro Channel
    CHRDPAGE_SCALES, // Root, Scale, Lock Scale Notes, Group notes. 
	CHRDPAGE_1,	   // Chord Type, MidiFX, 0, Midi Channel
	CHRDPAGE_2,	   // Note, Octave, Chord,         | numNotes, degree, octave, transpose
	CHRDPAGE_3,	   //                              | spread, rotate, voicing
	CHRDPAGE_4,	   //                              | spreadUpDown, quartalVoicing
};

enum ChordsModeParams
{
	CPARAM_UIMODE,
	// CPARAM_SCALE_ROOT,
	// CPARAM_SCALE_PAT,
	// CPARAM_SCALE_LOCK,
	// CPARAM_SCALE_GRP16,
	// CPARAM_GBL_OCT,
	// CPARAM_GBL_MCHAN,
	// CPARAM_GBL_VEL,
	CPARAM_MAN_STRUM,
	// CPARAM_GBL_POTCC,
	// CPARAM_GBL_PBANK,
	// CPARAM_GBL_MIDITHRU,
	// CPARAM_GBL_MIDIMACRO,
	// CPARAM_GBL_MACROCHAN,
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

// enum ChordsModeIntervalPage {
//     CHRDINTPAGE_NOTES,
//     CHRDINTPAGE_GBL1, // Root, Scale, Octave
//     CHRDINTPAGE_1, // numNotes, degree, octave, transpose
//     CHRDINTPAGE_2, // spread, rotate, voicing
//     CHRDINTPAGE_3, // spreadUpDown, quartalVoicing
// };

// enum ChordsModeBasicPage {
//     CHRDBASPAGE_NOTES,
//     CHRDBASPAGE_GBL1, // Root, Scale, Octave
//     CHRDBASPAGE_1, // numNotes, degree, octave, transpose
//     CHRDBASPAGE_2, // spread, rotate, voicing
//     CHRDBASPAGE_3, // spreadUpDown, quartalVoicing
// };

enum ChordsUIModes
{
	CUIMODE_FULL,
	CUIMODE_SPLIT
};

const char *kUIModeDisp[8] = {"FULL", "SPLT"};

enum ChordsMainMode
{
	CHRDMODE_PLAY, // Play Chords
	CHRDMODE_EDIT, // Play Chords, jumps to edit page
	CHRDMODE_PRESET, // Loads groups of chord presets
	CHRDMODE_MANSTRUM, // Manually strum chords using the encoder
};

enum ChordType
{
	CTYPE_BASIC, // Chords are copied from the Syntakt Chord machine, has a root, octave, scale, and ghosts. The ghosts determine number of notes in chord and notes will either be brought down or up and octave
	CTYPE_INTERVAL, // Advanced chord config using intervals, can be locked to a the global scale. 
	CTYPE_BYOC, // Build your own chord however you'd like. 
};

// const int chordPatterns[16][3] = {
// 	{ -1, -1, -1 }, // 0:  N/A
// 	{ 4, 7, -1 },   // 1:  MAJ
// 	{ 3, 7, -1 },   // 2:  MIN
// 	{ 4, 7, 11 },   // 3:  MAJ7
// 	{ 3, 7, 10 },   // 4:  MIN7
// 	{ 4, 7, 10 },   // 5:  7
// 	{ 2, 7, -1 },   // 6:  SUS2
// 	{ 5, 7, -1 },   // 7:  SUS4
// 	{ 4, 8, -1 },   // 8:  AUG
// 	{ 3, 6, -1 },   // 9:  DIM
// 	{ 3, 6, 10 },   // 10: HDIM
// 	{ 7, -1, -1 },  // 11: 5
// 	{ 4, 11, 14 },  // 12: MAJ9
// 	{ 3, 10, 14 },  // 13: MIN9
// 	{ 4, 10, 14 },  // 14: 9
// 	{ 5, 7, 11 },    // 15: 7SUS4
// };

// minor
// major
// sus2
// sus4
// m7
// M7
// hMaj7
// Maj7
// 7sus4
// dim7
// madd9 or hadd9
// Madd9
// m6
// M6
// mb5
// Mb5
// m7b5
// M7b5
// M#5
// m7#5
// M7#5
// mb6
// m9nos
// M9nos
// Madd9b5
// Maj7b5
// M7b9nos
// sus4#5b9
// sus4add#5
// Maddb5
// M6add4nos
// Maj7/6nos
// Maj9nos
// Fourths
// Fifths
// C C# D D# E F F# G G# A A# B  C  C# D  D#
// 0 1  2 3  4 5 6  7 8  9 10 11 12 13 14 15

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

int balSize = sizeof(chordBalance);
int patSize = sizeof(chordPatterns);

const char *kChordTypeDisp[8] = {"BASC", "INTV"};

const char *kVoicingNames[8] = {"NONE", "POWR", "SUS2", "SUS4", "SU24", "+6", "+6+9", "KB11"};

const int kDegreeColor = ORANGE;
const int kDegreeSelColor = 0xFFBF80;
const int kNumNotesColor = BLUE;
const int kNumNotesSelColor = 0x9C9CFF;
const int kSpreadUpDownOnColor = RED;
const int kSpreadUpDownOffColor = 0x550000;
const int kQuartalVoicingOnColor = MAGENTA;
const int kQuartalVoicingOffColor = 0x500050;
const int kOctaveColor = BLUE;
const int kTransposeColor = BLUE;
const int kSpreadColor = BLUE;
const int kRotateColor = BLUE;
const int kVoicingColor = BLUE;

const int kPlayColor = ORANGE;
const int kEditColor = DKRED;
const int kPresetColor = DKGREEN;

const int kChordEditNoteInScaleColor = 0x040404;
// const int kChordEditNoteRootColor = MAGENTA;
// const int kChordEditNoteChordColor = ORANGE;

// const uint16_t kChordEditNoteRootHue = MAGENTA;
const uint16_t kChordEditNoteChordHue = 5461; // Orange

OmxModeChords::OmxModeChords()
{
// 	enum ChordsModePage
// {
// 	CHRDPAGE_NOTES,
// 	CHRDPAGE_GBL1, // UI Mode
// 	// CHRDPAGE_GBL2, // Manual Strum, M-Chan,
// 	CHRDPAGE_OUTMIDI, // Oct, CH, Vel
//     CHRDPAGE_POTSANDMACROS, // PotBank, Thru, Macro, Macro Channel
//     CHRDPAGE_SCALES, // Root, Scale, Lock Scale Notes, Group notes. 
// 	CHRDPAGE_1,	   // Chord Type, MidiFX, 0, Midi Channel
// 	CHRDPAGE_2,	   // Note, Octave, Chord,         | numNotes, degree, octave, transpose
// 	CHRDPAGE_3,	   //                              | spread, rotate, voicing
// 	CHRDPAGE_4,	   //                              | spreadUpDown, quartalVoicing
// };

	basicParams_.addPage(1);
	basicParams_.addPage(4);
	basicParams_.addPage(4);
	basicParams_.addPage(4);
	basicParams_.addPage(4);
	basicParams_.addPage(4);
	basicParams_.addPage(4);
	basicParams_.addPage(6); // Custom chord notes

	intervalParams_.addPage(1);
	intervalParams_.addPage(4);
	intervalParams_.addPage(4);
	intervalParams_.addPage(4);
	intervalParams_.addPage(4);
	intervalParams_.addPage(4);
	intervalParams_.addPage(4);
	intervalParams_.addPage(4);
	intervalParams_.addPage(4);

	// 808 Colors
	for (uint8_t i = 0; i < 16; i++)
	{
		if (i >= 0 && i < 4)
		{
			chords_[i].color = RED; // Red
		}
		else if (i >= 4 && i < 8)
		{
			chords_[i].color = ORANGE; // Orange
		}
		else if (i >= 8 && i < 12)
		{
			chords_[i].color = YELLOW; // Yellow
		}
		else if (i >= 12)
		{
			chords_[i].color = 0xcfc08f; // Creme
		}
	}

	for (uint8_t i = 0; i < 16; i++)
	{
		chords_[i].type = CTYPE_BASIC;
		chords_[i].chord = i <= 7 ? 0 : 1; // Major left, minor right
		chords_[i].balance = 40;

		int adjnote = notes[i + 11] + (midiSettings.octave * 12);

		if (adjnote >= 0 && adjnote <= 127)
		{
			chords_[i].note = adjnote % 12;
			chords_[i].basicOct = (adjnote / 12) - 5;
		}
	}

	// save these to presets
	for (uint8_t i = 0; i < NUM_CHORD_SAVES; i++)
	{
		savePreset(i);
	}

	activeChordEditDegree_ = -1;
	activeChordEditNoteKey_ = -1;

	uiMode_ = CUIMODE_SPLIT;

	// chords_[0].numNotes = 3;
	// chords_[0].degree = 0;

	// chords_[1].numNotes = 3;
	// chords_[1].degree = 1;

	// chords_[2].numNotes = 4;
	// chords_[2].degree = 0;

	// chords_[3].numNotes = 4;
	// chords_[3].degree = 1;
}

void OmxModeChords::InitSetup()
{
}

void OmxModeChords::onModeActivated()
{
	basicParams_.setSelPageAndParam(0, 0);
	intervalParams_.setSelPageAndParam(0, 0);

	encoderSelect_ = true;
	heldChord_ = -1;
	activeChordEditDegree_ = -1;
	activeChordEditNoteKey_ = -1;

	lockScaleCache_ = scaleConfig.lockScale;
	grp16ScaleCache_ = scaleConfig.group16;

	scaleConfig.lockScale = false;
	scaleConfig.group16 = false;

	// sequencer.playing = false;
	stopSequencers();

	omxLeds.setDirty();
	omxDisp.setDirty();

	for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		subModeMidiFx[i].setEnabled(true);
		subModeMidiFx[i].setSelected(true);
		subModeMidiFx[i].onModeChanged();
		subModeMidiFx[i].setNoteOutputFunc(&OmxModeChords::onNotePostFXForwarder, this);
	}

	pendingNoteOffs.setNoteOffFunction(&OmxModeChords::onPendingNoteOffForwarder, this);

	selectMidiFx(mfxIndex_, false);
}

void OmxModeChords::onModeDeactivated()
{
	// sequencer.playing = false;
	stopSequencers();
	allNotesOff();

	for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		subModeMidiFx[i].setEnabled(false);
		subModeMidiFx[i].onModeChanged();
	}

	scaleConfig.lockScale = lockScaleCache_;
	scaleConfig.group16 = grp16ScaleCache_;
}

void OmxModeChords::stopSequencers()
{
	omxUtil.stopClocks();
	// MM::stopClock();
	pendingNoteOffs.allOff();
}

void OmxModeChords::selectMidiFx(uint8_t mfxIndex, bool dispMsg)
{
	this->mfxIndex_ = mfxIndex;

	for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		subModeMidiFx[i].setSelected(true);
	}

	if (dispMsg)
	{
		if (mfxIndex < NUM_MIDIFX_GROUPS)
		{
			omxDisp.displayMessageTimed("Key MFX " + String(mfxIndex + 1), 5);
		}
		else
		{
			omxDisp.displayMessageTimed("Key MFX Off", 5);
		}
	}
}

void OmxModeChords::selectMidiFxChordKey(int8_t mfxIndex, bool dispMsg)
{
    int8_t prevMidiFX = chords_[selectedChord_].midiFx;
    
    if(mfxIndex != prevMidiFX && (prevMidiFX >= 0 && prevMidiFX < NUM_MIDIFX_GROUPS))
    {
		onChordOff(selectedChord_);
    }

    chords_[selectedChord_].midiFx = mfxIndex;

	for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		subModeMidiFx[i].setSelected(i == mfxIndex);
	}

	if (dispMsg)
	{
		if (mfxIndex >= 0 && mfxIndex < NUM_MIDIFX_GROUPS)
		{
			omxDisp.displayMessageTimed("Chord MFX " + String(mfxIndex + 1), 5);
		}
		else
		{
			omxDisp.displayMessageTimed("Chord MFX Off", 5);
		}
	}
}

void OmxModeChords::onClockTick()
{
	for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		// Lets them do things in background
		subModeMidiFx[i].onClockTick();
	}
}

void OmxModeChords::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
	if (isSubmodeEnabled() && activeSubmode->usesPots())
	{
		activeSubmode->onPotChanged(potIndex, prevValue, newValue, analogDelta);
		return;
	}

	// Serial.println("onPotChanged: " + String(potIndex));
	if (chordEditMode_ == false && mode_ == CHRDMODE_MANSTRUM)
	{
		if (analogDelta < 3)
		{
			return;
		}

		// Serial.println("strum");

		if (potIndex == 0)
		{
			uint8_t oldV = manStrumSensit_;
			manStrumSensit_ = (uint8_t)map(newValue, 0, 127, 1, 32);
			if (manStrumSensit_ != oldV)
			{
				omxDisp.displayMessageTimed("Sens: " + String(manStrumSensit_), 5);
			}
		}
		else if (potIndex == 1)
		{
			bool oldV = wrapManStrum_;
			wrapManStrum_ = (bool)map(newValue, 0, 127, 0, 1);
			if (wrapManStrum_ != oldV)
			{
				if (wrapManStrum_)
				{
					omxDisp.displayMessageTimed("Wrap on", 5);
				}
				else
				{
					omxDisp.displayMessageTimed("Wrap off", 5);
				}
			}
		}
		else if (potIndex == 2)
		{
			uint8_t oldV = incrementManStrum_;
			incrementManStrum_ = (uint8_t)map(newValue, 0, 127, 0, 4);
			if (incrementManStrum_ != oldV)
			{
				omxDisp.displayMessageTimed("Increm: " + String(incrementManStrum_), 5);
			}
		}
		else if (potIndex == 3)
		{
			// Serial.println("length");

			uint8_t prevLength = manStrumNoteLength_;
			manStrumNoteLength_ = map(newValue, 0, 127, 0, kNumNoteLengths - 1);

			if (prevLength != manStrumNoteLength_)
			{
				omxDisp.displayMessageTimed(String(kNoteLengths[manStrumNoteLength_]), 10);
			}
		}

		omxDisp.setDirty();
		omxLeds.setDirty();
		return;
	}
	else
	{
		omxUtil.sendPots(potIndex, sysSettings.midiChannel);
		omxDisp.setDirty();
	}
	// if (potIndex < 4)
	// {
	//     if (analogDelta >= 10)
	//     {
	//         grids_.setDensity(potIndex, newValue * 2);

	//         if (params.getSelPage() == GRIDS_DENSITY)
	//         {
	//             setParam(potIndex);
	//             // setParam(GRIDS_DENSITY, potIndex + 1);
	//         }
	//     }

	//     omxDisp.setDirty();
	// }
	// else if (potIndex == 4)
	// {
	//     int newres = (float(newValue) / 128.f) * 3;
	//     grids_.setResolution(newres);
	//     if (newres != prevResolution_)
	//     {
	//         String msg = String(rateNames[newres]);
	//         omxDisp.displayMessageTimed(msg, 5);
	//     }
	//     prevResolution_ = newres;
	// }
}

void OmxModeChords::loopUpdate(Micros elapsedTime)
{
	updateFuncKeyMode();

	// if (elapsedTime > 0)
	// {
	// 	if (!sequencer.playing)
	// 	{
	//         // Needed to make pendingNoteOns/pendingNoteOffs work
	// 		omxUtil.advanceSteps(elapsedTime);
	// 	}
	// }

	for (uint8_t i = 0; i < 5; i++)
	{
		// Lets them do things in background
		subModeMidiFx[i].loopUpdate();
	}

	// Can be modified by scale MidiFX
	musicScale_->calculateScaleIfModified(scaleConfig.scaleRoot, scaleConfig.scalePattern);
}

void OmxModeChords::allNotesOff()
{
	omxUtil.allOff();
}

void OmxModeChords::updateFuncKeyMode()
{
	auto keyState = midiSettings.keyState;

	uint8_t prevMode = funcKeyMode_;

	funcKeyMode_ = FUNCKEYMODE_NONE;

	if (!auxDown_)
	{
		if (keyState[1] && !keyState[2])
		{
			funcKeyMode_ = FUNCKEYMODE_F1;
		}
		else if (!keyState[1] && keyState[2])
		{
			funcKeyMode_ = FUNCKEYMODE_F2;
		}
		else if (keyState[1] && keyState[2])
		{
			funcKeyMode_ = FUNCKEYMODE_F3;
		}
		else
		{
			funcKeyMode_ = FUNCKEYMODE_NONE;
		}
	}

	if (funcKeyMode_ != prevMode)
	{
		// omxUtil.allOff();

		omxDisp.setDirty();
		omxLeds.setDirty();
	}
}

void OmxModeChords::onEncoderChanged(Encoder::Update enc)
{
	if (isSubmodeEnabled())
	{
		activeSubmode->onEncoderChanged(enc);
		return;
	}

	if (chordEditMode_ == false && mode_ == CHRDMODE_MANSTRUM)
	{
		onEncoderChangedManStrum(enc);
		return;
	}

	auto params = getParams();

	if (getEncoderSelect())
	{
		params->changeParam(enc.dir());
		omxDisp.setDirty();
		return;
	}

	int8_t selPage = params->getSelPage();
	int8_t selParam = params->getSelParam() + 1; // Add one for readability

	// Global 1 - UI Mode, Root, Scale, Octave
	if (selPage == CHRDPAGE_GBL1)
	{
		onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_UIMODE);
		// onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_SCALE_ROOT);
		// onEncoderChangedEditParam(&enc, selParam, 3, CPARAM_SCALE_PAT);
		// onEncoderChangedEditParam(&enc, selParam, 4, CPARAM_GBL_OCT);
	}
	else if (selPage == CHRDPAGE_OUTMIDI) 
	{
		omxUtil.onEncoderChangedEditParam(&enc, selParam, 1, GPARAM_MOUT_OCT);
		omxUtil.onEncoderChangedEditParam(&enc, selParam, 2, GPARAM_MOUT_CHAN);
		omxUtil.onEncoderChangedEditParam(&enc, selParam, 3, GPARAM_MOUT_VEL);

		// onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_GBL_OCT);
		// onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_GBL_MCHAN);
		// onEncoderChangedEditParam(&enc, selParam, 3, CPARAM_GBL_VEL);
	}
	else if (selPage == CHRDPAGE_POTSANDMACROS)
	{
		omxUtil.onEncoderChangedEditParam(&enc, selParam, 1, GPARAM_POTS_PBANK);
		omxUtil.onEncoderChangedEditParam(&enc, selParam, 2, GPARAM_MIDI_THRU);
		omxUtil.onEncoderChangedEditParam(&enc, selParam, 3, GPARAM_MACRO_MODE);
		omxUtil.onEncoderChangedEditParam(&enc, selParam, 4, GPARAM_MACRO_CHAN);

		// onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_GBL_PBANK);
		// onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_GBL_MIDITHRU);
		// onEncoderChangedEditParam(&enc, selParam, 3, CPARAM_GBL_MIDIMACRO);
		// onEncoderChangedEditParam(&enc, selParam, 4, CPARAM_GBL_MACROCHAN);
	}
	else if (selPage == CHRDPAGE_SCALES)
	{
		omxUtil.onEncoderChangedEditParam(&enc, musicScale_, selParam, 1, GPARAM_SCALE_ROOT);
		omxUtil.onEncoderChangedEditParam(&enc, musicScale_, selParam, 2, GPARAM_SCALE_PAT);
		omxUtil.onEncoderChangedEditParam(&enc, musicScale_, selParam, 3, GPARAM_SCALE_LOCK);
		omxUtil.onEncoderChangedEditParam(&enc, musicScale_, selParam, 4, GPARAM_SCALE_GRP16);

		// onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_SCALE_ROOT);
		// onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_SCALE_PAT);
		// onEncoderChangedEditParam(&enc, selParam, 3, CPARAM_SCALE_LOCK);
		// onEncoderChangedEditParam(&enc, selParam, 4, CPARAM_SCALE_GRP16);
	}
	// else if (selPage == CHRDPAGE_GBL2)
	// {
	// 	onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_MAN_STRUM);
	// 	onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_GBL_MCHAN);
	// 	onEncoderChangedEditParam(&enc, selParam, 4, CPARAM_GBL_MCHAN);
	// }
	// PAGE ONE - Chord Type, MidiFX, 0, Midi Channel
	else if (selPage == CHRDPAGE_1)
	{
		onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_CHORD_TYPE);
		onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_CHORD_MFX);
		onEncoderChangedEditParam(&enc, selParam, 3, CPARAM_CHORD_VEL);
		onEncoderChangedEditParam(&enc, selParam, 4, CPARAM_CHORD_MCHAN);
	}
	// PAGE TWO - Basic: Note, Octave, Chord    Interval: numNotes, degree, octave, transpose
	else if (selPage == CHRDPAGE_2)
	{
		if (chords_[selectedChord_].type == CTYPE_INTERVAL)
		{
			onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_INT_NUMNOTES);
			onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_INT_DEGREE);
			onEncoderChangedEditParam(&enc, selParam, 3, CPARAM_INT_OCTAVE);
			onEncoderChangedEditParam(&enc, selParam, 4, CPARAM_INT_TRANSPOSE);
		}
		else if (chords_[selectedChord_].type == CTYPE_BASIC)
		{
			onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_BAS_NOTE);
			onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_BAS_OCT);
			onEncoderChangedEditParam(&enc, selParam, 3, CPARAM_BAS_BALANCE);
			onEncoderChangedEditParam(&enc, selParam, 4, CPARAM_BAS_CHORD);
		}
	}
	// PAGE THREE - spread, rotate, voicing
	else if (selPage == CHRDPAGE_3)
	{
		if (chords_[selectedChord_].type == CTYPE_INTERVAL)
		{
			onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_INT_SPREAD);
			onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_INT_ROTATE);
			onEncoderChangedEditParam(&enc, selParam, 3, CPARAM_INT_VOICING);
		}
		else if (chords_[selectedChord_].type == CTYPE_BASIC)
		{
			auto amtSlow = enc.accel(1);
			int8_t sel = params->getSelParam();
			chords_[selectedChord_].customNotes[sel].note = constrain(chords_[selectedChord_].customNotes[sel].note + amtSlow, -48, 48);

			if (amtSlow != 0) // To see notes change on keyboard leds
			{
				constructChord(selectedChord_);
			}
		}
	}
	// PAGE FOUR - spreadUpDown, quartalVoicing
	else if (selPage == CHRDPAGE_4)
	{
		if (chords_[selectedChord_].type == CTYPE_INTERVAL)
		{
			onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_INT_SPRDUPDOWN);
			onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_INT_QUARTVOICE);
		}
	}

	omxDisp.setDirty();
	omxLeds.setDirty();
}

// Put all params here to make it easy to switch order in pages
void OmxModeChords::onEncoderChangedEditParam(Encoder::Update *enc, uint8_t selectedParmIndex, uint8_t targetParamIndex, uint8_t paramType)
{
	if (selectedParmIndex != targetParamIndex)
		return;

	auto amtSlow = enc->accel(1);
	auto amtFast = enc->accel(5);

	bool triggerChord = false;

	switch (paramType)
	{
	case CPARAM_UIMODE:
	{
		uiMode_ = constrain(uiMode_ + amtSlow, 0, 1);
		if (amtSlow != 0)
		{
			allNotesOff();
			// omxUtil.allOff();
		}
	}
	break;
	case CPARAM_MAN_STRUM:
	{
		if (mode_ == CHRDMODE_MANSTRUM)
		{
			if (enc->dir() < 0)
			{
				mode_ = CHRDMODE_PLAY;
			}
		}
		else
		{
			if (enc->dir() > 0)
			{
				mode_ = CHRDMODE_MANSTRUM;
			}
		}
	}
	break;
	case CPARAM_CHORD_TYPE:
	{
		if (amtSlow != 0)
		{
			if (chordEditMode_)
			{
				onChordEditOff();
				enterChordEditMode();
			}
			else
			{
				onChordOff(selectedChord_);
			}
		}

		chords_[selectedChord_].type = constrain(chords_[selectedChord_].type + amtSlow, 0, 1);
	}
	break;
	case CPARAM_CHORD_MFX:
	{
		int8_t newMidiFx = constrain(chords_[selectedChord_].midiFx + amtSlow, -1, NUM_MIDIFX_GROUPS - 1);
		selectMidiFxChordKey(newMidiFx, false);
	}
	break;
	case CPARAM_CHORD_VEL:
	{
		chords_[selectedChord_].velocity = constrain(chords_[selectedChord_].velocity + amtFast, 0, 127);
	}
	break;
	case CPARAM_CHORD_MCHAN:
	{
		chords_[selectedChord_].mchan = constrain(chords_[selectedChord_].mchan + amtSlow, 0, 15);
	}
	break;
	case CPARAM_BAS_NOTE:
	{
		chords_[selectedChord_].note = constrain(chords_[selectedChord_].note + amtSlow, 0, 11);
		triggerChord = amtSlow != 0;
	}
	break;
	case CPARAM_BAS_OCT:
	{
		chords_[selectedChord_].basicOct = constrain(chords_[selectedChord_].basicOct + amtSlow, -5, 4);
		triggerChord = amtSlow != 0;
	}
	break;
	case CPARAM_BAS_CHORD:
	{
		uint8_t prevChord = chords_[selectedChord_].chord;
		chords_[selectedChord_].chord = constrain(chords_[selectedChord_].chord + amtSlow, 0, kNumChordPatterns - 1);
		if (chords_[selectedChord_].chord != prevChord)
		{
			triggerChord = true;

			// constructChord(selectedChord_);
			// omxDisp.displayMessage(kChordMsg[chords_[selectedChord_].chord]);
		}
	}
	break;
	case CPARAM_BAS_BALANCE:
	{
		chords_[selectedChord_].balance = constrain(chords_[selectedChord_].balance + amtFast, 0, (kNumChordBalance - 1) * 10);
		activeChordBalance_ = getChordBalanceDetails(chords_[selectedChord_].balance);

		// omxDisp.chordBalanceMsg(activeChordBalance_.type, activeChordBalance_.velMult, 10);

		if (amtSlow != 0) // To see notes change on keyboard leds
		{
			constructChord(selectedChord_);
		}
	}
	break;
	case CPARAM_INT_NUMNOTES:
	{
		chords_[selectedChord_].numNotes = constrain(chords_[selectedChord_].numNotes + amtSlow, 1, 4);
	}
	break;
	case CPARAM_INT_DEGREE:
	{
		chords_[selectedChord_].degree = constrain(chords_[selectedChord_].degree + amtSlow, 0, 7);
	}
	break;
	case CPARAM_INT_OCTAVE:
	{
		chords_[selectedChord_].octave = constrain(chords_[selectedChord_].octave + amtSlow, -2, 2);
	}
	break;
	case CPARAM_INT_TRANSPOSE:
	{
		chords_[selectedChord_].transpose = constrain(chords_[selectedChord_].transpose + amtSlow, -7, 7);
	}
	break;
	case CPARAM_INT_SPREAD:
	{
		chords_[selectedChord_].spread = constrain(chords_[selectedChord_].spread + amtSlow, -2, 2);
	}
	break;
	case CPARAM_INT_ROTATE:
	{
		chords_[selectedChord_].rotate = constrain(chords_[selectedChord_].rotate + amtSlow, 0, 4);
	}
	break;
	case CPARAM_INT_VOICING:
	{
		chords_[selectedChord_].voicing = constrain(chords_[selectedChord_].voicing + amtSlow, 0, 7);
	}
	break;
	case CPARAM_INT_SPRDUPDOWN:
	{
		chords_[selectedChord_].spreadUpDown = constrain(chords_[selectedChord_].spreadUpDown + amtSlow, 0, 1);
	}
	break;
	case CPARAM_INT_QUARTVOICE:
	{
		chords_[selectedChord_].quartalVoicing = constrain(chords_[selectedChord_].quartalVoicing + amtSlow, 0, 1);
	}
	break;
	}

	// Play chord if value changes
	if (triggerChord)
	{
		if (mode_ == CHRDMODE_EDIT || chordEditMode_)
		{
			if (!chordEditMode_ && heldChord_ == selectedChord_)
			{
				onChordOff(selectedChord_);
				onChordOn(selectedChord_);
			}
			else if (chordEditMode_ && activeChordEditNoteKey_ >= 0)
			{
				onChordEditOff();
				onChordEditOn(selectedChord_);
			}
			else
			{
				constructChord(selectedChord_);
			}
		}
		else
		{
			constructChord(selectedChord_);
		}
	}
}

void OmxModeChords::onEncoderChangedManStrum(Encoder::Update enc)
{
	if (chordNotes_[selectedChord_].active == false)
		return;

	auto amt = enc.accel(1);

	// Serial.println("EncDelta: " + String(chordNotes_[selectedChord_].encDelta));

	chordNotes_[selectedChord_].encDelta = chordNotes_[selectedChord_].encDelta + amt;

	if (abs(chordNotes_[selectedChord_].encDelta) >= manStrumSensit_)
	{

		uint8_t numNotes = 0;

		for (uint8_t i = 0; i < 6; i++)
		{
			if (chordNotes_[selectedChord_].notes[i] >= 0)
			{
				numNotes++;
			}
		}

		// Serial.println("Do Note");
		uint8_t velocity = midiSettings.defaultVelocity;

		int8_t strumPos = chordNotes_[selectedChord_].strumPos;

		// Serial.println("strumPos: " + String(strumPos));

		if (strumPos >= 0 && strumPos < numNotes)
		{
			int note = chordNotes_[selectedChord_].notes[strumPos] + (chordNotes_[selectedChord_].octIncrement * 12);

			if (note >= 0 && note <= 127)
			{
				uint32_t noteOnMicros = micros();
				float noteLength = kNoteLengths[manStrumNoteLength_];
				uint32_t noteOffMicros = noteOnMicros + (noteLength * clockConfig.step_micros);

				pendingNoteOns.insert(note, velocity, chordNotes_[selectedChord_].channel, noteOnMicros, false);
				pendingNoteOffs.insert(note, chordNotes_[selectedChord_].channel, noteOffMicros, false);

				omxDisp.displayMessage(musicScale_->getFullNoteName(note));
				omxDisp.setDirty();
				omxLeds.setDirty();
			}
		}

		if (chordNotes_[selectedChord_].encDelta > 0)
		{
			strumPos++;
		}
		else
		{
			strumPos--;
		}

		if (wrapManStrum_)
		{
			if (strumPos >= numNotes)
			{
				chordNotes_[selectedChord_].octIncrement++;
				if (chordNotes_[selectedChord_].octIncrement > incrementManStrum_)
				{
					chordNotes_[selectedChord_].octIncrement = 0;
				}
				strumPos = 0;
			}
			if (strumPos < 0)
			{

				chordNotes_[selectedChord_].octIncrement--;
				if (chordNotes_[selectedChord_].octIncrement < -incrementManStrum_)
				{
					chordNotes_[selectedChord_].octIncrement = 0;
				}
				strumPos = numNotes - 1;
			}

			chordNotes_[selectedChord_].strumPos = strumPos;
		}
		else
		{
			chordNotes_[selectedChord_].strumPos = constrain(strumPos, -1, 6); // Allow to be one outside of notes
		}

		// chordNotes_[selectedChord_].octIncrement = constrain(chordNotes_[selectedChord_].octIncrement, -8, 8);

		chordNotes_[selectedChord_].encDelta = 0;
	}
}

void OmxModeChords::onEncoderButtonDown()
{
	if (isSubmodeEnabled())
	{
		activeSubmode->onEncoderButtonDown();
		return;
	}

	encoderSelect_ = !encoderSelect_;
	omxDisp.setDirty();
}

void OmxModeChords::onEncoderButtonDownLong()
{
}

bool OmxModeChords::shouldBlockEncEdit()
{
	if (isSubmodeEnabled())
	{
		return activeSubmode->shouldBlockEncEdit();
	}

	return false;
}

void OmxModeChords::onKeyUpdate(OMXKeypadEvent e)
{
	uint8_t thisKey = e.key();

	if (isSubmodeEnabled())
	{
		if (activeSubmode->onKeyUpdate(e))
			return;
	}

	if (chordEditMode_)
	{
		onKeyUpdateChordEdit(e);
		return;
	}

	if (onKeyUpdateSelMidiFX(e))
		return;

	if (e.held())
		return;

	// auto keyState = midiSettings.keyState;

	auto params = getParams();

	// AUX KEY
	if (thisKey == 0)
	{
		if (e.down())
		{
			auxDown_ = true;
		}
		else
		{
			auxDown_ = false;

			// Forces all arps to work.
			for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
			{
				subModeMidiFx[i].setSelected(true);
			}
		}
		omxLeds.setDirty();
		omxDisp.setDirty();
		return;
	}

	if (auxDown_) // Aux mode
	{
		if (e.down())
		{
			if (thisKey == 11 || thisKey == 12) // Change Octave
			{
				int amt = thisKey == 11 ? -1 : 1;
				midiSettings.octave = constrain(midiSettings.octave + amt, -5, 4);
			}
			else if (thisKey == 1 || thisKey == 2) // Change Param selection
			{
				if (thisKey == 1)
				{
					params->decrementParam();
				}
				else if (thisKey == 2)
				{
					params->incrementParam();
				}
			}
		}
	}
	else
	{
		bool keyConsumed = false;
		if ((mode_ == CHRDMODE_PLAY || mode_ == CHRDMODE_EDIT) && uiMode_ == CUIMODE_SPLIT)
		{
			// Split UI Mode
			if (thisKey >= 19 || (thisKey >= 6 && thisKey < 11)) // Check if key is on right side starting from C2(19)
			{
				keyConsumed = true;

				uint8_t adjKeyIndex = thisKey >= 19 ? thisKey - 7 : thisKey - 5; // Pretends keys are down an octave

				// If we're in edit mode and holding down a chord of the basic type, then we can edit the chord
				// rather than play a note
				if (mode_ == CHRDMODE_EDIT && heldChord_ >= 0 && chords_[heldChord_].type == CTYPE_BASIC)
				{
					if (e.down())
					{
						int adjnote = notes[adjKeyIndex] + (midiSettings.octave * 12);

						// If valid note, we edit the chord setting the root note and octave, stop currently playing chord
						// and turn on new chords
						if (adjnote >= 0 && adjnote <= 127)
						{
							onChordOff(selectedChord_);
							// delay(10);
							onChordEditOff();
							// delay(10);
							chords_[selectedChord_].note = adjnote % 12;
							chords_[selectedChord_].basicOct = (adjnote / 12) - 5;
							activeChordEditNoteKey_ = thisKey;
							onChordEditOn(selectedChord_);
						}
					}
					else
					{
						if (thisKey == activeChordEditNoteKey_)
						{
							onChordEditOff();
							activeChordEditNoteKey_ = -1;
						}
					}
				}
				// Not holding a chord in edit mode, play a note
				else
				{
					activeChordEditNoteKey_ = -1;
					lastKeyWasKeyboard_ = true;

					if (e.down())
					{
						splitNoteOn(adjKeyIndex);
					}
					else
					{
						splitNoteOff(adjKeyIndex);
					}
				}
			}
		}

		//       [F1][F2]    [PLAY][EDIT][PRESET]  [STRUM]
		// [C1][C2][C3][C4][C5]  [C6]  [C7]

		if (!keyConsumed)
		{
			if (funcKeyMode_ == FUNCKEYMODE_NONE)
			{
				// Key Down, no function keys, no aux
				if (e.down())
				{
					if (thisKey == 3)
					{
						mode_ = CHRDMODE_PLAY;
						setSelPageAndParam(CHRDPAGE_NOTES, 0);
						encoderSelect_ = true;
						omxDisp.displayMessage("Play");
					} 
					else if (thisKey == 4)
					{
						mode_ = CHRDMODE_EDIT;
						setSelPageAndParam(CHRDPAGE_2, 0);
						encoderSelect_ = true;
						omxDisp.displayMessage("Edit");
						allNotesOff();
					}
					else if (thisKey == 5)
					{
						mode_ = CHRDMODE_PRESET;
						omxDisp.displayMessage("Preset");
						allNotesOff();
					}
					else if (thisKey == 6)
					{
						mode_ = CHRDMODE_MANSTRUM;
						omxDisp.displayMessage("Manual Strum");
						allNotesOff();
					}
					if (thisKey >= 11)
					{
						if (mode_ == CHRDMODE_PLAY) // Play
						{
							selectedChord_ = thisKey - 11;
							heldChord_ = thisKey - 11;
							lastKeyWasKeyboard_ = false;
							onChordOn(thisKey - 11);
						}
						else if (mode_ == CHRDMODE_EDIT) // Edit
						{
							selectedChord_ = thisKey - 11;
							heldChord_ = thisKey - 11;
							lastKeyWasKeyboard_ = false;
							onChordOn(thisKey - 11);
						}
						else if (mode_ == CHRDMODE_PRESET) // Preset
						{
							selectedChord_ = thisKey - 11;
							heldChord_ = thisKey - 11;
							lastKeyWasKeyboard_ = false;
							onChordOn(thisKey - 11);
						}
						else if (mode_ == CHRDMODE_MANSTRUM) // Manual Strum
						{
							// Enter chord edit mode
							selectedChord_ = thisKey - 11;
							heldChord_ = thisKey - 11;
							lastKeyWasKeyboard_ = false;
							onManualStrumOn(selectedChord_);
							return;
						}
					}
				}
				else
				{
					if (thisKey >= 11)
					{
						if (thisKey - 11 == heldChord_)
						{
							heldChord_ = -1;
						}

						onChordOff(thisKey - 11);
					}
				}
			}
			else // Function key held
			{
				// Alt way to enter manual strum useful in split screen view
				// if(mode_ == CHRDMODE_PLAY && funcKeyMode_ == FUNCKEYMODE_F1 && e.down() && thisKey == 3)
				if(funcKeyMode_ == FUNCKEYMODE_F1 && e.down() && thisKey == 3)
				{
					mode_ = CHRDMODE_MANSTRUM;
					omxDisp.displayMessage("Manual Strum");
					allNotesOff();
					return;
				}

				if (e.down() && thisKey >= 11)
				{
					// --- PLAY MODE ---
					if (mode_ == CHRDMODE_PLAY)
					{
						// if (funcKeyMode_ == FUNCKEYMODE_F1)
						// {
						// }
						// else if (funcKeyMode_ == FUNCKEYMODE_F2)
						// {
						// 	if (pasteSelectedChordTo(thisKey - 11))
						// 	{
						// 		omxDisp.displayMessageTimed("Copied to " + String(thisKey - 11), 5);
						// 	}
						// }
					}
					// --- EDIT MODE ---
					else if (mode_ == CHRDMODE_EDIT)
					{
						if (funcKeyMode_ == FUNCKEYMODE_F1)
						{
							selectedChord_ = thisKey - 11;
							lastKeyWasKeyboard_ = false;
							enterChordEditMode();
							return;
						}
						else if (funcKeyMode_ == FUNCKEYMODE_F2)
						{
							if (pasteSelectedChordTo(thisKey - 11))
							{
								omxDisp.displayMessageTimed("Copied to " + String(thisKey - 11), 5);
							}
						}
					}
					// --- PRESET MODE ---
					else if (mode_ == CHRDMODE_PRESET)
					{
						if (funcKeyMode_ == FUNCKEYMODE_F1)
						{
							// Autosave your current preset unless you are reloading the current preset
							if(thisKey - 11 != selectedSave_)
							{
								savePreset(selectedSave_);
							}
							if (loadPreset(thisKey - 11))
							{
								omxDisp.displayMessageTimed("Load " + String(thisKey - 11), 5);
							}
						}
						else if (funcKeyMode_ == FUNCKEYMODE_F2)
						{
							if (savePreset(thisKey - 11))
							{
								omxDisp.displayMessageTimed("Saved to " + String(thisKey - 11), 5);
							}
						}
					}
					// --- STRUM MODE ---
					else if (mode_ == CHRDMODE_MANSTRUM) // Manual Strum
					{
						// if (funcKeyMode_ == FUNCKEYMODE_F1)
						// {
						// 	selectedChord_ = thisKey - 11;
						// 	enterChordEditMode();
						// 	return;
						// }
						// else if (funcKeyMode_ == FUNCKEYMODE_F2)
						// {
						// 	if (pasteSelectedChordTo(thisKey - 11))
						// 	{
						// 		omxDisp.displayMessageTimed("Copied to " + String(thisKey - 11), 5);
						// 	}
						// }
					}
				}
			}
		}
	}

	omxLeds.setDirty();
	omxDisp.setDirty();
}

void OmxModeChords::onKeyUpdateChordEdit(OMXKeypadEvent e)
{
	if (e.held())
		return;

	uint8_t thisKey = e.key();

	getParams(); // Sync params;

	// auto params = getParams();

	// AUX KEY
	if (thisKey == 0)
	{
		if (e.down())
		{
			// Exit Chord Edit Mode
			onChordEditOff();
			if (mode_ == CHRDMODE_PLAY)
			{
				setSelPageAndParam(CHRDPAGE_NOTES, 0);
			}

			encoderSelect_ = true;
			chordEditMode_ = false;
			activeChordEditDegree_ = -1;
			activeChordEditNoteKey_ = -1;
		}

		omxLeds.setDirty();
		omxDisp.setDirty();
		return;
	}

	if (chords_[selectedChord_].type == CTYPE_INTERVAL)
	{
		if (e.down())
		{
			if (chordEditParam_ == 0)
			{
				if (thisKey == 1) // Select Root
				{
					setSelPageAndParam(CHRDPAGE_GBL1, 1);
					encoderSelect_ = false;
				}
				if (thisKey == 2) // Select Scale
				{
					setSelPageAndParam(CHRDPAGE_SCALES, 2);
					encoderSelect_ = false;
				}
				if (thisKey == 3) // Octave
				{
					chordEditParam_ = 1;
					setSelPageAndParam(CHRDPAGE_2, 2);
					encoderSelect_ = false;
				}
				else if (thisKey == 4) // Transpose
				{
					chordEditParam_ = 2;
					setSelPageAndParam(CHRDPAGE_2, 3);
					encoderSelect_ = false;
				}
				else if (thisKey == 5) // Spread
				{
					chordEditParam_ = 3;
					setSelPageAndParam(CHRDPAGE_3, 0);
					encoderSelect_ = false;
				}
				else if (thisKey == 6) // Rotate
				{
					chordEditParam_ = 4;
					setSelPageAndParam(CHRDPAGE_3, 1);
					encoderSelect_ = false;
				}
				else if (thisKey == 7) // Voicing
				{
					chordEditParam_ = 5;
					setSelPageAndParam(CHRDPAGE_3, 2);
					encoderSelect_ = false;
				}
				else if (thisKey == 10) // Show Chord Notes
				{
					setSelPageAndParam(CHRDPAGE_NOTES, 0);
					encoderSelect_ = true;
				}
				else if (thisKey >= 11 && thisKey < 15) // Num of Notes
				{
					chords_[selectedChord_].numNotes = (thisKey - 11) + 1;
					setSelPageAndParam(CHRDPAGE_2, 0);
					encoderSelect_ = false;
				}
				else if (thisKey == 15) // Spread Up Down
				{
					chords_[selectedChord_].spreadUpDown = !chords_[selectedChord_].spreadUpDown;
					setSelPageAndParam(CHRDPAGE_4, 0);
					encoderSelect_ = false;
					omxDisp.displayMessage(chords_[selectedChord_].spreadUpDown ? "SpdUpDn On" : "SpdUpDn Off");
				}
				else if (thisKey == 16) // Quartal Voicing
				{
					chords_[selectedChord_].quartalVoicing = !chords_[selectedChord_].quartalVoicing;
					setSelPageAndParam(CHRDPAGE_4, 1);
					encoderSelect_ = false;
					omxDisp.displayMessage(chords_[selectedChord_].quartalVoicing ? "Quartal On" : "Quartal Off");
				}
				else if (thisKey >= 19)
				{
					chords_[selectedChord_].degree = thisKey - 19;
					// params_.setSelPageAndParam(CHRDPAGE_2, 1);
					// encoderSelect_ = false;
					onChordEditOff();
					onChordEditOn(selectedChord_);
					activeChordEditDegree_ = thisKey - 19;
				}
			}
			else if (chordEditParam_ == 1) // Octave
			{
				// chords_[selectedChord_].octave = constrain(chords_[selectedChord_].octave + amt, -2, 2);
				if (thisKey >= 11 && thisKey <= 15)
				{
					chords_[selectedChord_].octave = thisKey - 11 - 2;
				}
			}
			else if (chordEditParam_ == 2) // Transpose
			{
				// chords_[selectedChord_].transpose = constrain(chords_[selectedChord_].transpose + amt, -7, 7);
				if (thisKey >= 11 && thisKey <= 25)
				{
					chords_[selectedChord_].transpose = thisKey - 11 - 7;
				}
			}
			else if (chordEditParam_ == 3) // Spread
			{
				// chords_[selectedChord_].spread = constrain(chords_[selectedChord_].spread + amt, -2, 2);
				if (thisKey >= 11 && thisKey <= 15)
				{
					chords_[selectedChord_].spread = thisKey - 11 - 2;
				}
			}
			else if (chordEditParam_ == 4) // Rotate
			{
				// chords_[selectedChord_].rotate = constrain(chords_[selectedChord_].rotate + amt, 0, 4);
				if (thisKey >= 11 && thisKey <= 15)
				{
					chords_[selectedChord_].rotate = thisKey - 11;
				}
			}
			else if (chordEditParam_ == 5) // Voicing
			{
				// chords_[selectedChord_].voicing = constrain(chords_[selectedChord_].octave + amt, 0, 7);
				if (thisKey >= 11 && thisKey <= 18)
				{
					chords_[selectedChord_].voicing = thisKey - 11;
				}
			}
		}
		else
		{
			if (thisKey >= 3 && thisKey <= 7)
			{
				chordEditParam_ = 0;
			}
			else if (thisKey >= 19)
			{
				if (thisKey - 19 == activeChordEditDegree_)
				{
					onChordEditOff();
					activeChordEditDegree_ = -1;
				}
			}
		}
	}
	else if (chords_[selectedChord_].type == CTYPE_BASIC)
	{
		if (e.down())
		{
			if (thisKey == 11 || thisKey == 26)
			{
				// Change octave
				int amt = thisKey == 11 ? -1 : 1;
				midiSettings.octave = constrain(midiSettings.octave + amt, -5, 4);
			}
			else
			{
				int adjnote = notes[thisKey] + (midiSettings.octave * 12);

				if (adjnote >= 0 && adjnote <= 127)
				{
					chords_[selectedChord_].note = adjnote % 12;
					chords_[selectedChord_].basicOct = (adjnote / 12) - 5;
					activeChordEditNoteKey_ = thisKey;
					onChordEditOff();
					onChordEditOn(selectedChord_);
				}
			}
		}
		else
		{
			if (thisKey == activeChordEditNoteKey_)
			{
				onChordEditOff();
				activeChordEditNoteKey_ = -1;
			}
		}
	}

	omxLeds.setDirty();
	omxDisp.setDirty();
}

void OmxModeChords::enterChordEditMode()
{
	omxDisp.displayMessageTimed("Editing " + String(selectedChord_), 5);
	constructChord(selectedChord_);

	allNotesOff();

	chordEditMode_ = true;
	chordEditParam_ = 0;
	heldChord_ = -1;
	activeChordEditDegree_ = -1;
	activeChordEditNoteKey_ = -1;
	setSelPageAndParam(CHRDPAGE_2, 0);
	encoderSelect_ = true;
	omxLeds.setDirty();
	omxDisp.setDirty();
}

void OmxModeChords::onKeyHeldUpdate(OMXKeypadEvent e)
{
	if (isSubmodeEnabled())
	{
		if (activeSubmode->onKeyHeldUpdate(e))
			return;
	}

	if (onKeyHeldSelMidiFX(e))
		return;
}

void OmxModeChords::enableSubmode(SubmodeInterface *subMode)
{
	if (activeSubmode != nullptr)
	{
		activeSubmode->setEnabled(false);
	}

	auxDown_ = false;

	activeSubmode = subMode;
	activeSubmode->setEnabled(true);
	omxDisp.setDirty();
}
void OmxModeChords::disableSubmode()
{
	if (activeSubmode != nullptr)
	{
		activeSubmode->setEnabled(false);
	}

	activeSubmode = nullptr;
	omxDisp.setDirty();
}

bool OmxModeChords::isSubmodeEnabled()
{
	if (activeSubmode == nullptr)
		return false;

	if (activeSubmode->isEnabled() == false)
	{
		disableSubmode();
		auxDown_ = false;
		return false;
	}

	return true;
}

bool OmxModeChords::getEncoderSelect()
{
	if (chordEditMode_)
	{
		return encoderSelect_ && !auxDown_ && activeChordEditDegree_ < 0 && activeChordEditNoteKey_ < 0;
	}

	return encoderSelect_ && !auxDown_ && heldChord_ < 0;
}

ParamManager *OmxModeChords::getParams()
{
	if (chords_[selectedChord_].type == CTYPE_BASIC)
	{
		basicParams_.setPageEnabled(CHRDPAGE_3, chords_[selectedChord_].chord == kCustomChordPattern);
		intervalParams_.setSelPageAndParam(basicParams_.getSelPage(), basicParams_.getSelParam());

		return &basicParams_;
	}
	else
	{
		basicParams_.setSelPageAndParam(intervalParams_.getSelPage(), intervalParams_.getSelParam());
		return &intervalParams_;
	}
}

void OmxModeChords::setSelPageAndParam(int8_t newPage, int8_t newParam)
{
	auto params = getParams();
	params->setSelPageAndParam(newPage, newParam);
	getParams(); // to sync the params
}

bool OmxModeChords::onKeyUpdateSelMidiFX(OMXKeypadEvent e)
{
	int thisKey = e.key();

	bool keyConsumed = false;

	uint8_t mfxdex = lastKeyWasKeyboard_ ? mfxIndex_ : chords_[selectedChord_].midiFx;

	if (!e.held())
	{
		if (!e.down() && e.clicks() == 2 && thisKey >= 6 && thisKey < 11)
		{
			if (auxDown_) // Aux mode
			{
				enableSubmode(&subModeMidiFx[thisKey - 6]);
				keyConsumed = true;
			}
		}

		if (e.down() && thisKey != 0)
		{
			if (auxDown_) // Aux mode
			{
				if (thisKey == 5)
				{
					keyConsumed = true;
					// Turn off midiFx
					if (lastKeyWasKeyboard_)
					{
						selectMidiFx(127, true);
					}
					else
					{
						selectMidiFxChordKey(127, true);
					}
				}
				else if (thisKey >= 6 && thisKey < 11)
				{
					keyConsumed = true;
					if (lastKeyWasKeyboard_)
					{
						selectMidiFx(thisKey - 6, true);
					}
					else
					{
						selectMidiFxChordKey(thisKey - 6, true);
					}
				}
				else if (thisKey == 22) // Goto arp params
				{
					keyConsumed = true;
					if (mfxdex < NUM_MIDIFX_GROUPS)
					{
						enableSubmode(&subModeMidiFx[mfxdex]);
						subModeMidiFx[mfxdex].gotoArpParams();
						auxDown_ = false;
					}
					else
					{
						omxDisp.displayMessage(mfxOffMsg);
					}
				}
				else if (thisKey == 23) // Next arp pattern
				{
					keyConsumed = true;
					if (mfxdex < NUM_MIDIFX_GROUPS)
					{
						subModeMidiFx[mfxdex].nextArpPattern();
					}
					else
					{
						omxDisp.displayMessage(mfxOffMsg);
					}
				}
				else if (thisKey == 24) // Next arp octave
				{
					keyConsumed = true;
					if (mfxdex < NUM_MIDIFX_GROUPS)
					{
						subModeMidiFx[mfxdex].nextArpOctRange();
					}
					else
					{
						omxDisp.displayMessage(mfxOffMsg);
					}
				}
				else if (thisKey == 25)
				{
					keyConsumed = true;
					if (mfxdex < NUM_MIDIFX_GROUPS)
					{
						subModeMidiFx[mfxdex].toggleArpHold();

						if (subModeMidiFx[mfxdex].isArpHoldOn())
						{
							omxDisp.displayMessageTimed("Arp Hold: On", 5);
						}
						else
						{
							omxDisp.displayMessageTimed("Arp Hold: Off", 5);
						}
					}
					else
					{
						omxDisp.displayMessage(mfxOffMsg);
					}
				}
				else if (thisKey == 26)
				{
					keyConsumed = true;
					if (mfxdex < NUM_MIDIFX_GROUPS)
					{
						subModeMidiFx[mfxdex].toggleArp();

						if (subModeMidiFx[mfxdex].isArpOn())
						{
							omxDisp.displayMessageTimed("Arp On", 5);
						}
						else
						{
							omxDisp.displayMessageTimed("Arp Off", 5);
						}
					}
					else
					{
						omxDisp.displayMessage(mfxOffMsg);
					}
				}
			}
		}
	}

	return keyConsumed;
}

bool OmxModeChords::onKeyHeldSelMidiFX(OMXKeypadEvent e)
{
	int thisKey = e.key();

	bool keyConsumed = false;

	if (auxDown_) // Aux mode
	{
		// Enter MidiFX mode
		if (thisKey >= 6 && thisKey < 11)
		{
			keyConsumed = true;
			enableSubmode(&subModeMidiFx[thisKey - 6]);
		}
	}

	return keyConsumed;
}

void OmxModeChords::doNoteOn(int noteNumber, uint8_t midifx, uint8_t velocity, uint8_t midiChannel)
{
	if (noteNumber < 0 || noteNumber > 127)
		return;

	bool trackerFound = false;

	for (uint8_t i = 0; i < noteOffTracker.size(); i++)
	{
		if (noteOffTracker[i].noteNumber == noteNumber && noteOffTracker[i].midiChannel == midiChannel - 1)
		{
			// Serial.println("Tracker found " + String(noteNumber));
			noteOffTracker[i].triggerCount = noteOffTracker[i].triggerCount + 1;
			// Serial.println("triggerCount: " + String(noteOffTracker[i].triggerCount));
			trackerFound = true;
			break;
		}
	}

	if (!trackerFound && noteOffTracker.size() == kMaxNoteTrackerSize)
		return; // Too many notes

	if (!trackerFound)
	{
		// Serial.println("Tracker not found ");
		NoteTracker tracker;
		tracker.noteNumber = noteNumber;
		tracker.midiChannel = midiChannel - 1;
		tracker.triggerCount = 1;
		noteOffTracker.push_back(tracker);
		trackerFound = true;
	}

	// MidiNoteGroup noteGroup = omxUtil.midiNoteOn2(musicScale, keyIndex, midiSettings.defaultVelocity, sysSettings.midiChannel);
	// if(noteGroup.noteNumber == 255) return;

	MidiNoteGroup noteGroup;

	noteGroup.noteOff = false;
	noteGroup.noteNumber = noteNumber;
	noteGroup.prevNoteNumber = noteNumber;
	noteGroup.velocity = velocity;
	noteGroup.channel = midiChannel;
	noteGroup.unknownLength = true;
	noteGroup.stepLength = 0;
	noteGroup.sendMidi = true;
	noteGroup.sendCV = false;
	noteGroup.noteonMicros = micros();

	// Serial.println("doNoteOn: " + String(noteGroup.noteNumber));

	if (midifx < NUM_MIDIFX_GROUPS)
	{
		subModeMidiFx[midifx].noteInput(noteGroup);
		// subModeMidiFx.noteInput(noteGroup);
	}
	else
	{
		onNotePostFX(noteGroup);
	}
}

void OmxModeChords::doNoteOff(int noteNumber, uint8_t midifx, uint8_t midiChannel)
{
	if (noteNumber < 0 || noteNumber > 127)
		return;

	bool trackerFound = false;
	bool doNoteOff = false;

	for (uint8_t i = 0; i < noteOffTracker.size(); i++)
	{
		if (noteOffTracker[i].noteNumber == noteNumber && noteOffTracker[i].midiChannel == midiChannel - 1)
		{
			// Serial.println("Tracker found " + String(noteNumber));
			// Serial.println("triggerCount " + String(noteOffTracker[i].triggerCount));

			noteOffTracker[i].triggerCount = noteOffTracker[i].triggerCount - 1;
			if (noteOffTracker[i].triggerCount <= 0)
			{
				// Serial.println("Do Note Off");
				doNoteOff = true;
			}
			trackerFound = true;

			// Serial.println("triggerCount " + String(noteOffTracker[i].triggerCount));
			break;
		}
	}

	if (doNoteOff)
	{
		auto it = noteOffTracker.begin();
		while (it != noteOffTracker.end())
		{
			// remove matching note numbers
			if (it->triggerCount <= 0)
			{
				// Serial.println("Erasing");
				it = noteOffTracker.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	if (!trackerFound || !doNoteOff)
		return; // No note off tracker found.

	// Serial.println("Doing Note Off");

	// MidiNoteGroup noteGroup = omxUtil.midiNoteOff2(keyIndex, sysSettings.midiChannel);

	// if(noteGroup.noteNumber == 255) return;

	MidiNoteGroup noteGroup;

	noteGroup.noteOff = true;
	noteGroup.noteNumber = noteNumber;
	noteGroup.prevNoteNumber = noteNumber;
	noteGroup.velocity = 0;
	noteGroup.channel = midiChannel;
	noteGroup.unknownLength = true;
	noteGroup.stepLength = 0;
	noteGroup.sendMidi = true;
	noteGroup.sendCV = false;
	noteGroup.noteonMicros = micros();

	// Serial.println("doNoteOff: " + String(noteGroup.noteNumber));

	noteGroup.unknownLength = true;
	noteGroup.prevNoteNumber = noteGroup.noteNumber;

	if (midifx >= 0 && midifx < NUM_MIDIFX_GROUPS)
	{
		subModeMidiFx[midifx].noteInput(noteGroup);
		// subModeMidiFx.noteInput(noteGroup);
	}
	else
	{
		onNotePostFX(noteGroup);
	}
}

void OmxModeChords::splitNoteOn(uint8_t keyIndex)
{
	MidiNoteGroup noteGroup = omxUtil.midiNoteOn2(musicScale_, keyIndex, midiSettings.defaultVelocity, sysSettings.midiChannel);
	doNoteOn(noteGroup.noteNumber, mfxIndex_, noteGroup.velocity, noteGroup.channel);

	// if(noteGroup.noteNumber > 127) return;

	// bool trackerFound = false;

	// for(uint8_t i = 0; i < noteOffTracker.size(); i++)
	// {
	//     if(noteOffTracker[i].noteNumber == noteGroup.noteNumber && noteOffTracker[i].midiChannel == noteGroup.channel - 1)
	//     {
	//         noteOffTracker[i].triggerCount++;
	//         trackerFound = true;
	//         break;
	//     }
	// }

	// if(!trackerFound && noteOffTracker.size() == kMaxNoteTrackerSize) return; // Too many notes

	// if(!trackerFound)
	// {
	//     NoteTracker tracker;
	//     tracker.noteNumber = noteGroup.noteNumber;
	//     tracker.midiChannel = noteGroup.channel - 1;
	//     tracker.triggerCount = 1;
	//     noteOffTracker.push_back(tracker);
	//     trackerFound = true;
	// }

	// noteGroup.unknownLength = true;
	// noteGroup.prevNoteNumber = noteGroup.noteNumber;

	// if (mfxIndex_ < NUM_MIDIFX_GROUPS)
	// {
	//     subModeMidiFx[mfxIndex_].noteInput(noteGroup);
	// }
	// else
	// {
	//     onNotePostFX(noteGroup);
	// }
}
void OmxModeChords::splitNoteOff(uint8_t keyIndex)
{
	MidiNoteGroup noteGroup = omxUtil.midiNoteOff2(keyIndex, sysSettings.midiChannel);
	doNoteOff(noteGroup.noteNumber, mfxIndex_, noteGroup.channel);

	// if(noteGroup.noteNumber > 127) return;

	// noteGroup.unknownLength = true;
	// noteGroup.prevNoteNumber = noteGroup.noteNumber;

	// if (mfxIndex_ < NUM_MIDIFX_GROUPS)
	// {
	//     subModeMidiFx[mfxIndex_].noteInput(noteGroup);
	// }
	// else
	// {
	//     onNotePostFX(noteGroup);
	// }
}

void OmxModeChords::onNotePostFX(MidiNoteGroup note)
{
	if (note.noteOff)
	{
		// Serial.println("OmxModeMidiKeyboard::onNotePostFX noteOff: " + String(note.noteNumber));

		if (note.sendMidi)
		{
			MM::sendNoteOff(note.noteNumber, note.velocity, note.channel);
		}
		if (note.sendCV)
		{
			omxUtil.cvNoteOff();
		}
	}
	else
	{
		if (note.unknownLength == false)
		{
			uint32_t noteOnMicros = note.noteonMicros; // TODO Might need to be set to current micros
			pendingNoteOns.insert(note.noteNumber, note.velocity, note.channel, noteOnMicros, note.sendCV);

			// Serial.println("StepLength: " + String(note.stepLength));

			uint32_t noteOffMicros = noteOnMicros + (note.stepLength * clockConfig.step_micros);
			pendingNoteOffs.insert(note.noteNumber, note.channel, noteOffMicros, note.sendCV);

			// Serial.println("noteOnMicros: " + String(noteOnMicros));
			// Serial.println("noteOffMicros: " + String(noteOffMicros));
		}
		else
		{
			// Serial.println("OmxModeMidiKeyboard::onNotePostFX noteOn: " + String(note.noteNumber));

			if (note.sendMidi)
			{
				MM::sendNoteOn(note.noteNumber, note.velocity, note.channel);
			}
			if (note.sendCV)
			{
				omxUtil.cvNoteOn(note.noteNumber);
			}
		}
	}
}

void OmxModeChords::onPendingNoteOff(int note, int channel)
{
	// Serial.println("OmxModeEuclidean::onPendingNoteOff " + String(note) + " " + String(channel));
	// subModeMidiFx.onPendingNoteOff(note, channel);

	for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
	{
		subModeMidiFx[i].onPendingNoteOff(note, channel);
	}
}

void OmxModeChords::updateLEDs()
{
	if (isSubmodeEnabled())
	{
		if (activeSubmode->updateLEDs())
			return;
	}

	if (chordEditMode_)
	{
		updateLEDsChordEdit();
		return;
	}

	bool blinkState = omxLeds.getBlinkState();

	omxLeds.setAllLEDS(0, 0, 0);

	if (auxDown_)
	{
		// Blink left/right keys for octave select indicators.
		strip.setPixelColor(0, RED);
		strip.setPixelColor(1, LIME);
		strip.setPixelColor(2, MAGENTA);

		if (midiSettings.octave == 0)
		{
			strip.setPixelColor(11, colorConfig.octDnColor);
			strip.setPixelColor(12, colorConfig.octUpColor);
		}
		else if (midiSettings.octave > 0)
		{
			bool blinkOctave = omxLeds.getBlinkPattern(midiSettings.octave);

			strip.setPixelColor(11, colorConfig.octDnColor);
			strip.setPixelColor(12, blinkOctave ? colorConfig.octUpColor : LEDOFF);
		}
		else
		{
			bool blinkOctave = omxLeds.getBlinkPattern(-midiSettings.octave);

			strip.setPixelColor(11, blinkOctave ? colorConfig.octDnColor : LEDOFF);
			strip.setPixelColor(12, colorConfig.octUpColor);
		}

		// MidiFX off
		uint8_t mfxdex = lastKeyWasKeyboard_ ? mfxIndex_ : chords_[selectedChord_].midiFx;

		strip.setPixelColor(5, (mfxdex >= NUM_MIDIFX_GROUPS ? colorConfig.selMidiFXGRPOffColor : colorConfig.midiFXGRPOffColor));

		for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
		{
			auto mfxColor = (i == mfxdex) ? colorConfig.selMidiFXGRPColor : colorConfig.midiFXGRPColor;

			strip.setPixelColor(6 + i, mfxColor);
		}

		strip.setPixelColor(22, colorConfig.gotoArpParams);
		strip.setPixelColor(23, colorConfig.nextArpPattern);

		if (mfxdex < NUM_MIDIFX_GROUPS)
		{
			uint8_t octaveRange = subModeMidiFx[mfxdex].getArpOctaveRange();
			if (octaveRange == 0)
			{
				strip.setPixelColor(24, colorConfig.nextArpOctave);
			}
			else
			{
				// Serial.println("Blink Octave: " + String(octaveRange));
				bool blinkOctave = omxLeds.getBlinkPattern(octaveRange);

				strip.setPixelColor(24, blinkOctave ? colorConfig.nextArpOctave : LEDOFF);
			}

			bool isOn = subModeMidiFx[mfxdex].isArpOn() && blinkState;
			bool isHoldOn = subModeMidiFx[mfxdex].isArpHoldOn();

			strip.setPixelColor(25, isHoldOn ? colorConfig.arpHoldOn : colorConfig.arpHoldOff);
			strip.setPixelColor(26, isOn ? colorConfig.arpOn : colorConfig.arpOff);
		}
		else
		{
			strip.setPixelColor(25, colorConfig.arpHoldOff);
			strip.setPixelColor(26, colorConfig.arpOff);
		}

		return;
	}

	// Function Keys
	if (funcKeyMode_ == FUNCKEYMODE_F3)
	{
		auto f3Color = blinkState ? LEDOFF : FUNKTHREE;
		strip.setPixelColor(1, f3Color);
		strip.setPixelColor(2, f3Color);
	}
	else
	{
		auto f1Color = (funcKeyMode_ == FUNCKEYMODE_F1 && blinkState) ? LEDOFF : FUNKONE;
		strip.setPixelColor(1, f1Color);

		auto f2Color = (funcKeyMode_ == FUNCKEYMODE_F2 && blinkState) ? LEDOFF : FUNKTWO;
		strip.setPixelColor(2, f2Color);
	}

	strip.setPixelColor(3, mode_ == CHRDMODE_PLAY ? WHITE : kPlayColor);
	strip.setPixelColor(4, mode_ == CHRDMODE_EDIT ? WHITE : kEditColor);
	strip.setPixelColor(5, mode_ == CHRDMODE_PRESET ? WHITE : kPresetColor);
	strip.setPixelColor(6, mode_ == CHRDMODE_MANSTRUM ? WHITE : MAGENTA);

	if (mode_ == CHRDMODE_PLAY || mode_ == CHRDMODE_MANSTRUM) // Play
	{
		for (uint8_t i = 0; i < 16; i++)
		{
			if (i == selectedChord_)
			{
				strip.setPixelColor(11 + i, (chordNotes_[i].active ? WHITE : CYAN));
			}
			else
			{
				strip.setPixelColor(11 + i, (chordNotes_[i].active ? WHITE : chords_[i].color));
			}
		}
	}
	else if (mode_ == CHRDMODE_EDIT) // Edit
	{
		for (uint8_t i = 0; i < 16; i++)
		{
			if (i == selectedChord_)
			{
				strip.setPixelColor(11 + i, (chordNotes_[i].active ? WHITE : CYAN));
			}
			else
			{
				strip.setPixelColor(11 + i, (chordNotes_[i].active ? WHITE : kEditColor));
			}
		}
	}
	else if (mode_ == CHRDMODE_PRESET) // Preset
	{
		for (uint8_t i = 0; i < NUM_CHORD_SAVES; i++)
		{
			strip.setPixelColor(11 + i, (i == selectedSave_ ? WHITE : kPresetColor));
		}
	}

	if ((mode_ == CHRDMODE_PLAY || mode_ == CHRDMODE_EDIT) && uiMode_ == CUIMODE_SPLIT)
	{
		bool blinkNote = activeChordEditNoteKey_ >= 0 ? omxLeds.getBlinkState() : true;

		// Render scale colors and chord notes
		for (int i = 1; i < LED_COUNT; i++)
		{
			if (i >= 19 || (i >= 6 && i < 11))
			{
				strip.setPixelColor(i, LEDOFF);

				uint8_t adjKeyIndex = i >= 19 ? i - 7 : i - 5; // Pretends keys are down an octave

				if (mode_ == CHRDMODE_EDIT && heldChord_ >= 0 && chords_[heldChord_].type == CTYPE_BASIC)
				{
					// Scale colors
					auto keyColor = omxLeds.getKeyColor(musicScale_, adjKeyIndex);
					if (keyColor != LEDOFF)
					{
						strip.setPixelColor(i, kChordEditNoteInScaleColor);
					}

					// Chord note colors
					for (uint8_t ni = 0; ni < 6; ni++)
					{
						int note = chordNotes_[selectedChord_].notes[ni];

						if (note >= 0 && note <= 127)
						{
							auto adjNote = notes[adjKeyIndex] + (midiSettings.octave * 12);

							if (adjNote == note && blinkNote)
							{
								uint8_t vel = map(chordNotes_[selectedChord_].velocities[ni], 0, 127, 0, 255);

								auto noteColor = ni == 0 ? strip.ColorHSV(kChordEditNoteChordHue, 50, vel) : strip.ColorHSV(kChordEditNoteChordHue, 255, vel);

								strip.setPixelColor(i, noteColor);
							}
						}
					}
				}
				else
				{
					if (midiSettings.midiKeyState[adjKeyIndex] >= 0)
					{
						strip.setPixelColor(i, LTCYAN);
					}
					else
					{
						// Scale colors
						strip.setPixelColor(i, omxLeds.getKeyColor(musicScale_, adjKeyIndex));
					}
				}
			}
		}
	}

	if (isSubmodeEnabled())
	{
		bool blinkStateSlow = omxLeds.getSlowBlinkState();

		auto auxColor = (blinkStateSlow ? RED : LEDOFF);
		strip.setPixelColor(0, auxColor);
	}
}

void OmxModeChords::updateLEDsChordEdit()
{
	bool blinkState = omxLeds.getBlinkState();

	omxLeds.setAllLEDS(0, 0, 0);

	strip.setPixelColor(0, RED); // EXIT

	if (chords_[selectedChord_].type == CTYPE_BASIC)
	{
		bool blinkNote = activeChordEditNoteKey_ >= 0 ? omxLeds.getBlinkState() : true;

		// Render scale colors and chord notes
		for (int i = 1; i < LED_COUNT; i++)
		{
			// Scale colors
			auto keyColor = omxLeds.getKeyColor(musicScale_, i);
			if (keyColor != LEDOFF)
			{
				strip.setPixelColor(i, kChordEditNoteInScaleColor);
			}

			// Chord note colors
			for (uint8_t ni = 0; ni < 6; ni++)
			{
				int note = chordNotes_[selectedChord_].notes[ni];

				if (note >= 0 && note <= 127)
				{
					auto adjNote = notes[i] + (midiSettings.octave * 12);

					if (adjNote == note && blinkNote)
					{
						uint8_t vel = map(chordNotes_[selectedChord_].velocities[ni], 0, 127, 0, 255);

						auto noteColor = ni == 0 ? strip.ColorHSV(kChordEditNoteChordHue, 50, vel) : strip.ColorHSV(kChordEditNoteChordHue, 255, vel);

						strip.setPixelColor(i, noteColor);
					}
				}
			}
		}

		if (midiSettings.octave == 0)
		{
			strip.setPixelColor(11, colorConfig.octDnColor);
			strip.setPixelColor(26, colorConfig.octUpColor);
		}
		else if (midiSettings.octave > 0)
		{
			bool blinkOctave = omxLeds.getBlinkPattern(midiSettings.octave);

			strip.setPixelColor(11, colorConfig.octDnColor);
			strip.setPixelColor(26, blinkOctave ? colorConfig.octUpColor : LEDOFF);
		}
		else
		{
			bool blinkOctave = omxLeds.getBlinkPattern(-midiSettings.octave);

			strip.setPixelColor(11, blinkOctave ? colorConfig.octDnColor : LEDOFF);
			strip.setPixelColor(26, colorConfig.octUpColor);
		}
	}
	else if (chords_[selectedChord_].type == CTYPE_INTERVAL)
	{
		// Function Keys
		if (funcKeyMode_ == FUNCKEYMODE_F3)
		{
			auto f3Color = blinkState ? LEDOFF : FUNKTHREE;
			strip.setPixelColor(1, f3Color);
			strip.setPixelColor(2, f3Color);
		}
		else
		{
			auto f1Color = (funcKeyMode_ == FUNCKEYMODE_F1 && blinkState) ? LEDOFF : FUNKONE;
			strip.setPixelColor(1, f1Color);

			auto f2Color = (funcKeyMode_ == FUNCKEYMODE_F2 && blinkState) ? LEDOFF : FUNKTWO;
			strip.setPixelColor(2, f2Color);
		}

		strip.setPixelColor(3, kOctaveColor);	 // Octave
		strip.setPixelColor(4, kTransposeColor); // Transpose
		strip.setPixelColor(5, kSpreadColor);	 // Spread
		strip.setPixelColor(6, kRotateColor);	 // Rotate
		strip.setPixelColor(7, kVoicingColor);	 // Voicing
		strip.setPixelColor(10, ROSE);			 // Show Chord Notes

		if (chordEditParam_ == 0)
		{
			// Num Notes
			for (uint8_t i = 11; i < 15; i++)
			{
				auto numNotesColor = chords_[selectedChord_].numNotes == (i - 11) + 1 ? kNumNotesSelColor : kNumNotesColor;
				strip.setPixelColor(i, numNotesColor);
			}

			strip.setPixelColor(15, chords_[selectedChord_].spreadUpDown ? kSpreadUpDownOnColor : kSpreadUpDownOffColor);
			strip.setPixelColor(16, chords_[selectedChord_].quartalVoicing ? kQuartalVoicingOnColor : kQuartalVoicingOffColor);

			// Degree
			for (uint8_t i = 19; i < 27; i++)
			{
				strip.setPixelColor(i, chords_[selectedChord_].degree == i - 19 ? kDegreeSelColor : kDegreeColor);
			}
		}
		else if (chordEditParam_ == 1) // Octave
		{
			strip.setPixelColor(3, blinkState ? LEDOFF : kOctaveColor);

			for (uint8_t i = 11; i < 16; i++)
			{
				auto valColor = chords_[selectedChord_].octave == (i - 11 - 2) ? WHITE : GREEN;
				strip.setPixelColor(i, valColor);
			}
		}
		else if (chordEditParam_ == 2) // Transpose
		{
			strip.setPixelColor(4, blinkState ? LEDOFF : kTransposeColor);

			for (uint8_t i = 11; i < 26; i++)
			{
				auto valColor = chords_[selectedChord_].transpose == (i - 11 - 7) ? WHITE : GREEN;
				strip.setPixelColor(i, valColor);
			}
		}
		else if (chordEditParam_ == 3) // Spread
		{
			strip.setPixelColor(5, blinkState ? LEDOFF : kSpreadColor);

			for (uint8_t i = 11; i < 16; i++)
			{
				auto valColor = chords_[selectedChord_].spread == (i - 11 - 2) ? WHITE : GREEN;
				strip.setPixelColor(i, valColor);
			}
		}
		else if (chordEditParam_ == 4) // Rotate
		{
			strip.setPixelColor(6, blinkState ? LEDOFF : kRotateColor);

			for (uint8_t i = 11; i < 16; i++)
			{
				auto valColor = chords_[selectedChord_].rotate == (i - 11) ? WHITE : GREEN;
				strip.setPixelColor(i, valColor);
			}
		}
		else if (chordEditParam_ == 5) // Voicing
		{
			strip.setPixelColor(7, blinkState ? LEDOFF : kVoicingColor);

			for (uint8_t i = 11; i < 19; i++)
			{
				auto valColor = chords_[selectedChord_].voicing == (i - 11) ? WHITE : GREEN;
				strip.setPixelColor(i, valColor);
			}
		}
	}

	if (isSubmodeEnabled())
	{
		bool blinkStateSlow = omxLeds.getSlowBlinkState();
		auto auxColor = (blinkStateSlow ? RED : LEDOFF);
		strip.setPixelColor(0, auxColor);
	}
}

void OmxModeChords::setupPageLegend(uint8_t index, uint8_t paramType)
{
	switch (paramType)
	{
	case CPARAM_UIMODE:
	{
		omxDisp.legends[index] = "UI";
		omxDisp.legendText[index] = kUIModeDisp[uiMode_];
	}
	break;
	case CPARAM_MAN_STRUM:
	{
		omxDisp.legends[index] = "STRUM";
		omxDisp.legendText[index] = mode_ == CHRDMODE_MANSTRUM ? "ON" : "OFF";
	}
	break;
	case CPARAM_CHORD_TYPE:
	{
		omxDisp.legends[index] = "TYPE";
		omxDisp.legendText[index] = kChordTypeDisp[chords_[selectedChord_].type];
	}
	break;
	case CPARAM_CHORD_MFX:
	{
		omxDisp.legends[index] = "MIFX";
		if (chords_[selectedChord_].midiFx >= 0)
		{
			omxDisp.legendVals[index] = chords_[selectedChord_].midiFx + 1;
		}
		else
		{
			omxDisp.legendText[index] = "OFF";
		}
	}
	break;
	case CPARAM_CHORD_VEL:
	{
		omxDisp.legends[index] = "VEL";
		omxDisp.legendVals[index] = chords_[selectedChord_].velocity;
	}
	break;
	case CPARAM_CHORD_MCHAN:
	{
		omxDisp.legends[index] = "MCHAN";
		omxDisp.legendVals[index] = chords_[selectedChord_].mchan + 1;
	}
	break;
	case CPARAM_BAS_NOTE:
	{
		omxDisp.legends[index] = "NOTE";
		omxDisp.legendText[index] = MusicScales::getNoteName(chords_[selectedChord_].note);
	}
	break;
	case CPARAM_BAS_OCT:
	{
		omxDisp.legends[index] = "C-OCT";
		omxDisp.legendVals[index] = chords_[selectedChord_].basicOct + 4;
	}
	break;
	case CPARAM_BAS_CHORD:
	{
		omxDisp.legends[index] = "CHRD";
		omxDisp.legendVals[index] = chords_[selectedChord_].chord;
	}
	break;
	case CPARAM_BAS_BALANCE:
	{
		omxDisp.legends[index] = "BAL";
		omxDisp.legendVals[index] = map(chords_[selectedChord_].balance, 0, (kNumChordBalance - 1) * 10, 0, 127);
	}
	break;
	case CPARAM_INT_NUMNOTES:
	{
		omxDisp.legends[index] = "#NTS";
		omxDisp.legendVals[index] = chords_[selectedChord_].numNotes;
	}
	break;
	case CPARAM_INT_DEGREE:
	{
		omxDisp.legends[index] = "DEG";
		omxDisp.legendVals[index] = chords_[selectedChord_].degree;
	}
	break;
	case CPARAM_INT_OCTAVE:
	{
		omxDisp.legends[index] = "OCT";
		omxDisp.legendVals[index] = chords_[selectedChord_].octave;
	}
	break;
	case CPARAM_INT_TRANSPOSE:
	{
		omxDisp.legends[index] = "TPS";
		omxDisp.legendVals[index] = chords_[selectedChord_].transpose;
	}
	break;
	case CPARAM_INT_SPREAD:
	{
		omxDisp.legends[index] = "SPRD";
		omxDisp.legendVals[index] = chords_[selectedChord_].spread;
	}
	break;
	case CPARAM_INT_ROTATE:
	{
		omxDisp.legends[index] = "ROT";
		omxDisp.legendVals[index] = chords_[selectedChord_].rotate;
	}
	break;
	case CPARAM_INT_VOICING:
	{
		omxDisp.legends[index] = "VOIC";
		omxDisp.legendText[index] = kVoicingNames[chords_[selectedChord_].voicing];
	}
	break;
	case CPARAM_INT_SPRDUPDOWN:
	{
		omxDisp.legends[index] = "UPDN";
		omxDisp.legendText[index] = chords_[selectedChord_].spreadUpDown ? "ON" : "OFF";
	}
	break;
	case CPARAM_INT_QUARTVOICE:
	{
		omxDisp.legends[index] = "QRTV";
		omxDisp.legendText[index] = chords_[selectedChord_].quartalVoicing ? "ON" : "OFF";
	}
	break;
	}
}

void OmxModeChords::setupPageLegends()
{
	omxDisp.clearLegends();

	int8_t page = getParams()->getSelPage();

	switch (page)
	{
	case CHRDPAGE_GBL1:
	{
		setupPageLegend(0, CPARAM_UIMODE);
	}
	break;
	case CHRDPAGE_OUTMIDI:
	{
		omxUtil.setupPageLegend(0, GPARAM_MOUT_OCT);
		omxUtil.setupPageLegend(1, GPARAM_MOUT_CHAN);
		omxUtil.setupPageLegend(2, GPARAM_MOUT_VEL);
	}
	break;
	case CHRDPAGE_POTSANDMACROS:
	{
		omxUtil.setupPageLegend(0, GPARAM_POTS_PBANK);
		omxUtil.setupPageLegend(1, GPARAM_MIDI_THRU);
		omxUtil.setupPageLegend(2, GPARAM_MACRO_MODE);
		omxUtil.setupPageLegend(3, GPARAM_MACRO_CHAN);
	}
	break;
	case CHRDPAGE_SCALES:
	{
		omxUtil.setupPageLegend(musicScale_, 0, GPARAM_SCALE_ROOT);
		omxUtil.setupPageLegend(musicScale_, 1, GPARAM_SCALE_PAT);
		omxUtil.setupPageLegend(musicScale_, 2, GPARAM_SCALE_LOCK);
		omxUtil.setupPageLegend(musicScale_, 3, GPARAM_SCALE_GRP16);
	}
	break;
	case CHRDPAGE_1:
	{
		setupPageLegend(0, CPARAM_CHORD_TYPE);
		setupPageLegend(1, CPARAM_CHORD_MFX);
		setupPageLegend(2, CPARAM_CHORD_VEL);
		setupPageLegend(3, CPARAM_CHORD_MCHAN);
	}
	break;
	case CHRDPAGE_2:
	{
		if (chords_[selectedChord_].type == CTYPE_INTERVAL)
		{
			setupPageLegend(0, CPARAM_INT_NUMNOTES);
			setupPageLegend(1, CPARAM_INT_DEGREE);
			setupPageLegend(2, CPARAM_INT_OCTAVE);
			setupPageLegend(3, CPARAM_INT_TRANSPOSE);
		}
		else if (chords_[selectedChord_].type == CTYPE_BASIC)
		{
			setupPageLegend(0, CPARAM_BAS_NOTE);
			setupPageLegend(1, CPARAM_BAS_OCT);
			setupPageLegend(2, CPARAM_BAS_CHORD);
			setupPageLegend(3, CPARAM_BAS_BALANCE);
		}
	}
	break;
	case CHRDPAGE_3:
	{
		if (chords_[selectedChord_].type == CTYPE_INTERVAL)
		{
			setupPageLegend(0, CPARAM_INT_SPREAD);
			setupPageLegend(1, CPARAM_INT_ROTATE);
			setupPageLegend(2, CPARAM_INT_VOICING);
		}
	}
	break;
	case CHRDPAGE_4:
	{
		if (chords_[selectedChord_].type == CTYPE_INTERVAL)
		{
			setupPageLegend(0, CPARAM_INT_SPRDUPDOWN);
			setupPageLegend(1, CPARAM_INT_QUARTVOICE);
		}
	}
	break;
	default:
		break;
	}
}

void OmxModeChords::onDisplayUpdate()
{
	// omxLeds.updateBlinkStates();

	if (isSubmodeEnabled())
	{
		if (omxLeds.isDirty())
		{
			updateLEDs();
		}

		activeSubmode->onDisplayUpdate();
		return;
	}

	if (omxLeds.isDirty())
	{
		updateLEDs();
	}

	if (omxDisp.isDirty())
	{
		if (!encoderConfig.enc_edit)
		{
			auto params = getParams();

			if (chordEditMode_ == false && (mode_ == CHRDMODE_EDIT) && funcKeyMode_ == FUNCKEYMODE_F1) // Edit mode enter edit mode
			{
				omxDisp.dispGenericModeLabel("Edit chord", params->getNumPages(), params->getSelPage());
			}
			else if (chordEditMode_ == false && (mode_ == CHRDMODE_EDIT) && funcKeyMode_ == FUNCKEYMODE_F2) // Edit mode copy
			{
				omxDisp.dispGenericModeLabel("Copy to", params->getNumPages(), params->getSelPage());
			}
			// if (chordEditMode_ == false && (mode_ == CHRDMODE_PLAY || mode_ == CHRDMODE_EDIT || mode_ == CHRDMODE_MANSTRUM) && funcKeyMode_ == FUNCKEYMODE_F2) // Play mode copy
			// {
			// 	omxDisp.dispGenericModeLabel("Copy to", params->getNumPages(), params->getSelPage());
			// }
			else if (chordEditMode_ == false && (mode_ == CHRDMODE_PRESET) && funcKeyMode_ == FUNCKEYMODE_F1) // Preset move load
			{
				omxDisp.dispGenericModeLabel("Load from", params->getNumPages(), params->getSelPage());
			}
			else if (chordEditMode_ == false && (mode_ == CHRDMODE_PRESET) && funcKeyMode_ == FUNCKEYMODE_F2) // Preset move save
			{
				omxDisp.dispGenericModeLabel("Save to", params->getNumPages(), params->getSelPage());
			}
			else if (chordEditMode_ == false && mode_ == CHRDMODE_MANSTRUM)
			{
				omxDisp.dispGenericModeLabel("Enc Strum", params->getNumPages(), 0);
			}
			else if (params->getSelPage() == CHRDPAGE_NOTES)
			{
				if (chordNotes_[selectedChord_].active || chordEditNotes_.active)
				{
					notesString = "";
					// notesString2 = "";

					for (uint8_t i = 0; i < 6; i++)
					{
						int8_t note = chordNotes_[selectedChord_].notes[i];

						if (chordEditNotes_.active)
						{
							note = chordEditNotes_.notes[i];
						}

						if (note >= 0 && note <= 127)
						{
							if (i > 0)
							{
								notesString.append(" ");
							}
							notesString.append(musicScale_->getFullNoteName(note));

							// if(i < 4)
							// {
							//     if (i > 0)
							//     {
							//         notesString.append(" ");
							//     }
							//     notesString.append(musicScale_->getFullNoteName(note));
							// }
							// else
							// {
							//     if (i > 4)
							//     {
							//         notesString2.append(" ");
							//     }
							//     notesString2.append(musicScale_->getFullNoteName(note));

							// }
						}
					}

					const char *labels[1];
					labels[0] = notesString.c_str();
					// omxDisp.dispGenericModeLabelDoubleLine(notesString.c_str(), notesString2.c_str(), params->getNumPages(), params->getSelPage());
					if (chordEditNotes_.active)
					{
						// int rootNote = chords_[selectedChord_].note;
						omxDisp.dispKeyboard(chordEditNotes_.rootNote, chordEditNotes_.notes, true, labels, 1);
					}
					else
					{
						omxDisp.dispKeyboard(chordNotes_[selectedChord_].rootNote, chordNotes_[selectedChord_].notes, true, labels, 1);
					}
				}
				else
				{
					omxDisp.dispKeyboard(-1, noNotes, false, nullptr, 0);

					// omxDisp.dispGenericModeLabel("-", params->getNumPages(), params->getSelPage());
				}
			}
			// Chord page
			else if (params->getSelPage() == CHRDPAGE_2 && chords_[selectedChord_].type == CTYPE_BASIC)
			{
				auto noteName = MusicScales::getNoteName(chords_[selectedChord_].note, true);
				int octave = chords_[selectedChord_].basicOct + 4;
				notesString2 = String(octave);
				auto chordType = kChordMsg[chords_[selectedChord_].chord];

				activeChordBalance_ = getChordBalanceDetails(chords_[selectedChord_].balance);

				omxDisp.dispChordBasicPage(params->getSelParam(), getEncoderSelect(), noteName, notesString2.c_str(), chordType, activeChordBalance_.type, activeChordBalance_.velMult);
			}
			// Custom Chord Notes
			else if (params->getSelPage() == CHRDPAGE_3 && chords_[selectedChord_].type == CTYPE_BASIC && chords_[selectedChord_].chord == kCustomChordPattern)
			{
				const char *labels[6];
				const char *headers[1];
				headers[0] = "Custom Chord";

				for (uint8_t i = 0; i < 6; i++)
				{
					int note = chords_[selectedChord_].customNotes[i].note;

					if (note == 0)
					{
						if (i == 0)
						{
							customNotesStrings[i] = "RT";
						}
						else
						{
							customNotesStrings[i] = "-";
						}
					}
					else
					{
						if (note > 0)
						{
							customNotesStrings[i] = "+" + String(note);
						}
						else
						{
							customNotesStrings[i] = "" + String(note);
						}
					}

					labels[i] = customNotesStrings[i].c_str();
				}

				omxDisp.dispCenteredSlots(labels, 6, params->getSelParam(), getEncoderSelect(), true, true, headers, 1);
			}
			else
			{

				setupPageLegends();
				omxDisp.dispGenericMode2(params->getNumPages(), params->getSelPage(), params->getSelParam(), getEncoderSelect());
			}
		}
	}
}

void OmxModeChords::SetScale(MusicScales *scale)
{
	musicScale_ = scale;
}

bool OmxModeChords::pasteSelectedChordTo(uint8_t chordIndex)
{
	if (chordIndex == selectedChord_ || chordIndex >= 16)
		return false;

	chords_[chordIndex].CopySettingsFrom(chords_[selectedChord_]);
	selectedChord_ = chordIndex;
	lastKeyWasKeyboard_ = false;
	return true;
}
bool OmxModeChords::loadPreset(uint8_t presetIndex)
{
	if (presetIndex >= NUM_CHORD_SAVES)
		return false;

	for (uint8_t i = 0; i < 16; i++)
	{
		chords_[i].CopySettingsFrom(chordSaves_[presetIndex][i]);
	}

	selectedSave_ = presetIndex;

	return true;
}

bool OmxModeChords::savePreset(uint8_t presetIndex)
{
	if (presetIndex >= NUM_CHORD_SAVES)
		return false;

	for (uint8_t i = 0; i < 16; i++)
	{
		chordSaves_[presetIndex][i].CopySettingsFrom(chords_[i]);
	}

	selectedSave_ = presetIndex;

	return true;
}

void OmxModeChords::onManualStrumOn(uint8_t chordIndex)
{
	// Serial.println("onManualStrumOn: " + String(chordIndex));
	if (chordNotes_[chordIndex].active)
	{
		// Serial.println("chord already active");
		return; // This shouldn't happen
	}

	if (constructChord(chordIndex))
	{
		chordNotes_[chordIndex].active = true;
		chordNotes_[chordIndex].channel = sysSettings.midiChannel;
		// uint8_t velocity = midiSettings.defaultVelocity;

		chordNotes_[chordIndex].strumPos = 0;
		chordNotes_[chordIndex].encDelta = 0;
		chordNotes_[chordIndex].octIncrement = 0;

		// Serial.print("Chord: ");
		// for(uint8_t i = 0; i < 6; i++)
		// {
		//     int note = chordNotes_[chordIndex].notes[i];
		//     // Serial.print(String(note) + " ");
		//     if(note >= 0 && note <= 127)
		//     {
		//         MM::sendNoteOn(note, velocity, chordNotes_[chordIndex].channel);
		//     }
		// }
		// Serial.print("\n");
	}
	else
	{
		Serial.println("constructChord failed");
	}
}

void OmxModeChords::onChordOn(uint8_t chordIndex)
{
	// Serial.println("onChordOn: " + String(chordIndex));
	if (chordNotes_[chordIndex].active)
	{
		// Serial.println("chord already active");
		return; // This shouldn't happen
	}

	if (constructChord(chordIndex))
	{
		chordNotes_[chordIndex].active = true;
		chordNotes_[chordIndex].channel = chords_[chordIndex].mchan + 1;

		// Prevent stuck notes
		playedChordNotes_[chordIndex].CopyFrom(chordNotes_[chordIndex]);
		// uint8_t velocity = chords_[chordIndex].velocity;

		// uint32_t noteOnMicros = micros();

		// Serial.print("Chord: ");
		for (uint8_t i = 0; i < 6; i++)
		{
			int note = chordNotes_[chordIndex].notes[i];
			uint8_t velocity = chordNotes_[chordIndex].velocities[i];

			// Serial.print("Note: " + String(note));
			// Serial.print(" Vel: " + String(velocity));
			// Serial.print("\n");

			// if(note >= 0 && note <= 127)
			// {
			//     // MM::sendNoteOn(note, velocity, chordNotes_[chordIndex].channel);
			//     pendingNoteOns.insert(note, velocity, chordNotes_[chordIndex].channel, noteOnMicros, false);
			// }

			doNoteOn(note, chordNotes_[chordIndex].midifx, velocity, chordNotes_[chordIndex].channel);
		}
		// Serial.print("\n");
	}
	else
	{
		// Serial.println("constructChord failed");
	}
}

void OmxModeChords::onChordOff(uint8_t chordIndex)
{
	// Serial.println("onChordOff: " + String(chordIndex));
	if (chordNotes_[chordIndex].active == false)
		return;

	for (uint8_t i = 0; i < 6; i++)
	{
		int note = playedChordNotes_[chordIndex].notes[i];

		doNoteOff(note, playedChordNotes_[chordIndex].midifx, playedChordNotes_[chordIndex].channel);

		// if (note >= 0 && note <= 127)
		// {
		//     // MM::sendNoteOff(note, 0, chordNotes_[chordIndex].channel);

		//     pendingNoteOns.remove(note, chordNotes_[chordIndex].channel);
		//     pendingNoteOffs.sendOffNow(note, chordNotes_[chordIndex].channel, false);
		// }
	}
	chordNotes_[chordIndex].active = false;
}

void OmxModeChords::onChordEditOn(uint8_t chordIndex)
{
	// Serial.println("onChordOn: " + String(chordIndex));
	if (chordEditNotes_.active)
	{
		// Serial.println("chord already active");
		return; // This shouldn't happen
	}

	if (constructChord(chordIndex))
	{
		// chordNotes_[chordIndex].active = true;
		chordNotes_[chordIndex].channel = chords_[chordIndex].mchan + 1;
		// uint8_t velocity = chords_[chordIndex].velocity;

		chordEditNotes_.CopyFrom(chordNotes_[chordIndex]);
		chordEditNotes_.active = true;

		// chordEditNotes_.channel = chordNotes_[chordIndex].channel;
		// chordEditNotes_.rootNote = chordNotes_[chordIndex].rootNote;

		// uint32_t noteOnMicros = micros();

		// Serial.print("Chord: ");
		for (uint8_t i = 0; i < 6; i++)
		{
			int note = chordEditNotes_.notes[i];
			uint8_t velocity = chordEditNotes_.velocities[i];

			// chordEditNotes_.notes[i] = note;
			// Serial.print(String(note) + " ");
			// if(note >= 0 && note <= 127)
			// {
			//     // MM::sendNoteOn(note, velocity, chordNotes_[chordIndex].channel);
			//     pendingNoteOns.insert(note, velocity, chordNotes_[chordIndex].channel, noteOnMicros, false);
			// }

			doNoteOn(note, chordEditNotes_.midifx, velocity, chordEditNotes_.channel);
		}
		// Serial.print("\n");
	}
	else
	{
		// Serial.println("constructChord failed");
	}
}

void OmxModeChords::onChordEditOff()
{
	// onChordOff(selectedChord_);

	// Serial.println("onChordOff: " + String(chordIndex));
	if (chordEditNotes_.active == false)
		return;

	for (uint8_t i = 0; i < 6; i++)
	{
		int note = chordEditNotes_.notes[i];

		doNoteOff(note, chordEditNotes_.midifx, chordEditNotes_.channel);

		// if (note >= 0 && note <= 127)
		// {
		//     // MM::sendNoteOff(note, 0, chordNotes_[chordIndex].channel);

		//     pendingNoteOns.remove(note, chordNotes_[chordIndex].channel);
		//     pendingNoteOffs.sendOffNow(note, chordNotes_[chordIndex].channel, false);
		// }
	}
	chordEditNotes_.active = false;
}

bool OmxModeChords::constructChord(uint8_t chordIndex)
{
	// Serial.println("Constructing Chord: " + String(chordIndex));
	auto chord = chords_[chordIndex];

	if (chord.type == CTYPE_BASIC)
	{
		return constructChordBasic(chordIndex);
	}

	int8_t octave = midiSettings.octave + chord.octave;

	uint8_t numNotes = 0;

	for (uint8_t i = 0; i < 6; i++)
	{
		chordNotes_[chordIndex].notes[i] = -1;
		chordNotes_[chordIndex].velocities[i] = chord.velocity;
	}

	if (chord.numNotes == 0)
	{
		return false;
	}
	else if (chord.numNotes == 1)
	{
		chordNotes_[chordIndex].notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
		numNotes = 1;
	}
	else if (chord.numNotes == 2)
	{
		chordNotes_[chordIndex].notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
		chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 2, octave);
		numNotes = 2;
	}
	else if (chord.numNotes == 3)
	{
		chordNotes_[chordIndex].notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
		chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 2, octave);
		chordNotes_[chordIndex].notes[2] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
		numNotes = 3;
	}
	else if (chord.numNotes == 4)
	{
		chordNotes_[chordIndex].notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
		chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 2, octave);
		chordNotes_[chordIndex].notes[2] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
		chordNotes_[chordIndex].notes[3] = musicScale_->getNoteByDegree(chord.degree + 6, octave);
		numNotes = 4;
	}

	chordNotes_[chordIndex].rootNote = chordNotes_[chordIndex].notes[0];

	// Serial.println("numNotes: " + String(numNotes));

	switch (chord.voicing)
	{
	case CHRDVOICE_NONE:
	{
	}
	break;
	case CHRDVOICE_POWER:
	{
		if (chord.numNotes > 1)
		{
			chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
		}
		if (chord.numNotes > 2)
		{
			chordNotes_[chordIndex].notes[2] = chordNotes_[chordIndex].notes[1] + 12;
			for (uint8_t i = 3; i < 6; i++)
			{
				chordNotes_[chordIndex].notes[i] = -1;
			}
			numNotes = 3;
		}
	}
	break;
	case CHRDVOICE_SUS2:
	{
		if (chord.numNotes > 1)
		{
			chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 1, octave);
		}
	}
	break;
	case CHRDVOICE_SUS4:
	{
		if (chord.numNotes > 1)
		{
			chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 3, octave);
		}
	}
	break;
	case CHRDVOICE_SUS24:
	{
		if (chord.numNotes > 1)
		{
			chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 1, octave);
		}
		if (chord.numNotes > 2)
		{
			chordNotes_[chordIndex].notes[2] = musicScale_->getNoteByDegree(chord.degree + 3, octave);
		}
	}
	break;
	case CHRDVOICE_ADD6:
	{
		chordNotes_[chordIndex].notes[chord.numNotes] = musicScale_->getNoteByDegree(chord.degree + 5, octave);
		numNotes = chord.numNotes + 1;
	}
	break;
	case CHRDVOICE_ADD69:
	{
		chordNotes_[chordIndex].notes[chord.numNotes] = musicScale_->getNoteByDegree(chord.degree + 5, octave);
		chordNotes_[chordIndex].notes[chord.numNotes + 1] = musicScale_->getNoteByDegree(chord.degree + 8, octave);
		numNotes = chord.numNotes + 2;
	}
	break;
	case CHRDVOICE_KB11:
	{
		if (chord.numNotes > 1)
		{
			chordNotes_[chordIndex].notes[0] = musicScale_->getNoteByDegree(chord.degree + 0, octave);
			chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
			numNotes = 2;
		}
		if (chord.numNotes > 2)
		{
			chordNotes_[chordIndex].notes[2] = musicScale_->getNoteByDegree(chord.degree + 8, octave);
			numNotes = 3;
		}
		if (chord.numNotes > 3)
		{
			chordNotes_[chordIndex].notes[3] = musicScale_->getNoteByDegree(chord.degree + 9, octave);
			chordNotes_[chordIndex].notes[4] = musicScale_->getNoteByDegree(chord.degree + 6, octave + 1);
			chordNotes_[chordIndex].notes[5] = musicScale_->getNoteByDegree(chord.degree + 10, octave + 1);
			numNotes = 6;
		}
	}
	break;
	default:
		break;
	}

	// Serial.println("numNotes: " + String(numNotes));

	if (chord.quartalVoicing)
	{
		chordNotes_[chordIndex].notes[0] = AddOctave(chordNotes_[chordIndex].notes[0], 2);
		chordNotes_[chordIndex].notes[1] = AddOctave(chordNotes_[chordIndex].notes[1], 0);
		chordNotes_[chordIndex].notes[2] = AddOctave(chordNotes_[chordIndex].notes[2], 1);
		chordNotes_[chordIndex].notes[3] = AddOctave(chordNotes_[chordIndex].notes[3], -1);
	}

	if (chord.spreadUpDown)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			if (i % 2 == 0)
			{
				chordNotes_[chordIndex].notes[i] = AddOctave(chordNotes_[chordIndex].notes[i], -1);
			}
			else
			{
				chordNotes_[chordIndex].notes[i] = AddOctave(chordNotes_[chordIndex].notes[i], 1);
			}
		}
	}

	if (chord.spread < 0)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			if (i % 2 == 0)
			{
				chordNotes_[chordIndex].notes[i] = AddOctave(chordNotes_[chordIndex].notes[i], chord.spread);
			}
		}
	}
	else if (chord.spread > 0)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			if (i % 2 != 0)
			{
				chordNotes_[chordIndex].notes[i] = AddOctave(chordNotes_[chordIndex].notes[i], chord.spread);
			}
		}
	}

	if (chord.rotate != 0 && numNotes > 0)
	{
		int temp[numNotes];

		uint8_t val = numNotes - chord.rotate;

		uint8_t offset = chord.rotate % numNotes;

		for (uint8_t i = 0; i < offset; i++)
		{
			chordNotes_[chordIndex].notes[i] = AddOctave(chordNotes_[chordIndex].notes[i], 1);
		}

		for (uint8_t i = 0; i < numNotes; i++)
		{
			temp[i] = chordNotes_[chordIndex].notes[abs((i + val) % numNotes)];
		}
		for (int i = 0; i < numNotes; i++)
		{
			chordNotes_[chordIndex].notes[i] = temp[i];
		}
	}

	for (uint8_t i = 0; i < 6; i++)
	{
		chordNotes_[chordIndex].notes[i] = TransposeNote(chordNotes_[chordIndex].notes[i], chord.transpose);
	}

	chordNotes_[chordIndex].midifx = chord.midiFx;

	return true;
}

bool OmxModeChords::constructChordBasic(uint8_t chordIndex)
{
	auto chord = chords_[chordIndex];

	// int8_t octave = midiSettings.octave + chord.octave;

	// uint8_t numNotes = 0;

	for (uint8_t i = 0; i < 6; i++)
	{
		chordNotes_[chordIndex].notes[i] = -1;
	}

	// int adjRoot = notes[thisKey] + (midiSettings.octave + 1 * 12);

	int rootNote = chord.note + ((chord.basicOct + 5) * 12);

	if (rootNote < 0 || rootNote > 127)
		return false;

	chordNotes_[chordIndex].rootNote = rootNote;

	chordNotes_[chordIndex].midifx = chord.midiFx;

	chordNotes_[chordIndex].notes[0] = rootNote;

	if (chord.chord == kCustomChordPattern)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			int noteOffset = chord.customNotes[i].note;

			if (noteOffset != 0 || (noteOffset == 0 && i == 0))
			{
				chordNotes_[chordIndex].notes[i] = rootNote + noteOffset;
			}
			// else offset is zero, do nothing.
		}
	}
	else
	{
		auto pattern = chordPatterns[chord.chord];

		for (uint8_t i = 0; i < 3; i++)
		{
			if (pattern[i] >= 0)
			{
				chordNotes_[chordIndex].notes[i + 1] = rootNote + pattern[i];
			}
		}
	}

	activeChordBalance_ = getChordBalanceDetails(chord.balance);

	for (uint8_t i = 0; i < 4; i++)
	{
		int pnote = chordNotes_[chordIndex].notes[i];

		if (pnote >= 0 && pnote <= 127)
		{
			int bal = activeChordBalance_.type[i];

			chordNotes_[chordIndex].notes[i] = (bal <= -10 ? -1 : (pnote + (12 * bal)));
			chordNotes_[chordIndex].velocities[i] = chord.velocity * activeChordBalance_.velMult[i];
		}
	}

	return true;
}

ChordBalanceDetails OmxModeChords::getChordBalanceDetails(uint8_t balance)
{
	ChordBalanceDetails bDetails;

	bDetails.type[0] = 0;
	bDetails.velMult[0] = 1.0f;

	uint8_t balanceIndex = balance / 10;

	auto balancePat = chordBalance[balanceIndex];

	for (uint8_t i = 0; i < 3; i++)
	{
		int8_t bal = balancePat[i];

		bDetails.type[i + 1] = bal;

		if (balanceIndex < kNumChordBalance)
		{
			int8_t nextBal = chordBalance[balanceIndex + 1][i];

			if ((balance % 10) != 0)
			{
				if (nextBal > -10)
				{
					bDetails.type[i + 1] = nextBal;
				}
			}

			float v1 = bal <= -10 ? 0.0f : 1.0f;
			float v2 = nextBal <= -10 ? 0.0f : 1.0f;

			bDetails.velMult[i + 1] = map((float)balance, balanceIndex * 10.0f, (balanceIndex + 1) * 10.0f, v1, v2);
		}
		else
		{
			bDetails.velMult[i + 1] = 1.0f;
		}
	}

	return bDetails;
}

int OmxModeChords::AddOctave(int note, int8_t octave)
{
	if (note < 0 || note > 127)
		return -1;

	int newNote = note + (12 * octave);
	if (newNote < 0 || newNote > 127)
		return -1;
	return newNote;
}

int OmxModeChords::TransposeNote(int note, int8_t semitones)
{
	if (note < 0 || note > 127)
		return -1;

	int newNote = note + semitones;
	if (newNote < 0 || newNote > 127)
		return -1;
	return newNote;
}

int OmxModeChords::saveToDisk(int startingAddress, Storage *storage)
{
	int saveSize = sizeof(ChordSettings);

	for (uint8_t saveIndex = 0; saveIndex < NUM_CHORD_SAVES; saveIndex++)
	{
		for (uint8_t i = 0; i < 16; i++)
		{
			auto saveBytesPtr = (byte *)(&chordSaves_[saveIndex][i]);
			for (int j = 0; j < saveSize; j++)
			{
				storage->write(startingAddress + j, *saveBytesPtr++);
			}

			startingAddress += saveSize;
		}
	}

	return startingAddress;
}

int OmxModeChords::loadFromDisk(int startingAddress, Storage *storage)
{
	int saveSize = sizeof(ChordSettings);

	for (uint8_t saveIndex = 0; saveIndex < NUM_CHORD_SAVES; saveIndex++)
	{
		for (uint8_t i = 0; i < 16; i++)
		{
			auto chord = ChordSettings{};
			auto current = (byte *)&chord;
			for (int j = 0; j < saveSize; j++)
			{
				*current = storage->read(startingAddress + j);
				current++;
			}

			chordSaves_[saveIndex][i] = chord;
			startingAddress += saveSize;
		}
	}

	loadPreset(0);

	return startingAddress;
}
