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

    String MidiFXScaler::getName()
    {
        return String("Scaler");
    }

    void MidiFXScaler::onEnabled()
    {
    }

    void MidiFXScaler::onDisabled()
    {
    }

    void MidiFXScaler::noteInput(MidiNoteGroup note)
    {
        // Probability that we scale the note
        if (chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
        {
            sendNoteOut(note);
            return;
        }

        // transpose original note by rootNote
        int8_t transposedNote = note.noteNumber - rootNote;

        int8_t noteIndex = transposedNote % 12;
        int8_t octave = transposedNote / 12;

        // offset for root note
        // noteIndex = (noteIndex + rootNote) % 12;

        int8_t remapedNoteIndex = scaleRemapper[noteIndex];

        // remove root note offset
        // remapedNoteIndex = (remapedNoteIndex - rootNote + 12) % 12;

        int8_t newNoteNumber = octave * 12 + remapedNoteIndex;
        newNoteNumber = newNoteNumber + rootNote;

        // note out of range, kill
        if (newNoteNumber < 0 || newNoteNumber > 127)
            return;

        note.noteNumber = newNoteNumber;

        sendNoteOut(note);
    }

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

            scaleRemapper[i] = lastNoteIndex;
        }
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
                // int prevRoot = rootNote;
                rootNote = constrain(rootNote + amt, 0, 12 - 1);
                // if (prevRoot != rootNote)
                // {
                //     musicScale->calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
                // }
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
}