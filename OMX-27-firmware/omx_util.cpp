#include <U8g2_for_Adafruit_GFX.h>

#include "omx_util.h"
#include "consts.h"
#include "MM.h"
#include "colors.h"
#include "omx_leds.h"
#include "omx_disp.h"

void OmxUtil::setup()
{
}

void OmxUtil::sendPots(int val, int channel)
{
    MM::sendControlChange(pots[potSettings.potbank][val], potSettings.analogValues[val], channel);
    potSettings.potCC = pots[potSettings.potbank][val];
    potSettings.potVal = potSettings.analogValues[val];
    potSettings.potValues[val] = potSettings.potVal;
}

void OmxUtil::cvNoteOn(int notenum)
{
    if (notenum >= midiLowestNote && notenum < midiHightestNote)
    {
        midiSettings.pitchCV = static_cast<int>(roundf((notenum - midiLowestNote) * stepsPerSemitone)); // map (adjnote, 36, 91, 0, 4080);
        digitalWrite(CVGATE_PIN, HIGH);
        analogWrite(CVPITCH_PIN, midiSettings.pitchCV);
    }
}
void OmxUtil::cvNoteOff()
{
    digitalWrite(CVGATE_PIN, LOW);
    //	analogWrite(CVPITCH_PIN, 0);
}

// #### Outbound MIDI Mode note on/off
void OmxUtil::midiNoteOn(int notenum, int velocity, int channel)
{
    int adjnote = notes[notenum] + (midiSettings.octave * 12); // adjust key for octave range
    midiSettings.rrChannel = (midiSettings.rrChannel % midiSettings.midiRRChannelCount) + 1;
    int adjchan = midiSettings.rrChannel;

    if (adjnote >= 0 && adjnote < 128)
    {
        midiSettings.midiLastNote = adjnote;

        // keep track of adjusted note when pressed so that when key is released we send
        // the correct note off message
        midiSettings.midiKeyState[notenum] = adjnote;

        // RoundRobin Setting?
        if (midiSettings.midiRoundRobin)
        {
            adjchan = midiSettings.rrChannel + midiSettings.midiRRChannelOffset;
        }
        else
        {
            adjchan = channel;
        }
        midiSettings.midiChannelState[notenum] = adjchan;
        MM::sendNoteOn(adjnote, velocity, adjchan);
        // CV
        cvNoteOn(adjnote);
    }

    strip.setPixelColor(notenum, MIDINOTEON); //  Set pixel's color (in RAM)
    omxLeds.setDirty();
    omxDisp.setDirty();
}

void OmxUtil::midiNoteOff(int notenum, int channel)
{
    // we use the key state captured at the time we pressed the key to send the correct note off message
    int adjnote = midiSettings.midiKeyState[notenum];
    int adjchan = midiSettings.midiChannelState[notenum];
    if (adjnote >= 0 && adjnote < 128)
    {
        MM::sendNoteOff(adjnote, 0, adjchan);
        // CV off
        cvNoteOff();
        midiSettings.midiKeyState[notenum] = -1;
    }

    strip.setPixelColor(notenum, LEDOFF);
    omxLeds.setDirty();
    omxDisp.setDirty();
}

OmxUtil omxUtil;