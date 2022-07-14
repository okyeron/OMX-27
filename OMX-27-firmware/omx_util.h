#pragma once
#include "config.h"
#include "music_scales.h"

class OmxUtil
{
public:
    OmxUtil() {}

    void setup();

    void sendPots(int val, int channel);

    // #### Clocks, might want to put in own class
    void advanceClock(Micros advance);
    void advanceSteps(Micros advance);
    void setGlobalSwing(int swng_amt);
    void resetClocks();

    // #### Outbound CV note on/off
    void cvNoteOn(int notenum);
    void cvNoteOff();

    // #### Outbound MIDI note on/off
    void midiNoteOn(int notenum, int velocity, int channel);
    void midiNoteOn(MusicScales* scale, int notenum, int velocity, int channel);
    void midiNoteOff(int notenum, int channel);

private:
    // int potbank = 0;
    // int analogValues[5] = {0,0,0,0,0};		// default values
    // int potValues[5] = {0,0,0,0,0};
    // int potCC = pots[potbank][0];
    // int potVal = analogValues[0];
};

extern OmxUtil omxUtil;