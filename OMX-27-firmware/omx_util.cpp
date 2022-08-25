#include <U8g2_for_Adafruit_GFX.h>

#include "omx_util.h"
#include "consts.h"
#include "MM.h"
#include "colors.h"
#include "omx_leds.h"
#include "omx_disp.h"
#include "noteoffs.h"
#include "sequencer.h"

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

void OmxUtil::advanceClock(OmxModeInterface* activeOmxMode, Micros advance)
{
	static Micros timeToNextClock = 0;
	while (advance >= timeToNextClock)
	{
		advance -= timeToNextClock;

		MM::sendClock();

        seqConfig.currentFrameMicros = micros();
        seqConfig.lastClockMicros = micros();

        if (activeOmxMode != nullptr)
        {
            activeOmxMode->onClockTick();
        }

        timeToNextClock = clockConfig.ppqInterval * (PPQ / 24);
	}
	timeToNextClock -= advance;
}

void OmxUtil::advanceSteps(Micros advance)
{
	static Micros timeToNextStep = 0;
	//	static Micros stepnow = micros();
	while (advance >= timeToNextStep)
	{
		advance -= timeToNextStep;
		timeToNextStep = clockConfig.ppqInterval;

        auto currentMicros = micros();

        pendingNoteHistory.clearIfChanged(currentMicros);

        // turn off any expiring notes
		pendingNoteOffs.play(currentMicros);

        // turn on any pending notes
		pendingNoteOns.play(currentMicros);
	}
	timeToNextStep -= advance;
}

void OmxUtil::setGlobalSwing(int swng_amt)
{
	for (int z = 0; z < NUM_SEQ_PATTERNS; z++)
	{
		sequencer.getPattern(z)->swing = swng_amt;
	}
}

void OmxUtil::resetClocks()
{
	// BPM tempo to step_delay calculation
	clockConfig.ppqInterval = 60000000 / (PPQ * clockConfig.clockbpm); // ppq interval is in microseconds
	clockConfig.step_micros = clockConfig.ppqInterval * (PPQ / 4);				   // 16th note step in microseconds (quarter of quarter note)

	// 16th note step length in milliseconds
	clockConfig.step_delay = clockConfig.step_micros * 0.001; // ppqInterval * 0.006; // 60000 / clockbpm / 4;
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

void OmxUtil::midiNoteOn(int notenum, int velocity, int channel)
{
    midiNoteOn(nullptr, notenum, velocity, channel);
}

// #### Outbound MIDI Mode note on/off
void OmxUtil::midiNoteOn(MusicScales* scale, int notenum, int velocity, int channel)
{
    int adjnote = notes[notenum] + (midiSettings.octave * 12); // adjust key for octave range

    if (scale != nullptr)
    {
        if (scaleConfig.group16)
        {
            adjnote = scale->getGroup16Note(notenum, midiSettings.octave);
        }
        else
        {
            if (scaleConfig.lockScale && scale->isNoteInScale(adjnote) == false)
                return; // Only play note if in scale
        }
    }

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
    else
    {
        return; // no note sent, don't light LEDs
    }

    strip.setPixelColor(notenum, MIDINOTEON); //  Set pixel's color (in RAM)
    omxLeds.setDirty();
    omxDisp.setDirty();
}

void OmxUtil::allOff()
{
    for(uint8_t i = 0; i < 27; i++)
    {
        if(midiSettings.midiKeyState[i] >= 0)
        {
            midiNoteOff(i, midiSettings.midiChannelState[i]);
        }
    }
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

MidiNoteGroup OmxUtil::midiNoteOn2(MusicScales* scale, int notenum, int velocity, int channel)
{
    int adjnote = notes[notenum] + (midiSettings.octave * 12); // adjust key for octave range

    MidiNoteGroup noteGroup;

    if (scale != nullptr)
    {
        if (scaleConfig.group16)
        {
            adjnote = scale->getGroup16Note(notenum, midiSettings.octave);
        }
        else
        {
            if (scaleConfig.lockScale && scale->isNoteInScale(adjnote) == false)
            {
                noteGroup.noteNumber = 255;
                return noteGroup; // Only play note if in scale
            }
        }
    }

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

        noteGroup.noteNumber = adjnote;
        noteGroup.velocity = velocity;
        noteGroup.channel = adjchan;
        noteGroup.stepLength = 0;
        noteGroup.sendMidi = true;
        noteGroup.sendCV = true;
        noteGroup.noteonMicros = micros();

        // MM::sendNoteOn(adjnote, velocity, adjchan);
        // // CV
        // cvNoteOn(adjnote);
    }
    else
    {
        noteGroup.noteNumber = 255;
        return noteGroup; // no note sent, don't light LEDs
    }

    strip.setPixelColor(notenum, MIDINOTEON); //  Set pixel's color (in RAM)
    omxLeds.setDirty();
    omxDisp.setDirty();

    return noteGroup;
}

MidiNoteGroup OmxUtil::midiNoteOff2(int notenum, int channel)
{
    // we use the key state captured at the time we pressed the key to send the correct note off message
    int adjnote = midiSettings.midiKeyState[notenum];
    int adjchan = midiSettings.midiChannelState[notenum];

    MidiNoteGroup noteGroup;
    noteGroup.noteOff = true;

    if (adjnote >= 0 && adjnote < 128)
    {
        MM::sendNoteOff(adjnote, 0, adjchan);
        // CV off
        cvNoteOff();
        midiSettings.midiKeyState[notenum] = -1;

        noteGroup.noteNumber = adjnote;
        noteGroup.velocity = 0;
        noteGroup.channel = adjchan;
        noteGroup.stepLength = 0;
        noteGroup.sendMidi = true;
        noteGroup.sendCV = true;
        noteGroup.noteonMicros = micros();
    }
    else
    {
        noteGroup.noteNumber = 255;
        return noteGroup;
    }

    strip.setPixelColor(notenum, LEDOFF);
    omxLeds.setDirty();
    omxDisp.setDirty();

    return noteGroup;
}


OmxUtil omxUtil;