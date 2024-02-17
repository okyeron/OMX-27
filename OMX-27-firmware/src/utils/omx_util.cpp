#include <U8g2_for_Adafruit_GFX.h>

#include "omx_util.h"
#include "../consts/consts.h"
#include "../midi/midi.h"
#include "../consts/colors.h"
#include "../hardware/omx_leds.h"
#include "../hardware/omx_disp.h"
#include "../midi/noteoffs.h"
#include "../modes/sequencer.h"

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

void OmxUtil::advanceClock(OmxModeInterface *activeOmxMode, Micros advance)
{
	activeOmxMode_ = activeOmxMode;

	signed long long adv = advance;

	while (adv >= timeToNextClock)
	{
		adv -= timeToNextClock;

		if (sendClocks_)
		{
			MM::sendClock();
		}

		seqConfig.currentFrameMicros = micros();
		seqConfig.lastClockMicros = micros();

		if (activeOmxMode_ != nullptr)
		{
			activeOmxMode_->onClockTick();
		}

		timeToNextClock = clockConfig.ppqInterval * (PPQ / 24);
	}
	timeToNextClock = timeToNextClock - adv;
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
	clockConfig.step_micros = clockConfig.ppqInterval * (PPQ / 4);	   // 16th note step in microseconds (quarter of quarter note)

	// 16th note step length in milliseconds
	clockConfig.step_delay = clockConfig.step_micros * 0.001; // ppqInterval * 0.006; // 60000 / clockbpm / 4;
}

void OmxUtil::restartClocks()
{
	resetClocks();
	timeToNextClock = 0;
	seqConfig.currentFrameMicros = micros();
	seqConfig.lastClockMicros = seqConfig.currentFrameMicros;
}

void OmxUtil::startClocks()
{
	sendClocks_ = true;
	MM::startClock();
}

void OmxUtil::resumeClocks()
{
	sendClocks_ = true;
	MM::continueClock();
}

void OmxUtil::stopClocks()
{
	sendClocks_ = false;
	MM::stopClock();
}

bool OmxUtil::areClocksRunning()
{
	return sendClocks_;
}

void OmxUtil::cvNoteOn(int notenum)
{
	if (notenum >= midiLowestNote && notenum < midiHightestNote)
	{
		midiSettings.pitchCV = static_cast<int>(roundf((notenum - midiLowestNote) * stepsPerSemitone)); // map (adjnote, 36, 91, 0, 4080);
		digitalWrite(CVGATE_PIN, HIGH);
		//         analogWrite(CVPITCH_PIN, midiSettings.pitchCV);
#if T4
		dac.setVoltage(midiSettings.pitchCV, false);
#else
		analogWrite(CVPITCH_PIN, midiSettings.pitchCV);
#endif
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
void OmxUtil::midiNoteOn(MusicScales *scale, int notenum, int velocity, int channel)
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
		midiSettings.midiLastVel = velocity;

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
	for (uint8_t i = 0; i < 27; i++)
	{
		if (midiSettings.midiKeyState[i] >= 0)
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

MidiNoteGroup OmxUtil::midiNoteOn2(MusicScales *scale, int notenum, int velocity, int channel)
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
		midiSettings.midiLastVel = velocity;

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
		// MM::sendNoteOff(adjnote, 0, adjchan);
		// CV off
		// cvNoteOff();
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

MidiNoteGroup OmxUtil::midiDrumNoteOn(uint8_t keyIndex, uint8_t notenum, int velocity, int channel)
{
    MidiNoteGroup noteGroup;

	// Not a valid note
	if(notenum >= 128)
	{
		noteGroup.noteNumber = 255;
		return noteGroup;
	}

    // keep track of adjusted note when pressed so that when key is released we send
    // the correct note off message
    midiSettings.midiKeyState[keyIndex] = notenum;
    midiSettings.midiChannelState[keyIndex] = channel;

    noteGroup.noteNumber = notenum;
    noteGroup.velocity = velocity;
    noteGroup.channel = channel;
    noteGroup.stepLength = 0;
    noteGroup.sendMidi = true;
    noteGroup.sendCV = true;
    noteGroup.noteonMicros = micros();

    midiSettings.midiLastNote = notenum;
    midiSettings.midiLastVel = velocity;
	omxLeds.setDirty();
	omxDisp.setDirty();

	return noteGroup;
}

MidiNoteGroup OmxUtil::midiDrumNoteOff(uint8_t keyIndex)
{
	// we use the key state captured at the time we pressed the key to send the correct note off message
	int adjnote = midiSettings.midiKeyState[keyIndex];
	int adjchan = midiSettings.midiChannelState[keyIndex];

	MidiNoteGroup noteGroup;
	noteGroup.noteOff = true;

	if (adjnote >= 0 && adjnote < 128)
	{
		midiSettings.midiKeyState[keyIndex] = -1;

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

	omxLeds.setDirty();
	omxDisp.setDirty();

	return noteGroup;
}

OmxUtil omxUtil;
