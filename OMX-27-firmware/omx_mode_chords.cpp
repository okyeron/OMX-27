#include "omx_mode_chords.h"
#include "config.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "sequencer.h"
#include "MM.h"

enum ChordsModePage {
    CHRDPAGE_1,
    CHRDPAGE_2
};

OmxModeChords::OmxModeChords()
{
    params_.addPage(4);
    params_.addPage(4);

    chords_[0].numNotes = 3;
    chords_[0].degree = 0;

    chords_[1].numNotes = 3;
    chords_[1].degree = 1;

    chords_[2].numNotes = 4;
    chords_[2].degree = 0;

    chords_[3].numNotes = 4;
    chords_[3].degree = 1;
}

void OmxModeChords::InitSetup()
{
}

void OmxModeChords::onModeActivated()
{
    params_.setSelPageAndParam(0,0);
    encoderSelect_ = true;
}

void OmxModeChords::onClockTick() {
}

void OmxModeChords::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
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

void OmxModeChords::loopUpdate()
{
    updateFuncKeyMode();
}

void OmxModeChords::updateFuncKeyMode()
{
    auto keyState = midiSettings.keyState;

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
}

void OmxModeChords::onEncoderChanged(Encoder::Update enc)
{
    if (encoderSelect_)
    {
        params_.changeParam(enc.dir());
        omxDisp.setDirty();
        return;
    }

    int8_t selPage = params_.getSelPage(); // Add one for readability
    int8_t selParam = params_.getSelParam() + 1;

    // PAGE ONE
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
    
    omxDisp.setDirty();
}

void OmxModeChords::onEncoderButtonDown()
{
    encoderSelect_ = !encoderSelect_;
    omxDisp.setDirty();
}

void OmxModeChords::onEncoderButtonDownLong()
{
}

bool OmxModeChords::shouldBlockEncEdit()
{
    return false;
}

void OmxModeChords::onKeyUpdate(OMXKeypadEvent e)
{
    if(e.held()) return;

    uint8_t thisKey = e.key();
	// auto keyState = midiSettings.keyState;

    if(e.down())
    {
        if(thisKey >= 11)
        {
            onChordOn(thisKey - 11);
        }
    }
    else
    {
        if(thisKey >= 11)
        {
            onChordOff(thisKey - 11);
        }
    }
	

    omxLeds.setDirty();
    omxDisp.setDirty();
}

void OmxModeChords::onKeyHeldUpdate(OMXKeypadEvent e)
{
    
}

void OmxModeChords::updateLEDs()
{
    // bool blinkState = omxLeds.getBlinkState();
}

void OmxModeChords::setupPageLegends()
{
    omxDisp.clearLegends();

    int8_t page = params_.getSelPage();

    switch (page)
    {
    case CHRDPAGE_1:
    {
        // omxDisp.legends[0] = "DS 1";
        // omxDisp.legends[1] = "DS 2";
        // omxDisp.legends[2] = "DS 3";
        // omxDisp.legends[3] = "DS 4";
        // omxDisp.legendVals[0] = grids_.getDensity(0);
        // omxDisp.legendVals[1] = grids_.getDensity(1);
        // omxDisp.legendVals[2] = grids_.getDensity(2);
        // omxDisp.legendVals[3] = grids_.getDensity(3);

        omxDisp.legends[0] = "ROOT";
        omxDisp.legendText[0] = musicScale_->getNoteName(scaleConfig.scaleRoot);
        omxDisp.legends[1] = "SCALE";
        
        omxDisp.legendVals[0] = -127;
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
    default:
        break;
    }
}

void OmxModeChords::onDisplayUpdate()
{
    omxLeds.updateBlinkStates();

    if (omxLeds.isDirty())
    {
        updateLEDs();
    }

    if (omxDisp.isDirty())
    { 
        if (!encoderConfig.enc_edit)
        {
            setupPageLegends();
                omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
        }
    }
}

void OmxModeChords::SetScale(MusicScales *scale)
{
    musicScale_ = scale;
}

void OmxModeChords::onChordOn(uint8_t chordIndex)
{
    Serial.println("onChordOn: " + String(chordIndex));
    if(chordNotes_[chordIndex].active) 
    {
        Serial.println("chord already active");
        return; // This shouldn't happen
    }

    if(constructChord(chordIndex))
    {
        chordNotes_[chordIndex].active = true;
        chordNotes_[chordIndex].channel = sysSettings.midiChannel;
        uint8_t velocity = midiSettings.defaultVelocity;

        Serial.print("Chord: ");
        for(uint8_t i = 0; i < 6; i++)
        {
            int note = chordNotes_[chordIndex].notes[i];
            Serial.print(String(note) + " ");
            if(note >= 0 && note <= 127)
            {
                MM::sendNoteOn(note, velocity, chordNotes_[chordIndex].channel);
            }
        }
        Serial.print("\n");
    }
    else
    {
        Serial.println("constructChord failed");
    }
}

void OmxModeChords::onChordOff(uint8_t chordIndex)
{
    Serial.println("onChordOff: " + String(chordIndex));
    if(chordNotes_[chordIndex].active == false) return;

    for (uint8_t i = 0; i < 6; i++)
    {
        int note = chordNotes_[chordIndex].notes[i];
        if (note >= 0 && note <= 127)
        {
            MM::sendNoteOff(note, 0, chordNotes_[chordIndex].channel);
        }
    }
    chordNotes_[chordIndex].active = false;
}

bool OmxModeChords::constructChord(uint8_t chordIndex)
{
    Serial.println("Constructing Chord: " + String(chordIndex));
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

    Serial.println("numNotes: " + String(numNotes));

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

    Serial.println("numNotes: " + String(numNotes));

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

    if(chord.rotate != 0)
    {
        int temp[numNotes];

        uint8_t val = numNotes - chord.rotate;

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