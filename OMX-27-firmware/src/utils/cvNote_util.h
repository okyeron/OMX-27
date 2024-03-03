#pragma once

#include <stdint.h>
#include <vector>

enum CVTriggerModes
{
    CVTRIGMODE_LEGATO,
    CVTRIGMODE_RETRIG,
};

class CVNoteUtil
{
public:
    CVNoteUtil();
    ~CVNoteUtil();

    uint8_t triggerMode = CVTRIGMODE_LEGATO;

    void loopUpdate(unsigned long elapsedTime);

    void cvNoteOn(uint8_t notenum);
    void cvNoteOff(uint8_t notenum);

    const char* getTriggerModeDispName();

    int cvPitch;

//     static const uint8_t midiMiddleC = 60;
// static const uint8_t midiLowestNote = midiMiddleC - 3 * 12; // 3 is how many octaves under middle c
// static const int midiHightestNote = midiLowestNote + int(fullRangeV * 12) - 1;

private:
    struct CVNoteTracker
    {
        uint8_t cvNote : 6; // 0 - 54, note gets 24 added to it
    };

    bool pulseGate;

    unsigned long lastNoteOnTime;

    static const uint8_t trackedSize = 16;

    std::vector<CVNoteTracker> cvNotes_; // Keeps track of which notes are being played

    static bool isNoteValid(uint8_t midiNoteNum);

    static void setGate(bool high);
    void setPitch(uint8_t cvNoteNum);

    static uint8_t midi2CVNote(uint8_t noteNumber);
    static uint8_t cv2MidiNote(uint8_t noteNumber);
};

extern CVNoteUtil cvNoteUtil;