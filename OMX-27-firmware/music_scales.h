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

    void calculateScale(uint8_t scaleRoot, uint8_t scalePattern);
    static uint8_t getNumScales();
    // int scaleLength;

    // returns true if note 0-11 is in the currently calculated scale. NoteNum should be a midi note
    bool isNoteInScale(int8_t noteNum);

    // returns a note in the scale if key is one of the lower 16. Returns -1 otherwise. 
    // TODO This won't work unless returns int, won't work with int8_t not sure why
    int getGroup16Note(uint8_t keyNum, int8_t octave);

    int8_t getNoteByDegree(uint8_t degree, int8_t octave);

    // Returns a color for the note
    int getScaleColor(uint8_t noteIndex);

    int getGroup16Color(uint8_t keyNum);

    static const char* getNoteName(uint8_t noteIndex);
    static const char* getScaleName(uint8_t scaleIndex);
    static const int8_t* getScalePattern(uint8_t noteIndex);
    int getScaleLength();


private:
    bool scaleCalculated = false;
    int8_t scaleOffsets[12];
    int8_t scaleDegrees[12];
    int scaleColors[12];
    uint8_t scaleLength = 0;

    int8_t rootNote;
    int8_t scaleIndex;
    
    int group16Offsets[16]; // 16 offsets for group16 mode
};