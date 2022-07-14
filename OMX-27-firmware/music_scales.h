#pragma once

class MusicScales
{
public:
    // int scaleDegrees[12];
    // int scaleOffsets[12];
    // int scaleColors[12];
    // const char* scaleNames[];
    // const char* noteNames[];
    // const int scalePatterns[][7];

    void calculateScale(int scaleRoot, int scalePattern);
    int getNumScales();
    // int scaleLength;

    // returns true if note 0-11 is in the currently calculated scale. NoteNum should be a midi note
    bool isNoteInScale(int noteNum);

    // returns a note in the scale if key is one of the lower 16. Returns -1 otherwise. 
    int getGroup16Note(int keyNum, int octave);

    // Returns a color for the note
    int getScaleColor(int noteIndex);

    int getGroup16Color(int keyNum);

    const char* getNoteName(int noteIndex);
    const char* getScaleName(int scaleIndex);
    int getScaleLength();


private:
    bool scaleCalculated = false;
    int scaleOffsets[12];
    int scaleDegrees[12];
    int scaleColors[12];
    int scaleLength = 0;

    int rootNote;
    int scaleIndex;
    
    int group16Offsets[16]; // 16 offsets for group16 mode
};