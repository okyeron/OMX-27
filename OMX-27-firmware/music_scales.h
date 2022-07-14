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

    // returns true if note 0-11 is in the currently calculated scale
    bool isNoteInScale(int noteIndex);

    // Returns a color for the note
    int getScaleColor(int noteIndex);

    const char* getNoteName(int noteIndex);
    const char* getScaleName(int scaleIndex);
    int getScaleLength();


private:
    bool scaleCalculated = false;
    int scaleOffsets[12];
    int scaleDegrees[12];
    int scaleColors[12];
    int scaleLength = 0;
};