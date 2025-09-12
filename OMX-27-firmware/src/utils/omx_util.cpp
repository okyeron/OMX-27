#include <U8g2_for_Adafruit_GFX.h>

#include "omx_util.h"
#include "../consts/consts.h"
#include "../midi/midi.h"
#include "../consts/colors.h"
#include "../hardware/omx_leds.h"
#include "../hardware/omx_disp.h"
#include "../midi/noteoffs.h"
#include "../modes/sequencer.h"
#include "cvNote_util.h"

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

float OmxUtil::randFloat()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

float OmxUtil::lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

void OmxUtil::advanceClock(OmxModeInterface *activeOmxMode, Micros advance)
{
	// advance is delta in Micros from previous loop update to this loop update. 

	// XXXXXXXXXXXXXXXXXXXXXXXX
	// Txxxxxxxxxxxxxxxxxxxxxxx - Quarter Note - 24 ticks
	// TxxxxxxxxxxxTxxxxxxxxxxx - 8th note - 12 ticks
	// TxxxxxTxxxxxTxxxxxTxxxxx - 16th note - 6 ticks
	// TxxTxxTxxTxxTxxTxxTxxTxx - 32nd note - 3 ticks

	// TxxxxxT
	// xTxxxxT
	// xxTxxxT


	activeOmxMode_ = activeOmxMode;

	signed long long adv = advance;

	// Not sure what advantage of doing the time comparison
	// in a while loop like this is
	// Maybe so if there is a long advance multiple clocks
	// will get fired to catch up?
	// Keeping like this for now as it works. 
	while (adv >= timeToNextClock)
	{
		adv -= timeToNextClock;

		// if (sendClocks_)
		// {
		// 	MM::sendClock();
		// }

		seqConfig.currentFrameMicros = micros();
		seqConfig.lastClockMicros = micros();

		// if(seqConfig.currentClockTick == 0)
		// {
		// 	Serial.println("Quarter Note");
		// }

		// Midi Clock should be sent out at ppq of 24
		// Since ppq is 96, every 4 clock ticks should send midi clock
		if(seqConfig.midiOutClockTick % 4 == 0)
		{
			// Should always send clock
			// This way external gear can update themselves
			if (clockConfig.send_always)
			{
				MM::sendClock();
			}
		}

		if (activeOmxMode_ != nullptr)
		{
			// Update internally at PPQ of 94
			activeOmxMode_->onClockTick();
		}

		// timeToNextClock = clockConfig.ppqInterval * (PPQ / 24); // ppqInt=5.208ms * 4 = 20.83 milliseconds for 120 bpm, 120 bpm = 2 beats per second, a beat being a quarter note

		seqConfig.midiOutClockTick = (seqConfig.midiOutClockTick + 1) % PPQ;
		seqConfig.currentClockTick = (seqConfig.currentClockTick + 1) % (PPQ * 4);

		timeToNextClock = clockConfig.ppqInterval;
	}
	timeToNextClock = timeToNextClock - adv;
}

void OmxUtil::resetPPQCounter()
{
	seqConfig.currentClockTick = 0;
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
	// 60000000 = 60 secs
	clockConfig.ppqInterval = 60000000 / (PPQ * clockConfig.clockbpm); // ppq interval is in microseconds, 96 * 120 = 11520, 60000000 / 11520 = 52083 microsecond, * 0.001 = 5.208 milliseconds, 
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
	clockConfig.send_always = true;
	MM::startClock();
}

void OmxUtil::resumeClocks()
{
	sendClocks_ = true;
	clockConfig.send_always = true;
	MM::continueClock();
}

void OmxUtil::stopClocks()
{
	sendClocks_ = false;
	clockConfig.send_always = false;
	MM::stopClock();
}

bool OmxUtil::areClocksRunning()
{
	return sendClocks_;
}

// void OmxUtil::cvNoteOn(uint8_t notenum)
// {
// 	if (notenum >= cvLowestNote && notenum < cvHightestNote)
// 	{
// 		midiSettings.pitchCV = static_cast<int>(roundf((notenum - cvLowestNote) * stepsPerSemitone)); // map (adjnote, 36, 91, 0, 4080);
// 		digitalWrite(CVGATE_PIN, HIGH);
// 		//         analogWrite(CVPITCH_PIN, midiSettings.pitchCV);
// #if T4
// 		dac.setVoltage(midiSettings.pitchCV, false);
// #else
// 		analogWrite(CVPITCH_PIN, midiSettings.pitchCV);
// #endif
// 	}
// }
// void OmxUtil::cvNoteOff(uint8_t notenum)
// {
// 	if (notenum >= cvLowestNote && notenum < cvHightestNote)
// 	{
// 		digitalWrite(CVGATE_PIN, LOW);
// 	//	analogWrite(CVPITCH_PIN, 0);
// 	}
// }

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
		cvNoteUtil.cvNoteOn(adjnote);
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
		cvNoteUtil.cvNoteOff(adjnote);
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

void OmxUtil::onEncoderChangedEditParam(Encoder::Update *enc, uint8_t selectedParmIndex, uint8_t targetParamIndex, uint8_t paramType)
{
	onEncoderChangedEditParam(enc, nullptr, selectedParmIndex, targetParamIndex, paramType);
}

void OmxUtil::onEncoderChangedEditParam(Encoder::Update *enc, MusicScales *musicScale, uint8_t selectedParmIndex, uint8_t targetParamIndex, uint8_t paramType)
{
	if (selectedParmIndex != targetParamIndex)
		return;

	auto amtSlow = enc->accel(1);
	auto amtFast = enc->accel(5);

	switch (paramType)
	{
	case GPARAM_MOUT_OCT:
	{
		midiSettings.octave = constrain(midiSettings.octave + amtSlow, -5, 4);
	}
	break;
	case GPARAM_MOUT_CHAN:
	{
		sysSettings.midiChannel = constrain(sysSettings.midiChannel + amtSlow, 1, 16);
	}
	break;
	case GPARAM_MOUT_VEL:
	{
		midiSettings.defaultVelocity = constrain((int)midiSettings.defaultVelocity + amtFast, 0, 127); // cast to int to prevent rollover
	}
	break;
	case GPARAM_MIDI_THRU:
	{
		midiSettings.midiSoftThru = constrain(midiSettings.midiSoftThru + amtSlow, 0, 1);
	}
	break;
	case GPARAM_POTS_PBANK:
	{
		potSettings.potbank = constrain(potSettings.potbank + amtSlow, 0, NUM_CC_BANKS - 1);
	}
	break;
	case GPARAM_SCALE_ROOT:
	{
		if (musicScale != nullptr)
		{
			int prevRoot = scaleConfig.scaleRoot;
			scaleConfig.scaleRoot = constrain(scaleConfig.scaleRoot + amtSlow, 0, 12 - 1);
			if (prevRoot != scaleConfig.scaleRoot)
			{
				musicScale->calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
			}
		}
	}
	break;
	case GPARAM_SCALE_PAT:
	{
		if (musicScale != nullptr)
		{
			int prevPat = scaleConfig.scalePattern;
			scaleConfig.scalePattern = constrain(scaleConfig.scalePattern + amtSlow, -1, musicScale->getNumScales() - 1);
			if (prevPat != scaleConfig.scalePattern)
			{
				omxDisp.displayMessage(musicScale->getScaleName(scaleConfig.scalePattern));
				musicScale->calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
			}
		}
	}
	break;
	case GPARAM_SCALE_LOCK:
	{
		scaleConfig.lockScale = constrain(scaleConfig.lockScale + amtSlow, 0, 1);
	}
	break;
	case GPARAM_SCALE_GRP16:
	{
		scaleConfig.group16 = constrain(scaleConfig.group16 + amtSlow, 0, 1);
	}
	break;
	case GPARAM_MACRO_MODE:
	{
		midiMacroConfig.midiMacro = constrain(midiMacroConfig.midiMacro + amtSlow, 0, nummacromodes);
	}
	break;
	case GPARAM_MACRO_CHAN:
	{
		midiMacroConfig.midiMacroChan = constrain(midiMacroConfig.midiMacroChan + amtSlow, 1, 16);
	}
	break;
	case GPARAM_MIDI_LASTNOTE:
	case GPARAM_MIDI_LASTVEL:
	case GPARAM_POTS_LASTVAL:
	case GPARAM_POTS_LASTCC:
	{
		Serial.println("Param not editable: ");
		Serial.println(paramType);
	}
	break;
	}
}

void OmxUtil::setupPageLegend(uint8_t index, uint8_t paramType)
{
	setupPageLegend(nullptr, index, paramType);
}

void OmxUtil::setupPageLegend(MusicScales *musicScale, uint8_t index, uint8_t paramType)
{
	switch (paramType)
	{
	case GPARAM_MOUT_OCT:
	{
		omxDisp.legends[index] = "OCT";
		omxDisp.legendVals[index] = (int)midiSettings.octave + 4;
	}
	break;
	case GPARAM_MOUT_CHAN:
	{
		omxDisp.legends[index] = "CH";
		omxDisp.legendVals[index] = sysSettings.midiChannel;
	}
	break;
	case GPARAM_MOUT_VEL:
	{
		omxDisp.legends[index] = "VEL";
		omxDisp.legendVals[index] = midiSettings.defaultVelocity;
	}
	break;
	case GPARAM_MIDI_THRU:
	{
		omxDisp.legends[index] = "THRU"; // MIDI thru (usb to hardware)
		omxDisp.legendText[index] = midiSettings.midiSoftThru ? "On" : "Off";
	}
	break;
	case GPARAM_MIDI_LASTNOTE:
	{
		omxDisp.legends[index] = "NOTE"; 
		omxDisp.legendVals[index] = midiSettings.midiLastNote;
	}
	break;
	case GPARAM_MIDI_LASTVEL:
	{
		omxDisp.legends[index] = "VEL"; 
		omxDisp.legendVals[index] = midiSettings.midiLastVel;
	}
	break;
	case GPARAM_POTS_LASTCC:
	{
		omxDisp.legends[index] = "P CC";
		omxDisp.legendVals[index] = potSettings.potCC;
	}
	break;
	case GPARAM_POTS_LASTVAL:
	{
		omxDisp.legends[index] = "P VAL";
		omxDisp.legendVals[index] = potSettings.potVal;
	}
	break;
	case GPARAM_POTS_PBANK:
	{
		omxDisp.legends[index] = "PBNK"; // Potentiometer Banks
		omxDisp.legendVals[index] = potSettings.potbank + 1;
	}
	break;
	case GPARAM_SCALE_ROOT:
	{
		if(musicScale != nullptr)
		{
			omxDisp.legends[index] = "ROOT";
			omxDisp.legendText[index] = musicScale->getNoteName(scaleConfig.scaleRoot);
		}
	}
	break;
	case GPARAM_SCALE_PAT:
	{
		omxDisp.legends[index] = "SCALE";

		if (scaleConfig.scalePattern < 0)
		{
			omxDisp.legendText[index] = "CHRM";
		}
		else
		{
			omxDisp.legendVals[index] = scaleConfig.scalePattern;
		}
	}
	break;
	case GPARAM_SCALE_LOCK:
	{
		omxDisp.legends[index] = "LOCK";
		omxDisp.legendText[index] = scaleConfig.lockScale ? "On" : "Off";
	}
	break;
	case GPARAM_SCALE_GRP16:
	{
		omxDisp.legends[index] = "GROUP";
		omxDisp.legendText[index] = scaleConfig.group16 ? "On" : "Off";
	}
	break;
	case GPARAM_MACRO_MODE:
	{
		omxDisp.legends[index] = "MCRO"; // Macro mode
		omxDisp.legendText[index] = macromodes[midiMacroConfig.midiMacro];
	}
	break;
	case GPARAM_MACRO_CHAN:
	{
		omxDisp.legends[index] = "M-CH";
		omxDisp.legendVals[index] = midiMacroConfig.midiMacroChan;
	}
	break;
	}
}

OmxUtil omxUtil;
