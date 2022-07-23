#include "midifx_randomizer.h"
#include "omx_disp.h"

namespace midifx
{
    enum RandPage {
        RZPAGE_1,
        RZPAGE_2
    };

    MidiFXRandomizer::MidiFXRandomizer()
    {
        params_.addPage(4);
        params_.addPage(4);
        encoderSelect_ = true;
    }

    int MidiFXRandomizer::getFXType()
    {
        return MIDIFX_RANDOMIZER;
    }

    String MidiFXRandomizer::getName()
    {
        return String("Randomizer");
    }

    void MidiFXRandomizer::onEnabled()
    {
    }

    void MidiFXRandomizer::onDisabled()
    {
    }

    void MidiFXRandomizer::noteInput(MidiNoteGroup note)
    {
        // Serial.println("MidiFXRandomNote::noteInput");

        // Probability that we randomize the note
        if(random(100) > chancePerc_)
        {
            sendNoteOut(note);
            return;
        }

        int8_t octaveMax = octMinus_ + octPlus_ + 1;
        int8_t octave = random(0, octaveMax) - octMinus_;

        note.noteNumber = getRand(note.noteNumber, noteMinus_, notePlus_);
        note.noteNumber = constrain(note.noteNumber + (octave * 12), 0, 127);
        note.velocity = getRand(note.velocity, velMinus_, velPlus_);
        note.stepLength = note.stepLength * map(random(lengthPerc_), 0, 100, 1, 16);

        sendNoteOut(note);
    }

    uint8_t MidiFXRandomizer::getRand(uint8_t v, uint8_t minus, uint8_t plus)
    {
        uint8_t minV = max(v - minus, 0);
        uint8_t maxV = min(v + plus + 1, 127);
        return random(minV, maxV);
    }

    void MidiFXRandomizer::loopUpdate()
    {
    }

    void MidiFXRandomizer::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        auto amt = enc.accel(5); 

        if(page == RZPAGE_1)
        {
            if (param == 0)
            {
                noteMinus_ = constrain(noteMinus_ + amt, 0, 12);
            }
            else if (param == 1)
            {
                notePlus_ = constrain(notePlus_ + amt, 0, 12);
            }
            else if (param == 2)
            {
                octMinus_ = constrain(octMinus_ + amt, 0, 12);
            }
            else if (param == 3)
            {
                octPlus_ = constrain(octPlus_ + amt, 0, 12);
            }
        }
        else if(page == RZPAGE_2)
        {
            if (param == 0)
            {
                velMinus_ = constrain(velMinus_ + amt, 0, 127);
            }
            else if (param == 1)
            {
                velPlus_ = constrain(velPlus_ + amt, 0, 127);
            }
            else if (param == 2)
            {
                lengthPerc_ = constrain(lengthPerc_ + amt, 0, 100);
            }
            else if (param == 3)
            {
                chancePerc_ = constrain(chancePerc_ + amt, 0, 100);
            }
        }
        omxDisp.setDirty();
    }

    void MidiFXRandomizer::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        // omxDisp.dispPage = page + 1;

        int8_t page = params_.getSelPage();

        switch (page)
        {
        case RZPAGE_1:
        {
            omxDisp.legends[0] = "NT-";
            omxDisp.legends[1] = "NT+";
            omxDisp.legends[2] = "OCT-";
            omxDisp.legends[3] = "OCT+";
            omxDisp.legendVals[0] = -127;
            omxDisp.legendVals[1] = -127;
            omxDisp.legendVals[2] = -127;
            omxDisp.legendVals[3] = -127;
            String msg1 = "-"+String(noteMinus_);
            String msg2 = "+"+String(notePlus_);
            String msg3 = "-"+String(octMinus_);
            String msg4 = "+"+String(octPlus_);
            omxDisp.legendText[0] = msg1.c_str();
            omxDisp.legendText[1] = msg2.c_str();
            omxDisp.legendText[2] = msg3.c_str();
            omxDisp.legendText[3] = msg4.c_str();
        }
        break;
        case RZPAGE_2:
        {
            omxDisp.legends[0] = "VEL-";
            omxDisp.legends[1] = "VEL+";
            omxDisp.legends[2] = "LEN%";
            omxDisp.legends[3] = "CHC%";
            omxDisp.legendVals[0] = -127;
            omxDisp.legendVals[1] = -127;
            omxDisp.legendVals[2] = -127;
            omxDisp.legendVals[3] = -127;
            String msg1 = "-" + String(velMinus_);
            String msg2 = "+" + String(velPlus_);
            String msg3 = "+" + String(lengthPerc_) + "%";
            String msg4 = String(chancePerc_) + "%";
            omxDisp.legendText[0] = msg1.c_str();
            omxDisp.legendText[1] = msg2.c_str();
            omxDisp.legendText[2] = msg3.c_str();
            omxDisp.legendText[3] = msg4.c_str();
        }
        break;
        default:
            break;
        }

        omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
    }
}