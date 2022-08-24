#pragma once

#include "omx_mode_interface.h"
#include "music_scales.h"
// #include "colors.h"
#include "config.h"
// #include "omx_mode_midi_keyboard.h"
#include "param_manager.h"
#include "storage.h"
#include "submode_interface.h"
#include "submode_midifxgroup.h"

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

#define NUM_CHORD_SAVES 8

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
    uint8_t chord;

    // Interval Type:
    uint8_t numNotes : 3;
    uint8_t degree : 3; // degree from root note of scale, if scale is cmaj, degree of 0 = c, degree of 3 = e
    int8_t octave : 4; // transposes note by octave
    int8_t transpose : 5; // transposes note by semitone, can bump off scale
    int8_t spread : 4; // spreads chord notes over octave
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
        
        numNotes = 3;
        degree = 0;
        octave = 0;
        transpose = 0;
        spread = 0;
        rotate = 0;
        spreadUpDown = false;
        quartalVoicing = false;
        voicing = 0;
    }

    void CopySettingsFrom(ChordSettings other)
    {
        this->type = other.type;
        this->midiFx = other.midiFx;
        this->mchan = other.mchan;
        this->velocity = other.velocity;

        // Basic Type:
        this->note = other.note;
        this->basicOct = other.basicOct;
        this->chord = other.chord;

        this->numNotes = other.numNotes;
        this->degree = other.degree;
        this->octave = other.octave;
        this->transpose = other.transpose;
        this->spread = other.spread;
        this->rotate = other.rotate;
        this->spreadUpDown = other.spreadUpDown;
        this->quartalVoicing = other.quartalVoicing;
        this->voicing = other.voicing;
    }
};

struct ChordNotes
{
    bool active = false;
    uint8_t channel = 0;
    // uint8_t velocity = 100;
    int notes[6] = {-1,-1,-1,-1,-1,-1};
    int8_t strumPos = 0;
    int8_t encDelta = 0;
    int8_t octIncrement = 0;
};

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

    bool shouldBlockEncEdit() override;

    void onKeyUpdate(OMXKeypadEvent e) override;
    void onKeyHeldUpdate(OMXKeypadEvent e) override;

    void onDisplayUpdate() override;
    void SetScale(MusicScales* scale);

    int saveToDisk(int startingAddress, Storage *storage);
    int loadFromDisk(int startingAddress, Storage *storage);
private:
    // If true, encoder selects param rather than modifies value
    bool auxDown_ = false;
    bool encoderSelect_ = false;
    bool chordEditMode_ = false;
    // bool splitKeyboardMode_ = false;

    bool wrapManStrum_ = true;
    uint8_t incrementManStrum_ = 0;
    uint8_t manStrumSensit_ = 10;

    uint8_t selectedChord_ = 0;
    int8_t heldChord_ = -1;

    uint8_t selectedSave_ = 0;

    uint8_t uiMode_ = 0; // FULL, Split

    uint8_t mode_ = 0; // Play, Edit Chord, Presets, Manual Strum

    uint8_t manStrumNoteLength_ = 4;

    ParamManager params_;
    // ParamManager chordEditParams_;
    uint8_t funcKeyMode_ = 0;
    uint8_t chordEditParam_ = 0; // None, Octave, Transpose, Spread, Rotate, Voicing 

    MusicScales* musicScale_;

    ChordSettings chords_[16];
    ChordNotes chordNotes_[16];

    ChordNotes chordEditNotes_;
    uint8_t activeChordEditDegree_;

    ChordSettings chordSaves_[NUM_CHORD_SAVES][16];

    String notesString = "";
    String notesString2 = "";

     // SubModes
    SubmodeInterface* activeSubmode = nullptr;

    uint8_t mfxIndex_ = 0;



    // int chordSize = sizeof(chords_);

    void updateFuncKeyMode();
    void onEncoderChangedEditParam(Encoder::Update* enc, uint8_t selectedParmIndex, uint8_t targetParamIndex, uint8_t paramType);
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


    void enableSubmode(SubmodeInterface* subMode);
    void disableSubmode();
    bool isSubmodeEnabled();

    bool getEncoderSelect();

    void selectMidiFx(uint8_t mfxIndex, bool dispMsg);
    bool onKeyUpdateSelMidiFX(OMXKeypadEvent e);
    bool onKeyHeldSelMidiFX(OMXKeypadEvent e);

    void doNoteOn(int noteNumber, uint8_t velocity, uint8_t midiChannel);
    void doNoteOff(int noteNumber, uint8_t midiChannel);

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