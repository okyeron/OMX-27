#include "midifx_scaler.h"
#include "omx_disp.h"
#include "music_scales.h"

namespace midifx
{
    enum ScalerPage
    {
        SCLPAGE_1
    };

    MidiFXScaler::MidiFXScaler()
    {
        params_.addPage(4);
        encoderSelect_ = true;
        calculateRemap();
    }

    int MidiFXScaler::getFXType()
    {
        return MIDIFX_SCALER;
    }

    const char* MidiFXScaler::getName()
    {
        return "Scaler";
    }

    const char* MidiFXScaler::getDispName()
    {
        return "SCAL";
    }

    void MidiFXScaler::onEnabled()
    {
    }

    void MidiFXScaler::onDisabled()
    {
    }

    void MidiFXScaler::noteInput(MidiNoteGroup note)
    {
        if(note.noteOff)
        {
            processNoteOff(note);
            return;
        }

        // Probability that we scale the note
        if (chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
        {
            sendNoteOut(note);
            return;
        }

        int8_t origNote = note.noteNumber;

        // transpose original note by rootNote
        // int8_t transposedNote = note.noteNumber + rootNote;

        int8_t transposedNote = note.noteNumber;


        int8_t noteIndex = transposedNote % 12;
        int8_t octave = transposedNote / 12;

        // offset for root note
        // noteIndex = (noteIndex + rootNote) % 12;
        // if(noteIndex + rootNote >= 12)
        // {
        //     octave++;
        // }

        // noteIndex = (noteIndex + rootNote) % 12;

        // noteIndex = (noteIndex - rootNote + 12) % 12;

        int8_t remapedNoteIndex = scaleRemapper[noteIndex];

        if(remapedNoteIndex > noteIndex)
        {
            octave--;
        }

        // remapedNoteIndex = (noteIndex + remapedNoteIndex) % 12;

        // remove root note offset
        // remapedNoteIndex = (remapedNoteIndex - rootNote + 12) % 12;

        int8_t newNoteNumber = octave * 12 + remapedNoteIndex;

        // untranspose
        // newNoteNumber = newNoteNumber - rootNote;

        // note out of range, kill
        if (newNoteNumber < 0 || newNoteNumber > 127)
            return;

        note.noteNumber = newNoteNumber;

        processNoteOn(origNote, note);

        sendNoteOut(note);
    }

    // MidiNoteGroup MidiFXScaler::findTriggeredNote(uint8_t noteNumber)
    // {
    //     for(int i = 0; i < triggeredNotes.size(); i++)
    //     {
    //         if(triggeredNotes[i].prevNoteNumber)



    //     }
    //     triggeredNotes.


    //     return nullptr;
    // }

    void MidiFXScaler::calculateRemap()
    {
        if (scaleIndex < 0)
        {
            for (uint8_t i = 0; i < 12; i++)
            {
                scaleRemapper[i] = i; // Chromatic scale
            }
            return;
        }

        auto scalePattern = MusicScales::getScalePattern(scaleIndex);

        uint8_t scaleIndex = 0;
        uint8_t lastNoteIndex = 0;

        // looks through 12 notes, and sets each note to last note in scale
        // so notes out of scale get rounded down to the previous note in the scale.
        for (uint8_t i = 0; i < 12; i++)
        {
            if (scaleIndex < 7 && scalePattern[scaleIndex] == i)
            {
                lastNoteIndex = i;
                scaleIndex++;
            }

            // uint8_t destIndex = (i - rootNote + 12) % 12;
            // uint8_t destIndex = i + rootNote % 12;

            scaleRemapper[i] = (lastNoteIndex + rootNote) % 12;
        }

        if (rootNote > 0)
        {
            // rotate the scale to root
            int8_t temp[12];

            uint8_t val = 12 - rootNote;

            for (uint8_t i = 0; i < 12; i++)
            {
                temp[i] = scaleRemapper[(i + val) % 12];
            }
            for (int i = 0; i < 12; i++)
            {
                scaleRemapper[i] = temp[i];
            }
        }

        // String msg = "scaleRemapper: ";

        // for (int i = 0; i < 12; i++)
        // {
        //     msg += String(scaleRemapper[i]) + ", ";
        // }

        // msg += "\n\n";

        // Serial.println(msg);
    }

    void MidiFXScaler::loopUpdate()
    {
    }

    void MidiFXScaler::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        auto amt = enc.accel(5);

        if (page == SCLPAGE_1)
        {
            if (param == 0)
            {
                int prevRoot = rootNote;
                rootNote = constrain(rootNote + amt, 0, 12 - 1);
                if (prevRoot != rootNote)
                {
                    calculateRemap();
                }
            }
            else if (param == 1)
            {
                int prevPat = scaleIndex;
                scaleIndex = constrain(scaleIndex + amt, -1, MusicScales::getNumScales() - 1);
                if (prevPat != scaleIndex)
                {
                    omxDisp.displayMessage(MusicScales::getScaleName(scaleIndex));
                    calculateRemap();
                }
            }
            else if (param == 3)
            {
                chancePerc_ = constrain(chancePerc_ + amt, 0, 100);
            }
        }

        omxDisp.setDirty();
    }

    void MidiFXScaler::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        int8_t page = params_.getSelPage();

        switch (page)
        {
        case SCLPAGE_1:
        {
            omxDisp.legends[0] = "ROOT";
            omxDisp.legendVals[0] = -127;
            omxDisp.legendText[0] = MusicScales::getNoteName(rootNote);

            omxDisp.legends[1] = "SCALE";
            if (scaleIndex < 0)
            {
                omxDisp.legendVals[1] = -127;
                omxDisp.legendText[1] = "Off";
            }
            else
            {
                omxDisp.legendVals[1] = scaleIndex;
            }

            omxDisp.legends[2] = "";
            omxDisp.legendVals[2] = -127;

            omxDisp.legends[3] = "CHC%";
            omxDisp.legendVals[3] = -127;
            String msg4 = String(chancePerc_) + "%";
            omxDisp.legendText[3] = msg4.c_str();
        }
        break;
        default:
            break;
        }

        omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
    }

    int MidiFXScaler::saveToDisk(int startingAddress, Storage *storage)
    {
        // Serial.println((String) "Saving mfx scaler: " + startingAddress); // 5969
        storage->write(startingAddress + 0, chancePerc_);
        storage->write(startingAddress + 1, (uint8_t)rootNote);
        storage->write(startingAddress + 2, (uint8_t)scaleIndex);

        return startingAddress + 3;
    }

    int MidiFXScaler::loadFromDisk(int startingAddress, Storage *storage)
    {
        // Serial.println((String) "Loading mfx scaler: " + startingAddress); // 5969

        chancePerc_ = storage->read(startingAddress + 0);
        rootNote = (int8_t)storage->read(startingAddress + 1);
        scaleIndex = (int8_t)storage->read(startingAddress + 2);

        calculateRemap();

        return startingAddress + 3;
    }
}