#include "omx_mode_chords.h"
#include "config.h"
#include "colors.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "sequencer.h"
#include "MM.h"
#include "noteoffs.h"

enum ChordsModePage {
    CHRDPAGE_NOTES,
    CHRDPAGE_1, // Root, Scale, Octave, Midi Channel
    CHRDPAGE_2, // numNotes, degree, octave, transpose
    CHRDPAGE_3, // spread, rotate, voicing
    CHRDPAGE_4, // spreadUpDown, quartalVoicing
};

enum ChordsMainMode {
    CHRDMODE_PLAY,
    CHRDMODE_EDIT,
    CHRDMODE_PRESET,
    CHRDMODE_MANSTRUM,
};

const char* kVoicingNames[8] = {"NONE", "POWR", "SUS2", "SUS4", "SU24", "+6", "+6+9", "KB11"};



const int kDegreeColor = ORANGE;
const int kDegreeSelColor = 0xFFBF80;
const int kNumNotesColor = BLUE;
const int kNumNotesSelColor = 0x9C9CFF;
const int kSpreadUpDownOnColor = RED;
const int kSpreadUpDownOffColor = 0x550000;
const int kQuartalVoicingOnColor = MAGENTA;
const int kQuartalVoicingOffColor = 0x500050;
const int kOctaveColor = BLUE;
const int kTransposeColor = BLUE;
const int kSpreadColor = BLUE;
const int kRotateColor = BLUE;
const int kVoicingColor = BLUE;

const int kPlayColor = ORANGE;
const int kEditColor = DKRED;
const int kPresetColor = DKGREEN;

OmxModeChords::OmxModeChords()
{
    params_.addPage(1);
    params_.addPage(4);
    params_.addPage(4);
    params_.addPage(4);
    params_.addPage(4);

    // 808 Colors
    for(uint8_t i = 0; i < 16; i++)
    {
        if(i >= 0 && i < 4)
        {
            chords_[i].color = RED; // Red
        }
        else if(i >= 4 && i < 8)
        {
            chords_[i].color = ORANGE; // Orange
        }
        else if(i >= 8 && i < 12)
        {
            chords_[i].color = YELLOW; // Yellow
        }
        else if(i >= 12)
        {
            chords_[i].color = 0xcfc08f; // Creme
        }
    }

    // chords_[0].numNotes = 3;
    // chords_[0].degree = 0;

    // chords_[1].numNotes = 3;
    // chords_[1].degree = 1;

    // chords_[2].numNotes = 4;
    // chords_[2].degree = 0;

    // chords_[3].numNotes = 4;
    // chords_[3].degree = 1;
}

void OmxModeChords::InitSetup()
{
}

void OmxModeChords::onModeActivated()
{
    params_.setSelPageAndParam(0,0);
    encoderSelect_ = true;

    sequencer.playing = false;
    stopSequencers();

    omxLeds.setDirty();
    omxDisp.setDirty();

    for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    {
        subModeMidiFx[i].setEnabled(true);
        subModeMidiFx[i].onModeChanged();
        subModeMidiFx[i].setNoteOutputFunc(&OmxModeChords::onNotePostFXForwarder, this);
    }

    pendingNoteOffs.setNoteOffFunction(&OmxModeChords::onPendingNoteOffForwarder, this);

    selectMidiFx(mfxIndex_, false);
}

void OmxModeChords::onModeDeactivated()
{
    sequencer.playing = false;
    stopSequencers();

    for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    {
        subModeMidiFx[i].setEnabled(false);
        subModeMidiFx[i].onModeChanged();
    }
}

void OmxModeChords::stopSequencers()
{
	MM::stopClock();
    pendingNoteOffs.allOff();
}

void OmxModeChords::selectMidiFx(uint8_t mfxIndex, bool dispMsg)
{
    this->mfxIndex_ = mfxIndex;

    for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    {
        subModeMidiFx[i].setSelected(i == mfxIndex);
    }

    if (dispMsg)
    {
        if (mfxIndex < NUM_MIDIFX_GROUPS)
        {
            omxDisp.displayMessageTimed("MidiFX " + String(mfxIndex + 1), 5);
        }
        else
        {
            omxDisp.displayMessageTimed("MidiFX Off", 5);
        }
    }
}

void OmxModeChords::onClockTick()
{
    for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    {
        // Lets them do things in background
        subModeMidiFx[i].onClockTick();
    }
}

void OmxModeChords::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
    if (isSubmodeEnabled() && activeSubmode->usesPots())
    {
        activeSubmode->onPotChanged(potIndex, prevValue, newValue, analogDelta);
        return;
    }

    // Serial.println("onPotChanged: " + String(potIndex));
    if (chordEditMode_ == false && mode_ == CHRDMODE_MANSTRUM)
    {
        if (analogDelta < 3)
        {
            return;
        }

        // Serial.println("strum");

        if (potIndex == 0)
        {
            uint8_t oldV = manStrumSensit_;
            manStrumSensit_ = (uint8_t)map(newValue, 0, 127, 1, 32);
            if (manStrumSensit_ != oldV)
            {
                omxDisp.displayMessageTimed("Sens: " + String(manStrumSensit_), 5);
            }
        }
        else if (potIndex == 1)
        {
            bool oldV = wrapManStrum_;
            wrapManStrum_ = (bool)map(newValue, 0, 127, 0, 1);
            if (wrapManStrum_ != oldV)
            {
                if (wrapManStrum_)
                {
                    omxDisp.displayMessageTimed("Wrap on", 5);
                }
                else
                {
                    omxDisp.displayMessageTimed("Wrap off", 5);
                }
            }
        }
        else if (potIndex == 2)
        {
            uint8_t oldV = incrementManStrum_;
            incrementManStrum_ = (uint8_t)map(newValue, 0, 127, 0, 4);
            if (incrementManStrum_ != oldV)
            {
                omxDisp.displayMessageTimed("Increm: " + String(incrementManStrum_), 5);
            }
        }
        else if (potIndex == 3)
        {
            // Serial.println("length");

            uint8_t prevLength = manStrumNoteLength_;
            manStrumNoteLength_ = map(newValue, 0, 127, 0, kNumNoteLengths - 1);

            if (prevLength != manStrumNoteLength_)
            {
                omxDisp.displayMessageTimed(String(kNoteLengths[manStrumNoteLength_]), 10);
            }
        }

        omxDisp.setDirty();
        omxLeds.setDirty();
        return;
    }
    // if (potIndex < 4)
    // {
    //     if (analogDelta >= 10)
    //     {
    //         grids_.setDensity(potIndex, newValue * 2);

    //         if (params.getSelPage() == GRIDS_DENSITY)
    //         {
    //             setParam(potIndex);
    //             // setParam(GRIDS_DENSITY, potIndex + 1);
    //         }
    //     }

    //     omxDisp.setDirty();
    // }
    // else if (potIndex == 4)
    // {
    //     int newres = (float(newValue) / 128.f) * 3;
    //     grids_.setResolution(newres);
    //     if (newres != prevResolution_)
    //     {
    //         String msg = String(rateNames[newres]);
    //         omxDisp.displayMessageTimed(msg, 5);
    //     }
    //     prevResolution_ = newres;
    // }
}

void OmxModeChords::loopUpdate(Micros elapsedTime)
{
    updateFuncKeyMode();

    if (elapsedTime > 0)
	{
		if (!sequencer.playing)
		{
            // Needed to make pendingNoteOns/pendingNoteOffs work
			omxUtil.advanceSteps(elapsedTime);
		}
	}

    for(uint8_t i = 0; i < 5; i++)
    {
        // Lets them do things in background
        subModeMidiFx[i].loopUpdate();
    }
}

void OmxModeChords::updateFuncKeyMode()
{
    auto keyState = midiSettings.keyState;

    uint8_t prevMode = funcKeyMode_;

    funcKeyMode_ = FUNCKEYMODE_NONE;

    if(keyState[1] && !keyState[2])
    {
        funcKeyMode_ = FUNCKEYMODE_F1;
    }
    else if(!keyState[1] && keyState[2])
    {
        funcKeyMode_ = FUNCKEYMODE_F2;
    }
    else if(keyState[1] && keyState[2])
    {
        funcKeyMode_ = FUNCKEYMODE_F3;
    }
    else
    {
        funcKeyMode_ = FUNCKEYMODE_NONE;
    }

    if(funcKeyMode_ != prevMode)
    {
        omxDisp.setDirty();
        omxLeds.setDirty();
    }
}

void OmxModeChords::onEncoderChanged(Encoder::Update enc)
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onEncoderChanged(enc);
        return;
    }

    if(chordEditMode_ == false && mode_ == CHRDMODE_MANSTRUM)
    {
        onEncoderChangedManStrum(enc);
        return;
    }

    if (encoderSelect_)
    {
        params_.changeParam(enc.dir());
        omxDisp.setDirty();
        return;
    }

    int8_t selPage = params_.getSelPage(); 
    int8_t selParam = params_.getSelParam() + 1; // Add one for readability

    // PAGE ONE - Root, Scale, Octave, Midi Channel
    if (selPage == CHRDPAGE_1)
    {
        auto amt = enc.accel(1); // where 5 is the acceleration factor if you want it, 0 if you don't)

        // Root, Scale, Octave, Channel
        if (selParam == 1)
        {
            int prevRoot = scaleConfig.scaleRoot;
            scaleConfig.scaleRoot = constrain(scaleConfig.scaleRoot + amt, 0, 12 - 1);
            if (prevRoot != scaleConfig.scaleRoot)
            {
                musicScale_->calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
            }
        }
        else if (selParam == 2)
        {
            int prevPat = scaleConfig.scalePattern;
            scaleConfig.scalePattern = constrain(scaleConfig.scalePattern + amt, -1, musicScale_->getNumScales() - 1);
            if (prevPat != scaleConfig.scalePattern)
            {
                omxDisp.displayMessage(musicScale_->getScaleName(scaleConfig.scalePattern));
                musicScale_->calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
            }
        }
        else if (selParam == 3)
        {
            // set octave
            midiSettings.newoctave = constrain(midiSettings.octave + amt, -5, 4);
            if (midiSettings.newoctave != midiSettings.octave)
            {
                midiSettings.octave = midiSettings.newoctave;
            }
        }
        else if (selParam == 4)
        {
            int newchan = constrain(sysSettings.midiChannel + amt, 1, 16);
            if (newchan != sysSettings.midiChannel)
            {
                sysSettings.midiChannel = newchan;
            }
        }
    }
    // PAGE TWO - numNotes, degree, octave, transpose
    else if (selPage == CHRDPAGE_2)
    {
        auto amt = enc.accel(1); // where 5 is the acceleration factor if you want it, 0 if you don't)

        if (selParam == 1)
        {
            chords_[selectedChord_].numNotes = constrain(chords_[selectedChord_].numNotes + amt, 1, 4);
        }
        else if (selParam == 2)
        {
            chords_[selectedChord_].degree = constrain(chords_[selectedChord_].degree + amt, 0, 7);
        }
        else if (selParam == 3)
        {
            chords_[selectedChord_].octave = constrain(chords_[selectedChord_].octave + amt, -2, 2);
        }
        else if (selParam == 4)
        {
            chords_[selectedChord_].transpose = constrain(chords_[selectedChord_].transpose + amt, -7, 7);
        }
    }
    // PAGE THREE - spread, rotate, voicing
    else if (selPage == CHRDPAGE_3)
    {
        auto amt = enc.accel(1); // where 5 is the acceleration factor if you want it, 0 if you don't)

        if (selParam == 1)
        {
            chords_[selectedChord_].spread = constrain(chords_[selectedChord_].spread + amt, -2, 2);
        }
        else if (selParam == 2)
        {
            chords_[selectedChord_].rotate = constrain(chords_[selectedChord_].rotate + amt, 0, 4);
        }
        else if (selParam == 3)
        {
            chords_[selectedChord_].voicing = constrain(chords_[selectedChord_].voicing + amt, 0, 7);
        }
    }
    // PAGE FOUR - spreadUpDown, quartalVoicing
    else if (selPage == CHRDPAGE_4)
    {
        auto amt = enc.accel(1); // where 5 is the acceleration factor if you want it, 0 if you don't)

        if (selParam == 1)
        {
            chords_[selectedChord_].spreadUpDown = constrain(chords_[selectedChord_].spreadUpDown + amt, 0, 1);
        }
        else if (selParam == 2)
        {
            chords_[selectedChord_].quartalVoicing = constrain(chords_[selectedChord_].quartalVoicing + amt, 0, 1);
        }
    }
    
    omxDisp.setDirty();
}

void OmxModeChords::onEncoderChangedManStrum(Encoder::Update enc)
{
    if(chordNotes_[selectedChord_].active == false) return;

    auto amt = enc.accel(1);

    // Serial.println("EncDelta: " + String(chordNotes_[selectedChord_].encDelta));

    chordNotes_[selectedChord_].encDelta = chordNotes_[selectedChord_].encDelta + amt;

    if(abs(chordNotes_[selectedChord_].encDelta) >= manStrumSensit_)
    {

        uint8_t numNotes = 0;

        for(uint8_t i = 0; i < 6; i++)
        {
            if(chordNotes_[selectedChord_].notes[i] >= 0)
            {
                numNotes++;
            }
        }

        // Serial.println("Do Note");
        uint8_t velocity = midiSettings.defaultVelocity;

        int8_t strumPos = chordNotes_[selectedChord_].strumPos;

        // Serial.println("strumPos: " + String(strumPos));

        if (strumPos >= 0 && strumPos < numNotes)
        {
            int note = chordNotes_[selectedChord_].notes[strumPos] + (chordNotes_[selectedChord_].octIncrement * 12);

            if (note >= 0 && note <= 127)
            {
                uint32_t noteOnMicros = micros();
                float noteLength = kNoteLengths[manStrumNoteLength_];
                uint32_t noteOffMicros = noteOnMicros + (noteLength * clockConfig.step_micros);

                pendingNoteOns.insert(note, velocity, chordNotes_[selectedChord_].channel, noteOnMicros, false);
                pendingNoteOffs.insert(note, chordNotes_[selectedChord_].channel, noteOffMicros, false);

                omxDisp.displayMessage(musicScale_->getFullNoteName(note));
                omxDisp.setDirty();
                omxLeds.setDirty();
            }
        }

        if(chordNotes_[selectedChord_].encDelta > 0)
        {
            strumPos++;
        }
        else
        {
            strumPos--;
        }

        if(wrapManStrum_)
        {
            if (strumPos >= numNotes)
            {
                chordNotes_[selectedChord_].octIncrement++;
                if (chordNotes_[selectedChord_].octIncrement > incrementManStrum_)
                {
                    chordNotes_[selectedChord_].octIncrement = 0;
                }
                strumPos = 0;
            }
            if (strumPos < 0)
            {

                chordNotes_[selectedChord_].octIncrement--;
                if (chordNotes_[selectedChord_].octIncrement < -incrementManStrum_)
                {
                    chordNotes_[selectedChord_].octIncrement = 0;
                }
                strumPos = numNotes - 1;
            }

            chordNotes_[selectedChord_].strumPos = strumPos;
        }
        else
        {
            chordNotes_[selectedChord_].strumPos = constrain(strumPos, -1, 6); // Allow to be one outside of notes
        }

        // chordNotes_[selectedChord_].octIncrement = constrain(chordNotes_[selectedChord_].octIncrement, -8, 8);

        chordNotes_[selectedChord_].encDelta = 0;
    }
}

void OmxModeChords::onEncoderButtonDown()
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onEncoderButtonDown();
        return;
    }

    encoderSelect_ = !encoderSelect_;
    omxDisp.setDirty();
}

void OmxModeChords::onEncoderButtonDownLong()
{
}

bool OmxModeChords::shouldBlockEncEdit()
{
    if (isSubmodeEnabled())
    {
        return activeSubmode->shouldBlockEncEdit();
    }

    return false;
}

void OmxModeChords::onKeyUpdate(OMXKeypadEvent e)
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onKeyUpdate(e);
        return;
    }

    if(chordEditMode_)
    {
        onKeyUpdateChordEdit(e);
        return;
    }

    if(onKeyUpdateSelMidiFX(e)) return;

    if(e.held()) return;

    uint8_t thisKey = e.key();
	// auto keyState = midiSettings.keyState;

    // AUX KEY
    if(thisKey == 0)
    {
        if(e.down())
        {
            auxDown_ = true;
        }
        else
        {
            auxDown_ = false;
        }
        omxLeds.setDirty();
        omxDisp.setDirty();
        return;
    }

    if (auxDown_) // Aux mode
    {
        if (e.down())
        {
            if (thisKey == 11 || thisKey == 12) // Change Octave
            {
                int amt = thisKey == 11 ? -1 : 1;
                midiSettings.newoctave = constrain(midiSettings.octave + amt, -5, 4);
                if (midiSettings.newoctave != midiSettings.octave)
                {
                    midiSettings.octave = midiSettings.newoctave;
                }
            }
            else if (thisKey == 1 || thisKey == 2) // Change Param selection
            {
                if (thisKey == 1)
                {
                    params_.decrementParam();
                }
                else if (thisKey == 2)
                {
                    params_.incrementParam();
                }
            }
            // else if (thisKey == 5)
            // {
            //     // Turn off midiFx
            //     mfxIndex = 127;
            // }
            // else if (thisKey >= 6 && thisKey < 11)
            // {
            //     // Change active midiFx
            //     mfxIndex = thisKey - 6;
            //     // enableSubmode(&subModeMidiFx[thisKey - 6]);
            // }
        }
    }
    else
    {
        if (funcKeyMode_ == FUNCKEYMODE_NONE)
        {
            if (e.down())
            {
                if(thisKey == 3)
                {
                    mode_ = CHRDMODE_PLAY;
                    params_.setSelPageAndParam(CHRDPAGE_NOTES, 0);
                    omxDisp.displayMessage("Play");
                }
                else if(thisKey == 4)
                {
                    mode_ = CHRDMODE_EDIT;
                    params_.setSelPageAndParam(CHRDPAGE_NOTES, 0);
                    omxDisp.displayMessage("Edit");
                }
                else if(thisKey == 5)
                {
                    mode_ = CHRDMODE_PRESET;
                    omxDisp.displayMessage("Preset");
                }
                else if(thisKey == 6)
                {
                    mode_ = CHRDMODE_MANSTRUM;
                    omxDisp.displayMessage("Manual Strum");
                }
                if (thisKey >= 11)
                {
                    if(mode_ == CHRDMODE_PLAY) // Play
                    {
                        selectedChord_ = thisKey - 11;
                        onChordOn(thisKey - 11);
                    }
                    else if(mode_ == CHRDMODE_EDIT) // Edit
                    {
                        // Enter chord edit mode
                        selectedChord_ = thisKey - 11;
                        enterChordEditMode();
                        return;
                    }
                    else if(mode_ == CHRDMODE_PRESET) // Preset
                    {
                        if(loadPreset(thisKey - 11))
                        {
                            omxDisp.displayMessageTimed("Load " + String(thisKey - 11), 5);
                        }
                    }
                    else if(mode_ == CHRDMODE_MANSTRUM) // Manual Strum
                    {
                        // Enter chord edit mode
                        selectedChord_ = thisKey - 11;
                        onManualStrumOn(selectedChord_);
                        return;
                    }
                }
            }
            else
            {
                if (thisKey >= 11)
                {
                    onChordOff(thisKey - 11);
                }
            }
        }
        else // Function key held
        {
            if (e.down() && thisKey >= 11)
            {
                if (mode_ == CHRDMODE_PLAY) // Play
                {
                    if (funcKeyMode_ == FUNCKEYMODE_F1)
                    {
                        selectedChord_ = thisKey - 11;
                        enterChordEditMode();
                        return;
                    }
                    else if (funcKeyMode_ == FUNCKEYMODE_F2)
                    {
                        if (pasteSelectedChordTo(thisKey - 11))
                        {
                            omxDisp.displayMessageTimed("Copied to " + String(thisKey - 11), 5);
                        }
                    }
                }
                else if (mode_ == CHRDMODE_EDIT) // Edit
                {
                    if (funcKeyMode_ == FUNCKEYMODE_F1)
                    {
                        selectedChord_ = thisKey - 11;
                        enterChordEditMode();
                        return;
                    }
                    else if (funcKeyMode_ == FUNCKEYMODE_F2)
                    {
                        if (pasteSelectedChordTo(thisKey - 11))
                        {
                            omxDisp.displayMessageTimed("Copied to " + String(thisKey - 11), 5);
                        }
                    }
                }
                else if (mode_ == CHRDMODE_PRESET) // Preset
                {
                    if (funcKeyMode_ == FUNCKEYMODE_F1)
                    {
                    }
                    else if (funcKeyMode_ == FUNCKEYMODE_F2)
                    {
                        if (savePreset(thisKey - 11))
                        {
                            omxDisp.displayMessageTimed("Saved to " + String(thisKey - 11), 5);
                        }
                    }
                }
                else if (mode_ == CHRDMODE_MANSTRUM) // Manual Strum
                {
                    if (funcKeyMode_ == FUNCKEYMODE_F1)
                    {
                        selectedChord_ = thisKey - 11;
                        enterChordEditMode();
                        return;
                    }
                    else if (funcKeyMode_ == FUNCKEYMODE_F2)
                    {
                        if (pasteSelectedChordTo(thisKey - 11))
                        {
                            omxDisp.displayMessageTimed("Copied to " + String(thisKey - 11), 5);
                        }
                    }
                }
            }
        }
    }

    omxLeds.setDirty();
    omxDisp.setDirty();
}

void OmxModeChords::onKeyUpdateChordEdit(OMXKeypadEvent e)
{
    if(e.held()) return;

    uint8_t thisKey = e.key();

    // AUX KEY
    if(thisKey == 0)
    {
        if(e.down())
        {
            // Exit Chord Edit Mode
            onChordEditOff();
            params_.setSelPageAndParam(CHRDPAGE_NOTES, 0);
            encoderSelect_ = true;
            chordEditMode_ = false;
        }

        omxLeds.setDirty();
        omxDisp.setDirty();
        return;
    }

    if (e.down())
    {
        if (chordEditParam_ == 0)
        {
            if (thisKey == 1) // Select Root
            {
                params_.setSelPageAndParam(CHRDPAGE_1, 0);
                encoderSelect_ = false;
            }
            if (thisKey == 2) // Select Scale
            {
                params_.setSelPageAndParam(CHRDPAGE_1, 1);
                encoderSelect_ = false;
            }
            if (thisKey == 3) // Octave
            {
                chordEditParam_ = 1;
                params_.setSelPageAndParam(CHRDPAGE_2, 2);
                encoderSelect_ = false;
            }
            else if (thisKey == 4) // Transpose
            {
                chordEditParam_ = 2;
                params_.setSelPageAndParam(CHRDPAGE_2, 3);
                encoderSelect_ = false;
            }
            else if (thisKey == 5) // Spread
            {
                chordEditParam_ = 3;
                params_.setSelPageAndParam(CHRDPAGE_3, 0);
                encoderSelect_ = false;
            }
            else if (thisKey == 6) // Rotate
            {
                chordEditParam_ = 4;
                params_.setSelPageAndParam(CHRDPAGE_3, 1);
                encoderSelect_ = false;
            }
            else if (thisKey == 7) // Voicing
            {
                chordEditParam_ = 5;
                params_.setSelPageAndParam(CHRDPAGE_3, 2);
                encoderSelect_ = false;
            }
            else if (thisKey == 10) // Show Chord Notes
            {
                params_.setSelPageAndParam(CHRDPAGE_NOTES, 0);
                encoderSelect_ = true;
            }
            else if (thisKey >= 11 && thisKey < 15)
            {
                chords_[selectedChord_].numNotes = (thisKey - 11) + 1;
                params_.setSelPageAndParam(CHRDPAGE_2, 0);
                encoderSelect_ = false;
            }
            else if (thisKey == 15)
            {
                chords_[selectedChord_].spreadUpDown = !chords_[selectedChord_].spreadUpDown;
                params_.setSelPageAndParam(CHRDPAGE_4, 0);
                encoderSelect_ = false;
                omxDisp.displayMessage(chords_[selectedChord_].spreadUpDown ? "SpdUpDn On" : "SpdUpDn Off");
            }
            else if (thisKey == 16)
            {
                chords_[selectedChord_].quartalVoicing = !chords_[selectedChord_].quartalVoicing;
                params_.setSelPageAndParam(CHRDPAGE_4, 1);
                encoderSelect_ = false;
                omxDisp.displayMessage(chords_[selectedChord_].quartalVoicing ? "Quartal On" : "Quartal Off");
            }
            else if (thisKey >= 19)
            {
                chords_[selectedChord_].degree = thisKey - 19;
                // params_.setSelPageAndParam(CHRDPAGE_2, 1);
                // encoderSelect_ = false;
                onChordEditOff();
                onChordEditOn(selectedChord_);
                activeChordEditDegree_ = thisKey - 19;
            }
        }
        else if(chordEditParam_ == 1) // Octave
        {
            // chords_[selectedChord_].octave = constrain(chords_[selectedChord_].octave + amt, -2, 2);
            if(thisKey >= 11 && thisKey <= 15)
            {
                chords_[selectedChord_].octave = thisKey - 11 - 2;
            }
        }
        else if(chordEditParam_ == 2) // Transpose
        {
            // chords_[selectedChord_].transpose = constrain(chords_[selectedChord_].transpose + amt, -7, 7);
            if(thisKey >= 11 && thisKey <= 25)
            {
                chords_[selectedChord_].transpose = thisKey - 11 - 7;
            }
        }
        else if(chordEditParam_ == 3) // Spread
        {
            // chords_[selectedChord_].spread = constrain(chords_[selectedChord_].spread + amt, -2, 2);
            if(thisKey >= 11 && thisKey <= 15)
            {
                chords_[selectedChord_].spread = thisKey - 11 - 2;
            }
        }
        else if(chordEditParam_ == 4) // Rotate
        {
            // chords_[selectedChord_].rotate = constrain(chords_[selectedChord_].rotate + amt, 0, 4);
            if(thisKey >= 11 && thisKey <= 15)
            {
                chords_[selectedChord_].rotate = thisKey - 11;
            }

        }
        else if(chordEditParam_ == 5) // Voicing
        {
            // chords_[selectedChord_].voicing = constrain(chords_[selectedChord_].octave + amt, 0, 7);
            if(thisKey >= 11 && thisKey <= 18)
            {
                chords_[selectedChord_].voicing = thisKey - 11;
            }
        }
    }
    else
    {
        if (thisKey >= 3 && thisKey <= 7)
        {
            chordEditParam_ = 0;
        }
        else if (thisKey >= 19)
        {
            if(thisKey - 19 == activeChordEditDegree_)
            {
                onChordEditOff();
            }
        }
    }

    omxLeds.setDirty();
    omxDisp.setDirty();
}

void OmxModeChords::enterChordEditMode()
{
    chordEditMode_ = true;
    chordEditParam_ = 0;
    omxLeds.setDirty();
    omxDisp.setDirty();
}

void OmxModeChords::onKeyHeldUpdate(OMXKeypadEvent e)
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onKeyHeldUpdate(e);
        return;
    }

    if(onKeyHeldSelMidiFX(e)) return;
}

void OmxModeChords::enableSubmode(SubmodeInterface *subMode)
{
    if(activeSubmode != nullptr)
    {
        activeSubmode->setEnabled(false);
    }

    activeSubmode = subMode;
    activeSubmode->setEnabled(true);
    omxDisp.setDirty();
}
void OmxModeChords::disableSubmode()
{
    if(activeSubmode != nullptr)
    {
        activeSubmode->setEnabled(false);
    }

    activeSubmode = nullptr;
    omxDisp.setDirty();
}

bool OmxModeChords::isSubmodeEnabled()
{
    if(activeSubmode == nullptr) return false;

    if(activeSubmode->isEnabled() == false){
        disableSubmode();
        return false;
    }

    return true;
}

bool OmxModeChords::onKeyUpdateSelMidiFX(OMXKeypadEvent e)
{
    int thisKey = e.key();

    bool keyConsumed = false;

    if (!e.held())
    { 
        if (e.down() && thisKey != 0)
        {
            if (auxDown_) // Aux mode
            {
                if (thisKey == 5)
                {
                    keyConsumed = true;
                    // Turn off midiFx
                    selectMidiFx(127, true);
                    // mfxIndex_ = 127;
                }
                else if (thisKey >= 6 && thisKey < 11)
                {
                    keyConsumed = true;
                    selectMidiFx(thisKey - 6, true);
                    // Change active midiFx
                    // mfxIndex_ = thisKey - 6;
                }
                else if (thisKey == 25)
                {
                    if (mfxIndex_ < NUM_MIDIFX_GROUPS)
                    {
                        subModeMidiFx[mfxIndex_].toggleArpHold();

                        if (subModeMidiFx[mfxIndex_].isArpHoldOn())
                        {
                            omxDisp.displayMessageTimed("Arp Hold: On", 5);
                        }
                        else
                        {
                            omxDisp.displayMessageTimed("Arp Hold: Off", 5);
                        }
                    }
                    else
                    {
                        omxDisp.displayMessageTimed("MidiFX are Off", 5);
                    }
                }
                else if (thisKey == 26)
                {
                    if (mfxIndex_ < NUM_MIDIFX_GROUPS)
                    {
                        subModeMidiFx[mfxIndex_].toggleArp();

                        if (subModeMidiFx[mfxIndex_].isArpOn())
                        {
                            omxDisp.displayMessageTimed("Arp On", 5);
                        }
                        else
                        {
                            omxDisp.displayMessageTimed("Arp Off", 5);
                        }
                    }
                    else
                    {
                        omxDisp.displayMessageTimed("MidiFX are Off", 5);
                    }
                }
            }
        }
    }

    return keyConsumed;
}

bool OmxModeChords::onKeyHeldSelMidiFX(OMXKeypadEvent e)
{
    int thisKey = e.key();

    bool keyConsumed = false;

    if (auxDown_) // Aux mode
    {
        // Enter MidiFX mode
        if (thisKey >= 6 && thisKey < 11)
        {
            keyConsumed = true;
            enableSubmode(&subModeMidiFx[thisKey - 6]);
        }
    }

    return keyConsumed;
}

void OmxModeChords::doNoteOn(int noteNumber, uint8_t velocity, uint8_t midiChannel)
{
    if(noteNumber < 0 || noteNumber > 127) return;

    // MidiNoteGroup noteGroup = omxUtil.midiNoteOn2(musicScale, keyIndex, midiSettings.defaultVelocity, sysSettings.midiChannel);
    // if(noteGroup.noteNumber == 255) return;

    MidiNoteGroup noteGroup;

    noteGroup.noteOff = false;
    noteGroup.noteNumber = noteNumber;
    noteGroup.prevNoteNumber = noteNumber;
    noteGroup.velocity = velocity;
    noteGroup.channel = midiChannel;
    noteGroup.unknownLength = true;
    noteGroup.stepLength = 0;
    noteGroup.sendMidi = true;
    noteGroup.sendCV = false;
    noteGroup.noteonMicros = micros();

    // Serial.println("doNoteOn: " + String(noteGroup.noteNumber));

    if (mfxIndex_ < NUM_MIDIFX_GROUPS)
    {
        subModeMidiFx[mfxIndex_].noteInput(noteGroup);
        // subModeMidiFx.noteInput(noteGroup);
    }
    else
    {
        onNotePostFX(noteGroup);
    }
}

void OmxModeChords::doNoteOff(int noteNumber, uint8_t midiChannel)
{
    if(noteNumber < 0 || noteNumber > 127) return;

    // MidiNoteGroup noteGroup = omxUtil.midiNoteOff2(keyIndex, sysSettings.midiChannel);

    // if(noteGroup.noteNumber == 255) return;

    MidiNoteGroup noteGroup;

    noteGroup.noteOff = true;
    noteGroup.noteNumber = noteNumber;
    noteGroup.prevNoteNumber = noteNumber;
    noteGroup.velocity = 0;
    noteGroup.channel = midiChannel;
    noteGroup.unknownLength = true;
    noteGroup.stepLength = 0;
    noteGroup.sendMidi = true;
    noteGroup.sendCV = false;
    noteGroup.noteonMicros = micros();

    // Serial.println("doNoteOff: " + String(noteGroup.noteNumber));

    noteGroup.unknownLength = true;
    noteGroup.prevNoteNumber = noteGroup.noteNumber;

    if (mfxIndex_ < NUM_MIDIFX_GROUPS)
    {
        subModeMidiFx[mfxIndex_].noteInput(noteGroup);
    // subModeMidiFx.noteInput(noteGroup);
    }
    else
    {
        onNotePostFX(noteGroup);
    }
}

void OmxModeChords::onNotePostFX(MidiNoteGroup note)
{
    if(note.noteOff)
    {
        // Serial.println("OmxModeMidiKeyboard::onNotePostFX noteOff: " + String(note.noteNumber));

        if (note.sendMidi)
        {
            MM::sendNoteOff(note.noteNumber, note.velocity, note.channel);
        }
        if (note.sendCV)
        {
            omxUtil.cvNoteOff();
        }
    }
    else
    {
        if (note.unknownLength == false)
        {
            uint32_t noteOnMicros = note.noteonMicros; // TODO Might need to be set to current micros
            pendingNoteOns.insert(note.noteNumber, note.velocity, note.channel, noteOnMicros, note.sendCV);

            // Serial.println("StepLength: " + String(note.stepLength));

            uint32_t noteOffMicros = noteOnMicros + (note.stepLength * clockConfig.step_micros);
            pendingNoteOffs.insert(note.noteNumber, note.channel, noteOffMicros, note.sendCV);

            // Serial.println("noteOnMicros: " + String(noteOnMicros));
            // Serial.println("noteOffMicros: " + String(noteOffMicros));
        }
        else
        {
            // Serial.println("OmxModeMidiKeyboard::onNotePostFX noteOn: " + String(note.noteNumber));

            if (note.sendMidi)
            {
                MM::sendNoteOn(note.noteNumber, note.velocity, note.channel);
            }
            if (note.sendCV)
            {
                omxUtil.cvNoteOn(note.noteNumber);
            }
        }
    }
}

void OmxModeChords::onPendingNoteOff(int note, int channel)
{
    // Serial.println("OmxModeEuclidean::onPendingNoteOff " + String(note) + " " + String(channel));
    // subModeMidiFx.onPendingNoteOff(note, channel);

    for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    {
        subModeMidiFx[i].onPendingNoteOff(note, channel);
    }
}

void OmxModeChords::updateLEDs()
{
    if(chordEditMode_)
    {
        updateLEDsChordEdit();
        return;
    }

    bool blinkState = omxLeds.getBlinkState();

    omxLeds.setAllLEDS(0,0,0);

    if(auxDown_)
    {
        // Blink left/right keys for octave select indicators.
		strip.setPixelColor(0, RED);
		strip.setPixelColor(1, (blinkState ? LIME : LEDOFF));
		strip.setPixelColor(2, (blinkState ? MAGENTA : LEDOFF));
		strip.setPixelColor(11, (blinkState ? ORANGE : LEDOFF));
		strip.setPixelColor(12, (blinkState ? RBLUE : LEDOFF));

        // MidiFX off
        strip.setPixelColor(5, (mfxIndex_ >= NUM_MIDIFX_GROUPS ? colorConfig.selMidiFXOffColor : colorConfig.midiFXOffColor));

        for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
        {
            auto mfxColor = (i == mfxIndex_) ? colorConfig.selMidiFXColor : colorConfig.midiFXColor;

            strip.setPixelColor(6 + i, mfxColor);
        }

        if(mfxIndex_ < NUM_MIDIFX_GROUPS)
        {
            bool isOn = subModeMidiFx[mfxIndex_].isArpOn() && blinkState;
            bool isHoldOn = subModeMidiFx[mfxIndex_].isArpHoldOn();

            strip.setPixelColor(25, isHoldOn ? colorConfig.arpHoldOn : colorConfig.arpHoldOff);
            strip.setPixelColor(26, isOn ? colorConfig.arpOn : colorConfig.arpOff);
        }
        else
        {
            strip.setPixelColor(25, colorConfig.arpHoldOff);
            strip.setPixelColor(26, colorConfig.arpOff);
        }

        return;
    }

    // Function Keys
    if (funcKeyMode_ == FUNCKEYMODE_F3)
    {
        auto f3Color = blinkState ? LEDOFF : FUNKTHREE;
        strip.setPixelColor(1, f3Color);
        strip.setPixelColor(2, f3Color);
    }
    else
    {
        auto f1Color = (funcKeyMode_ == FUNCKEYMODE_F1 && blinkState) ? LEDOFF : FUNKONE;
        strip.setPixelColor(1, f1Color);

        auto f2Color = (funcKeyMode_ == FUNCKEYMODE_F2 && blinkState) ? LEDOFF : FUNKTWO;
        strip.setPixelColor(2, f2Color);
    }

    strip.setPixelColor(3, mode_ == CHRDMODE_PLAY ? WHITE : kPlayColor);
    strip.setPixelColor(4, mode_ == CHRDMODE_EDIT ? WHITE : kEditColor);
    strip.setPixelColor(5, mode_ == CHRDMODE_PRESET ? WHITE : kPresetColor);
    strip.setPixelColor(6, mode_ == CHRDMODE_MANSTRUM ? WHITE : MAGENTA);

    if (mode_ == CHRDMODE_PLAY || mode_ == CHRDMODE_MANSTRUM) // Play
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            if (i == selectedChord_)
            {
                strip.setPixelColor(11 + i, (chordNotes_[i].active ? WHITE : CYAN));
            }
            else
            {
                strip.setPixelColor(11 + i, (chordNotes_[i].active ? WHITE : chords_[i].color));
            }
        }
    }
    else if (mode_ == CHRDMODE_EDIT) // Edit
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            if (i == selectedChord_)
            {
                strip.setPixelColor(11 + i, (chordNotes_[i].active ? WHITE : CYAN));
            }
            else
            {
                strip.setPixelColor(11 + i, (chordNotes_[i].active ? WHITE : kEditColor));
            }
        }
    }
    else if (mode_ == CHRDMODE_PRESET) // Preset
    {
        for (uint8_t i = 0; i < NUM_CHORD_SAVES; i++)
        {
            strip.setPixelColor(11 + i, (i == selectedSave_ ? WHITE : kPresetColor));
        }
    }
}

void OmxModeChords::updateLEDsChordEdit()
{
    bool blinkState = omxLeds.getBlinkState();

    omxLeds.setAllLEDS(0,0,0);

    strip.setPixelColor(0, RED); // EXIT
    
    // Function Keys
    if (funcKeyMode_ == FUNCKEYMODE_F3)
    {
        auto f3Color = blinkState ? LEDOFF : FUNKTHREE;
        strip.setPixelColor(1, f3Color);
        strip.setPixelColor(2, f3Color);
    }
    else
    {
        auto f1Color = (funcKeyMode_ == FUNCKEYMODE_F1 && blinkState) ? LEDOFF : FUNKONE;
        strip.setPixelColor(1, f1Color);

        auto f2Color = (funcKeyMode_ == FUNCKEYMODE_F2 && blinkState) ? LEDOFF : FUNKTWO;
        strip.setPixelColor(2, f2Color);
    }

    strip.setPixelColor(3, kOctaveColor);    // Octave
    strip.setPixelColor(4, kTransposeColor); // Transpose
    strip.setPixelColor(5, kSpreadColor);    // Spread
    strip.setPixelColor(6, kRotateColor);    // Rotate
    strip.setPixelColor(7, kVoicingColor);   // Voicing
    strip.setPixelColor(10, ROSE);   // Show Chord Notes

    if (chordEditParam_ == 0)
    {
        // Num Notes
        for(uint8_t i = 11; i < 15; i++)
        {
            auto numNotesColor = chords_[selectedChord_].numNotes == (i - 11) + 1 ? kNumNotesSelColor : kNumNotesColor;
            strip.setPixelColor(i, numNotesColor);
        }

        strip.setPixelColor(15, chords_[selectedChord_].spreadUpDown ? kSpreadUpDownOnColor : kSpreadUpDownOffColor);
        strip.setPixelColor(16, chords_[selectedChord_].quartalVoicing ? kQuartalVoicingOnColor : kQuartalVoicingOffColor);

        // Degree
        for(uint8_t i = 19; i < 27; i++)
        {
            strip.setPixelColor(i, chords_[selectedChord_].degree == i - 19 ? kDegreeSelColor : kDegreeColor);
        }
    }
    else if (chordEditParam_ == 1) // Octave
    {
        strip.setPixelColor(3, blinkState ? LEDOFF : kOctaveColor);

        for(uint8_t i = 11; i < 16; i++)
        {
            auto valColor = chords_[selectedChord_].octave == (i - 11 - 2) ? WHITE : GREEN;
            strip.setPixelColor(i, valColor);
        }
    }
    else if (chordEditParam_ == 2) // Transpose
    {
        strip.setPixelColor(4, blinkState ? LEDOFF : kTransposeColor);

        for(uint8_t i = 11; i < 26; i++)
        {
            auto valColor = chords_[selectedChord_].transpose == (i - 11 - 7) ? WHITE : GREEN;
            strip.setPixelColor(i, valColor);
        }
    }
    else if (chordEditParam_ == 3) // Spread
    {
        strip.setPixelColor(5, blinkState ? LEDOFF : kSpreadColor);

        for(uint8_t i = 11; i < 16; i++)
        {
            auto valColor = chords_[selectedChord_].spread == (i - 11 - 2) ? WHITE : GREEN;
            strip.setPixelColor(i, valColor);
        }
    }
    else if (chordEditParam_ == 4) // Rotate
    {
        strip.setPixelColor(6, blinkState ? LEDOFF : kRotateColor);

        for(uint8_t i = 11; i < 16; i++)
        {
            auto valColor = chords_[selectedChord_].rotate == (i - 11) ? WHITE : GREEN;
            strip.setPixelColor(i, valColor);
        }
    }
    else if (chordEditParam_ == 5) // Voicing
    {
        strip.setPixelColor(7, blinkState ? LEDOFF : kVoicingColor);

        for(uint8_t i = 11; i < 19; i++)
        {
            auto valColor = chords_[selectedChord_].voicing == (i - 11) ? WHITE : GREEN;
            strip.setPixelColor(i, valColor);
        }
    }
}

void OmxModeChords::setupPageLegends()
{
    omxDisp.clearLegends();

    int8_t page = params_.getSelPage();

    switch (page)
    {
    case CHRDPAGE_1:
    {
        // Root, Scale, Octave, Midi Channel
        omxDisp.legends[0] = "ROOT";
        omxDisp.legendVals[0] = -127;
        omxDisp.legendText[0] = musicScale_->getNoteName(scaleConfig.scaleRoot);
        omxDisp.legends[1] = "SCALE";
        
        if (scaleConfig.scalePattern < 0)
        {
            omxDisp.legendVals[1] = -127;
            omxDisp.legendText[1] = "CHRM";
        }
        else
        {
            omxDisp.legendVals[1] = scaleConfig.scalePattern;
        }

        omxDisp.legends[2] = "OCT";
        omxDisp.legendVals[2] = (int)midiSettings.octave + 4;;
        omxDisp.legends[3] = "CH";
        omxDisp.legendVals[3] = sysSettings.midiChannel;
    }
    break;
    case CHRDPAGE_2: 
    {
        // numNotes, degree, octave, transpose
        omxDisp.legends[0] = "#NTS";
        omxDisp.legends[1] = "DEG";
        omxDisp.legends[2] = "OCT";
        omxDisp.legends[3] = "TPS";
        if(selectedChord_ < 0)
        {
            omxDisp.legendText[0] = "-";
            omxDisp.legendText[1] = "-";
            omxDisp.legendText[2] = "-";
            omxDisp.legendText[3] = "-";
        }
        else
        {
            omxDisp.legendVals[0] = chords_[selectedChord_].numNotes;
            omxDisp.legendVals[1] = chords_[selectedChord_].degree;
            omxDisp.legendVals[2] = chords_[selectedChord_].octave;
            omxDisp.legendVals[3] = chords_[selectedChord_].transpose;
        }
    }
    break;
    case CHRDPAGE_3: 
    {
        // spread, rotate, voicing
        omxDisp.legends[0] = "SPRD";
        omxDisp.legends[1] = "ROT";
        omxDisp.legends[2] = "VOIC";
        omxDisp.legends[3] = "";
        if(selectedChord_ < 0)
        {
            omxDisp.legendText[0] = "-";
            omxDisp.legendText[1] = "-";
            omxDisp.legendText[2] = "-";
            omxDisp.legendText[3] = "-";
        }
        else
        {
            omxDisp.legendVals[0] = chords_[selectedChord_].spread;
            omxDisp.legendVals[1] = chords_[selectedChord_].rotate;
            omxDisp.legendText[2] = kVoicingNames[chords_[selectedChord_].voicing];
            omxDisp.legendText[3] = "-";
        }
    }
    break;
    case CHRDPAGE_4: 
    {
        // spreadUpDown, quartalVoicing
        omxDisp.legends[0] = "UPDN";
        omxDisp.legends[1] = "QRTV";
        omxDisp.legends[2] = "";
        omxDisp.legends[3] = "";
        if(selectedChord_ < 0)
        {
            omxDisp.legendText[0] = "-";
            omxDisp.legendText[1] = "-";
            omxDisp.legendText[2] = "-";
            omxDisp.legendText[3] = "-";
        }
        else
        {
            omxDisp.legendText[0] = chords_[selectedChord_].spreadUpDown ? "ON" : "OFF";
            omxDisp.legendText[1] = chords_[selectedChord_].quartalVoicing ? "ON" : "OFF";
            omxDisp.legendText[2] = "-";
            omxDisp.legendText[3] = "-";
        }
    }
    break;
    default:
        break;
    }
}

void OmxModeChords::onDisplayUpdate()
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onDisplayUpdate();
        return;
    }

    omxLeds.updateBlinkStates();

    if (omxLeds.isDirty())
    {
        updateLEDs();
    }

    if (omxDisp.isDirty())
    { 
        if (!encoderConfig.enc_edit)
        {
            if(chordEditMode_ == false && (mode_ == CHRDMODE_PLAY || mode_ == CHRDMODE_EDIT || mode_ == CHRDMODE_MANSTRUM) && funcKeyMode_ == FUNCKEYMODE_F2) // Play mode copy
            {
                omxDisp.dispGenericModeLabel("Copy to", params_.getNumPages(), params_.getSelPage());
            }
            else if(chordEditMode_ == false && (mode_ == CHRDMODE_PRESET) && funcKeyMode_ == FUNCKEYMODE_F2) // Preset move save
            {
                omxDisp.dispGenericModeLabel("Save to", params_.getNumPages(), params_.getSelPage());
            }
            else if(chordEditMode_ == false && mode_ == CHRDMODE_MANSTRUM)
            {
                omxDisp.dispGenericModeLabel("Enc Strum", params_.getNumPages(), 0);
            }
            else if(params_.getSelPage() == CHRDPAGE_NOTES)
            {
                if(chordNotes_[selectedChord_].active)
                {
                    notesString = "";
                    notesString2 = "";

                    for(uint8_t i = 0; i < 6; i++)
                    {
                        int8_t note = chordNotes_[selectedChord_].notes[i];

                        if(note >= 0 && note <= 127)
                        {
                            if(i < 4)
                            {
                                if (i > 0)
                                {
                                    notesString.append(" ");
                                }
                                notesString.append(musicScale_->getFullNoteName(note));
                            }
                            else
                            {
                                if (i > 4)
                                {
                                    notesString2.append(" ");
                                }
                                notesString2.append(musicScale_->getFullNoteName(note));

                            }
                        }
                    }

                    omxDisp.dispGenericModeLabelDoubleLine(notesString.c_str(), notesString2.c_str(), params_.getNumPages(), params_.getSelPage());
                }
                else{
                    omxDisp.dispGenericModeLabel("-", params_.getNumPages(), params_.getSelPage());
                }
            }
            else
            {
                setupPageLegends();
                omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
            }
        }
    }
}

void OmxModeChords::SetScale(MusicScales *scale)
{
    musicScale_ = scale;
}

bool OmxModeChords::pasteSelectedChordTo(uint8_t chordIndex)
{
    if(chordIndex == selectedChord_ || chordIndex >= 16) return false;

    chords_[chordIndex].CopySettingsFrom(chords_[selectedChord_]);
    selectedChord_ = chordIndex;
    return true;
}
bool OmxModeChords::loadPreset(uint8_t presetIndex)
{
    if(presetIndex >= NUM_CHORD_SAVES) return false;

    for(uint8_t i = 0; i < 16; i++)
    {
        chords_[i].CopySettingsFrom(chordSaves_[presetIndex][i]);
    }

    selectedSave_ = presetIndex;

    return true;
}

bool OmxModeChords::savePreset(uint8_t presetIndex)
{
    if(presetIndex >= NUM_CHORD_SAVES) return false;

    for(uint8_t i = 0; i < 16; i++)
    {
        chordSaves_[presetIndex][i].CopySettingsFrom(chords_[i]);
    }

    selectedSave_ = presetIndex;

    return true;
}

void OmxModeChords::onManualStrumOn(uint8_t chordIndex)
{
    Serial.println("onManualStrumOn: " + String(chordIndex));
    if(chordNotes_[chordIndex].active) 
    {
        Serial.println("chord already active");
        return; // This shouldn't happen
    }

    if(constructChord(chordIndex))
    {
        chordNotes_[chordIndex].active = true;
        chordNotes_[chordIndex].channel = sysSettings.midiChannel;
        // uint8_t velocity = midiSettings.defaultVelocity;

        chordNotes_[chordIndex].strumPos = 0;
        chordNotes_[chordIndex].encDelta = 0;
        chordNotes_[chordIndex].octIncrement = 0;


        // Serial.print("Chord: ");
        // for(uint8_t i = 0; i < 6; i++)
        // {
        //     int note = chordNotes_[chordIndex].notes[i];
        //     // Serial.print(String(note) + " ");
        //     if(note >= 0 && note <= 127)
        //     {
        //         MM::sendNoteOn(note, velocity, chordNotes_[chordIndex].channel);
        //     }
        // }
        // Serial.print("\n");
    }
    else
    {
        Serial.println("constructChord failed");
    }
}

void OmxModeChords::onChordOn(uint8_t chordIndex)
{
    // Serial.println("onChordOn: " + String(chordIndex));
    if(chordNotes_[chordIndex].active) 
    {
        // Serial.println("chord already active");
        return; // This shouldn't happen
    }

    if(constructChord(chordIndex))
    {
        chordNotes_[chordIndex].active = true;
        chordNotes_[chordIndex].channel = sysSettings.midiChannel;
        uint8_t velocity = midiSettings.defaultVelocity;

        // uint32_t noteOnMicros = micros();

        // Serial.print("Chord: ");
        for(uint8_t i = 0; i < 6; i++)
        {
            int note = chordNotes_[chordIndex].notes[i];

            // if(note >= 0 && note <= 127)
            // {
            //     // MM::sendNoteOn(note, velocity, chordNotes_[chordIndex].channel);
            //     pendingNoteOns.insert(note, velocity, chordNotes_[chordIndex].channel, noteOnMicros, false);
            // }

            doNoteOn(note, velocity, chordNotes_[chordIndex].channel);
        }
        // Serial.print("\n");
    }
    else
    {
        // Serial.println("constructChord failed");
    }
}

void OmxModeChords::onChordOff(uint8_t chordIndex)
{
    // Serial.println("onChordOff: " + String(chordIndex));
    if(chordNotes_[chordIndex].active == false) return;

    for (uint8_t i = 0; i < 6; i++)
    {
        int note = chordNotes_[chordIndex].notes[i];

        doNoteOff(note, chordNotes_[chordIndex].channel);

        // if (note >= 0 && note <= 127)
        // {
        //     // MM::sendNoteOff(note, 0, chordNotes_[chordIndex].channel);

        //     pendingNoteOns.remove(note, chordNotes_[chordIndex].channel);
        //     pendingNoteOffs.sendOffNow(note, chordNotes_[chordIndex].channel, false);
        // }
    }
    chordNotes_[chordIndex].active = false;
}

void OmxModeChords::onChordEditOn(uint8_t chordIndex)
{
    // Serial.println("onChordOn: " + String(chordIndex));
    if(chordEditNotes_.active) 
    {
        // Serial.println("chord already active");
        return; // This shouldn't happen
    }

    if(constructChord(chordIndex))
    {
        // chordNotes_[chordIndex].active = true;
        chordNotes_[chordIndex].channel = sysSettings.midiChannel;
        uint8_t velocity = midiSettings.defaultVelocity;

        chordEditNotes_.active = true;
        chordEditNotes_.channel = chordNotes_[chordIndex].channel;

        // uint32_t noteOnMicros = micros();

        // Serial.print("Chord: ");
        for(uint8_t i = 0; i < 6; i++)
        {
            int note = chordNotes_[chordIndex].notes[i];

            chordEditNotes_.notes[i] = note;
            // Serial.print(String(note) + " ");
            // if(note >= 0 && note <= 127)
            // {
            //     // MM::sendNoteOn(note, velocity, chordNotes_[chordIndex].channel);
            //     pendingNoteOns.insert(note, velocity, chordNotes_[chordIndex].channel, noteOnMicros, false);
            // }

            doNoteOn(note, velocity, chordEditNotes_.channel);
        }
        // Serial.print("\n");
    }
    else
    {
        // Serial.println("constructChord failed");
    }
}

void OmxModeChords::onChordEditOff()
{
    // Serial.println("onChordOff: " + String(chordIndex));
    if(chordEditNotes_.active == false) return;

    for (uint8_t i = 0; i < 6; i++)
    {
        int note = chordEditNotes_.notes[i];

        doNoteOff(note, chordEditNotes_.channel);

        // if (note >= 0 && note <= 127)
        // {
        //     // MM::sendNoteOff(note, 0, chordNotes_[chordIndex].channel);

        //     pendingNoteOns.remove(note, chordNotes_[chordIndex].channel);
        //     pendingNoteOffs.sendOffNow(note, chordNotes_[chordIndex].channel, false);
        // }
    }
    chordEditNotes_.active = false;
}

bool OmxModeChords::constructChord(uint8_t chordIndex)
{
    // Serial.println("Constructing Chord: " + String(chordIndex));
    auto chord = chords_[chordIndex];

    int8_t octave = midiSettings.octave + chord.octave;

    uint8_t numNotes = 0;

    for(uint8_t i = 0; i < 6; i++)
    {
        chordNotes_[chordIndex].notes[i] = -1;
    }

    if(chord.numNotes == 0)
    {
        return false;
    }
    else if(chord.numNotes == 1)
    {
        chordNotes_[chordIndex].notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
        numNotes = 1;
    }
    else if(chord.numNotes == 2)
    {
        chordNotes_[chordIndex].notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
        chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 2, octave);
        numNotes = 2;
    }
    else if(chord.numNotes == 3)
    {
        chordNotes_[chordIndex].notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
        chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 2, octave);
        chordNotes_[chordIndex].notes[2] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
        numNotes = 3;
    }
    else if(chord.numNotes == 4)
    {
        chordNotes_[chordIndex].notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
        chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 2, octave);
        chordNotes_[chordIndex].notes[2] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
        chordNotes_[chordIndex].notes[3] = musicScale_->getNoteByDegree(chord.degree + 6, octave);
        numNotes = 4;
    }

    // Serial.println("numNotes: " + String(numNotes));

    switch (chord.voicing)
    {
    case CHRDVOICE_NONE:
    {
    }
    break;
    case CHRDVOICE_POWER:
    {
        if (chord.numNotes > 1)
        {
            chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
        }
        if (chord.numNotes > 2)
        {
            chordNotes_[chordIndex].notes[2] = chordNotes_[chordIndex].notes[1] + 12;
            for (uint8_t i = 3; i < 6; i++)
            {
                chordNotes_[chordIndex].notes[i] = -1;
            }
            numNotes = 3;
        }
    }
    break;
    case CHRDVOICE_SUS2:
    {
        if (chord.numNotes > 1)
        {
            chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 1, octave);
        }
    }
    break;
    case CHRDVOICE_SUS4:
    {
        if (chord.numNotes > 1)
        {
            chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 3, octave);
        }
    }
    break;
    case CHRDVOICE_SUS24:
    {
        if (chord.numNotes > 1)
        {
            chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 1, octave);
        }
        if (chord.numNotes > 2)
        {
            chordNotes_[chordIndex].notes[2] = musicScale_->getNoteByDegree(chord.degree + 3, octave);
        }
    }
    break;
    case CHRDVOICE_ADD6:
    {
        chordNotes_[chordIndex].notes[chord.numNotes] = musicScale_->getNoteByDegree(chord.degree + 5, octave);
        numNotes = chord.numNotes + 1;
    }
    break;
    case CHRDVOICE_ADD69:
    {
        chordNotes_[chordIndex].notes[chord.numNotes] = musicScale_->getNoteByDegree(chord.degree + 5, octave);
        chordNotes_[chordIndex].notes[chord.numNotes + 1] = musicScale_->getNoteByDegree(chord.degree + 8, octave);
        numNotes = chord.numNotes + 2;
    }
    break;
    case CHRDVOICE_KB11:
    {
        if(chord.numNotes > 1)
        {
            chordNotes_[chordIndex].notes[0] = musicScale_->getNoteByDegree(chord.degree + 0, octave);
            chordNotes_[chordIndex].notes[1] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
            numNotes = 2;
        }
        if(chord.numNotes > 2)
        {
            chordNotes_[chordIndex].notes[2] = musicScale_->getNoteByDegree(chord.degree + 8, octave);
            numNotes = 3;
        }
        if(chord.numNotes > 3)
        {
            chordNotes_[chordIndex].notes[3] = musicScale_->getNoteByDegree(chord.degree + 9, octave);
            chordNotes_[chordIndex].notes[4] = musicScale_->getNoteByDegree(chord.degree + 6, octave + 1);
            chordNotes_[chordIndex].notes[5] = musicScale_->getNoteByDegree(chord.degree + 10, octave + 1);
            numNotes = 6;
        }
    }
    break;
    default:
        break;
    }

    // Serial.println("numNotes: " + String(numNotes));

    if (chord.quartalVoicing)
    {
        chordNotes_[chordIndex].notes[0] = AddOctave(chordNotes_[chordIndex].notes[0], 2);
        chordNotes_[chordIndex].notes[1] = AddOctave(chordNotes_[chordIndex].notes[1], 0);
        chordNotes_[chordIndex].notes[2] = AddOctave(chordNotes_[chordIndex].notes[2], 1);
        chordNotes_[chordIndex].notes[3] = AddOctave(chordNotes_[chordIndex].notes[3], -1);
    }

    if(chord.spreadUpDown)
    {
        for(uint8_t i = 0; i < 6; i++)
        {
            if(i % 2 == 0)
            {
                chordNotes_[chordIndex].notes[i] = AddOctave(chordNotes_[chordIndex].notes[i], -1);
            }
            else
            {
                chordNotes_[chordIndex].notes[i] = AddOctave(chordNotes_[chordIndex].notes[i], 1);
            }
        }
    }

    if(chord.spread < 0)
    {
        for(uint8_t i = 0; i < 6; i++)
        {
            if(i % 2 == 0)
            {
                chordNotes_[chordIndex].notes[i] = AddOctave(chordNotes_[chordIndex].notes[i], chord.spread);
            }
        }
    }
    else if(chord.spread > 0)
    {
        for(uint8_t i = 0; i < 6; i++)
        {
            if(i % 2 != 0)
            {
                chordNotes_[chordIndex].notes[i] = AddOctave(chordNotes_[chordIndex].notes[i], chord.spread);
            }
        }
    }

    if(chord.rotate != 0 && numNotes > 0)
    {
        int temp[numNotes];

        uint8_t val = numNotes - chord.rotate;

        uint8_t offset = chord.rotate % numNotes;

        for (uint8_t i = 0; i < offset; i++)
        {
            chordNotes_[chordIndex].notes[i] = AddOctave(chordNotes_[chordIndex].notes[i], 1);
        }

        for (uint8_t i = 0; i < numNotes; i++)
        {
            temp[i] = chordNotes_[chordIndex].notes[abs((i + val) % numNotes)];
        }
        for (int i = 0; i < numNotes; i++)
        {
            chordNotes_[chordIndex].notes[i] = temp[i];
        }
    }

    for(uint8_t i = 0; i < 6; i++)
    {
        chordNotes_[chordIndex].notes[i] = TransposeNote(chordNotes_[chordIndex].notes[i], chord.transpose);
    }

    return true;
}

int OmxModeChords::AddOctave(int note, int8_t octave)
{
    if(note < 0 || note > 127) return -1;

    int newNote = note + (12 * octave);
    if(newNote < 0 || newNote > 127) return -1;
    return newNote;
}

int OmxModeChords::TransposeNote(int note, int8_t semitones)
{
    if(note < 0 || note > 127) return -1;

    int newNote = note + semitones;
    if(newNote < 0 || newNote > 127) return -1;
    return newNote;
}


int OmxModeChords::saveToDisk(int startingAddress, Storage *storage)
{
    int saveSize = sizeof(ChordSettings);

    for (uint8_t saveIndex = 0; saveIndex < NUM_CHORD_SAVES; saveIndex++)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            auto saveBytesPtr = (byte *)(&chordSaves_[saveIndex][i]);
            for (int j = 0; j < saveSize; j++)
            {
                storage->write(startingAddress + j, *saveBytesPtr++);
            }

            startingAddress += saveSize;
        }
    }

    return startingAddress;
}

int OmxModeChords::loadFromDisk(int startingAddress, Storage *storage)
{
    int saveSize = sizeof(ChordSettings);

    for (uint8_t saveIndex = 0; saveIndex < NUM_CHORD_SAVES; saveIndex++)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            auto chord = ChordSettings{};
            auto current = (byte *)&chord;
            for (int j = 0; j < saveSize; j++)
            {
                *current = storage->read(startingAddress + j);
                current++;
            }

            chordSaves_[saveIndex][i] = chord;
            startingAddress += saveSize;
        }
    }

    loadPreset(0);

    return startingAddress;
}