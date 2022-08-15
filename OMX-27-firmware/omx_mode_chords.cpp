#include "omx_mode_chords.h"
#include "config.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "sequencer.h"

using namespace grids;

enum ChordsModePage {
    CHRDPAGE_1,
    CHRDPAGE_2
};

OmxModeChords::OmxModeChords()
{
    params_.addPage(4);
    params_.addPage(4);
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

    int thisKey = e.key();
	// auto keyState = midiSettings.keyState;

    if(e.down())
    {
        if(thisKey >= 11)
        {
            onChordOn(11 - thisKey);
        }
    }
    else
    {
        if(thisKey >= 11)
        {
            onChordOff(11 - thisKey);
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

bool OmxModeChords::constructChord(uint8_t chordIndex, ChordNotes notes)
{
    auto chord = chords_[chordIndex];

    int8_t octave = midiSettings.octave;

    for(uint8_t i = 0; i < 6; i++)
    {
        notes.notes[i] = -1;
    }

    if(chord.numNotes == 0)
    {
        return false;
    }
    else if(chord.numNotes == 1)
    {
        notes.notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
        return true;
    }
    else if(chord.numNotes == 2)
    {
        notes.notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
        notes.notes[1] = musicScale_->getNoteByDegree(chord.degree + 2, octave);
    }
    else if(chord.numNotes == 3)
    {
        notes.notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
        notes.notes[1] = musicScale_->getNoteByDegree(chord.degree + 2, octave);
        notes.notes[2] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
    }
    else if(chord.numNotes == 4)
    {
        notes.notes[0] = musicScale_->getNoteByDegree(chord.degree, octave);
        notes.notes[1] = musicScale_->getNoteByDegree(chord.degree + 2, octave);
        notes.notes[2] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
        notes.notes[3] = musicScale_->getNoteByDegree(chord.degree + 6, octave);
    }

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
            notes.notes[1] = musicScale_->getNoteByDegree(chord.degree + 4, octave);
        }
        if (chord.numNotes > 2)
        {
            notes.notes[2] = notes.notes[1] + 12;
            for (uint8_t i = 3; i < 6; i++)
            {
                notes.notes[i] = -1;
            }
        }
    }
    break;
    case CHRDVOICE_SUS2:
    {
        if (chord.numNotes > 1)
        {
            notes.notes[1] = musicScale_->getNoteByDegree(chord.degree + 1, octave);
        }
    }
    break;
    case CHRDVOICE_SUS4:
    {
        if (chord.numNotes > 1)
        {
            notes.notes[1] = musicScale_->getNoteByDegree(chord.degree + 3, octave);
        }
    }
    break;
    case CHRDVOICE_SUS24:
    {
        if (chord.numNotes > 1)
        {
            notes.notes[1] = musicScale_->getNoteByDegree(chord.degree + 1, octave);
        }
        if (chord.numNotes > 2)
        {
            notes.notes[2] = musicScale_->getNoteByDegree(chord.degree + 3, octave);
        }
    }
    break;
    case CHRDVOICE_ADD6:
    {
        notes.notes[chord.numNotes] = musicScale_->getNoteByDegree(chord.degree + 5, octave);
    }
    break;
    case CHRDVOICE_ADD69:
    {
        notes.notes[chord.numNotes] = musicScale_->getNoteByDegree(chord.degree + 5, octave);
        notes.notes[chord.numNotes + 1] = musicScale_->getNoteByDegree(chord.degree + 8, octave);
    }
    break;
    case CHRDVOICE_KB11:
    {
        if(chord.numNotes > 1)
        {
            notes.notes[chord.numNotes] = musicScale_->getNoteByDegree(chord.degree + 5, octave);
        }
    }
    break;
    default:
        break;
    }

    for(uint8_t i = 0; i < chord.numNotes; i++)
    {
        notes.notes[i] = chord.degree + i; 

    }

    chord.degree

    musicScale_->getNoteByDegree()





}