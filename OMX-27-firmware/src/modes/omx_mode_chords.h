#pragma once

#include "../modes/omx_mode_interface.h"
#include "../utils/music_scales.h"
// #include "../consts/colors.h"
#include "../config.h"
#include "../globals.h"
// #include "../modes/omx_mode_midi_keyboard.h"
#include "../utils/param_manager.h"
#include "../hardware/storage.h"
#include "submodes/submode_interface.h"
#include "submodes/submode_midifxgroup.h"
#include "submodes/submode_preset.h"
#include "../midimacro/midimacro_m8.h"
#include "../midimacro/midimacro_norns.h"
#include "../midimacro/midimacro_deluge.h"
#include "../utils/chord_structs.h"
#include "../utils/chord_util.h"

#define NUM_CHORD_SAVES 8

// struct CustomChordDegree
// {
//     uint8_t note : 3; // 0 - 7
//     int8_t octave : 3; // Octave Offset -3 to +3
// };

class OmxModeChords : public OmxModeInterface
{
public:
	OmxModeChords();
	~OmxModeChords() {}

	void InitSetup() override;

	void onModeActivated() override;
	void onModeDeactivated() override;

	void onClockTick() override;

	void onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta) override;

	void loopUpdate(Micros elapsedTime) override;

	void updateLEDs() override;

	void onEncoderChanged(Encoder::Update enc) override;
	void onEncoderButtonDown() override;
	void onEncoderButtonDownLong() override;

	void inMidiControlChange(byte channel, byte control, byte value) override;


	bool shouldBlockEncEdit() override;

	void onKeyUpdate(OMXKeypadEvent e) override;
	void onKeyHeldUpdate(OMXKeypadEvent e) override;

	void onDisplayUpdate() override;
	void SetScale(MusicScales *scale);

	int saveToDisk(int startingAddress, Storage *storage);
	int loadFromDisk(int startingAddress, Storage *storage);

private:
	struct NoteTracker
	{
		int8_t triggerCount;
		uint8_t noteNumber : 7;
		uint8_t midiChannel : 4;
	};

	SubModePreset presetManager;

	// void savePreset(uint8_t saveIndex);
	// void loadPreset(uint8_t loadIndex);

	static void doSavePresetForwarder(void *context, uint8_t presetIndex)
	{
		static_cast<OmxModeChords *>(context)->savePreset(presetIndex);
	}

	static void doLoadPresetForwarder(void *context, uint8_t presetIndex)
	{
		static_cast<OmxModeChords *>(context)->loadPreset(presetIndex);
	}

	bool macroActive_ = false;

	midimacro::MidiMacroNorns nornsMarco_;
	midimacro::MidiMacroM8 m8Macro_;
	midimacro::MidiMacroDeluge delugeMacro_;

	midimacro::MidiMacroInterface *activeMacro_;
	midimacro::MidiMacroInterface *getActiveMacro();

	// Used by the Macros for playing normal notes
	static void doNoteOnForwarder(void *context, uint8_t keyIndex)
	{
		auto chordsInstance = static_cast<OmxModeChords *>(context);
		chordsInstance->doNoteOn(keyIndex, chordsInstance->mfxIndex_, midiSettings.defaultVelocity, sysSettings.midiChannel);
	}

	// Used by the Macros for playing normal notes
	static void doNoteOffForwarder(void *context, uint8_t keyIndex)
	{
		auto chordsInstance = static_cast<OmxModeChords *>(context);
		chordsInstance->doNoteOff(keyIndex, chordsInstance->mfxIndex_, sysSettings.midiChannel);
	}

	// If true, encoder selects param rather than modifies value
	bool auxDown_ = false;
	bool encoderSelect_ = false;
	bool chordEditMode_ = false;
	// bool splitKeyboardMode_ = false;

	bool mfxQuickEdit_ = false;
	uint8_t quickEditMfxIndex_ = 0;

	bool wrapManStrum_ = true;

	bool lastKeyWasKeyboard_ = false; // This gets set to true if the last key pressed was a keyboard key and not a chord key

	uint8_t incrementManStrum_ = 0;
	uint8_t manStrumSensit_ = 10;

	uint8_t selectedChord_ = 0;
	int8_t heldChord_ = -1;

	uint8_t selectedSave_ = 0;

	uint8_t uiMode_ = 0; // FULL, Split

	uint8_t mode_ = 0; // Play, Edit Chord, Presets, Manual Strum

	uint8_t manStrumNoteLength_ = 4;

	// ParamManager params_;
	ParamManager basicParams_;
	ParamManager intervalParams_;

	// ParamManager chordEditParams_;
	uint8_t funcKeyMode_ = 0;
	uint8_t chordEditParam_ = 0; // None, Octave, Transpose, Spread, Rotate, Voicing

	MusicScales *musicScale_;

	ChordSettings chords_[16];
	ChordNotes chordNotes_[16];

	ChordNotes playedChordNotes_[16];

	ChordNotes chordEditNotes_;
	int8_t activeChordEditDegree_;
	int8_t activeChordEditNoteKey_;
	int8_t activeSplitKeyIndex_;

	ChordBalanceDetails activeChordBalance_;

	ChordSettings chordSaves_[NUM_CHORD_SAVES][16];

	// int saveSize = sizeof(chordSaves_);

	String notesString = "";
	String notesString2 = "";

	String customNotesStrings[6];

	// SubModes
	SubmodeInterface *activeSubmode = nullptr;

	uint8_t mfxIndex_ = 0;

	bool lockScaleCache_ = false; // Cache value when entering mode, restore on exit
	bool grp16ScaleCache_ = false;

	int noNotes[6] = {-1, -1, -1, -1, -1, -1};

	const uint8_t kMaxNoteTrackerSize = 32;

	std::vector<NoteTracker> noteOffTracker;

	// int chordSize = sizeof(chords_);

	ParamManager *getParams();
	void setSelPageAndParam(int8_t newPage, int8_t newParam);

	void allNotesOff();
	void updateFuncKeyMode();
	void onEncoderChangedEditParam(Encoder::Update *enc, uint8_t selectedParmIndex, uint8_t targetParamIndex, uint8_t paramType);
	void onEncoderChangedManStrum(Encoder::Update enc);
	void onKeyUpdateChordEdit(OMXKeypadEvent e);
	void enterChordEditMode();
	void updateLEDsChordEdit();
	void setupPageLegends();
	void setupPageLegend(uint8_t index, uint8_t paramType);

	bool pasteSelectedChordTo(uint8_t chordIndex);
	bool loadPreset(uint8_t presetIndex);
	bool savePreset(uint8_t presetIndex);

	void onManualStrumOn(uint8_t chordIndex);
	void onChordOn(uint8_t chordIndex);
	void onChordOff(uint8_t chordIndex);
	void onChordEditOn(uint8_t chordIndex);
	void onChordEditOff();

	bool constructChord(uint8_t chordIndex);
	bool constructChordBasic(uint8_t chordIndex);

	static int AddOctave(int note, int8_t octave);
	static int TransposeNote(int note, int8_t semitones);

	ChordBalanceDetails getChordBalanceDetails(uint8_t balance);

	void enableSubmode(SubmodeInterface *subMode);
	void disableSubmode();
	bool isSubmodeEnabled();

	bool getEncoderSelect();

	void selectMidiFx(uint8_t mfxIndex, bool dispMsg);
	void selectMidiFxChordKey(int8_t mfxIndex, bool dispMsg);
	bool onKeyUpdateSelMidiFX(OMXKeypadEvent e);
	bool onKeyHeldSelMidiFX(OMXKeypadEvent e);

	void doNoteOn(int noteNumber, uint8_t midifx, uint8_t velocity, uint8_t midiChannel);
	void doNoteOff(int noteNumber, uint8_t midifx, uint8_t midiChannel);

	void splitNoteOn(uint8_t keyIndex);
	void splitNoteOff(uint8_t keyIndex);

	void stopSequencers();

	static void onNotePostFXForwarder(void *context, MidiNoteGroup note)
	{
		static_cast<OmxModeChords *>(context)->onNotePostFX(note);
	}
	void onNotePostFX(MidiNoteGroup note);

	static void onPendingNoteOffForwarder(void *context, int note, int channel)
	{
		static_cast<OmxModeChords *>(context)->onPendingNoteOff(note, channel);
	}

	void onPendingNoteOff(int note, int channel);
};
