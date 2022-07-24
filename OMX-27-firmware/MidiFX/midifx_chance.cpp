#include "midifx_chance.h"
#include "omx_disp.h"

namespace midifx
{
    enum ChancePage {
        CHPAGE_1
    };

    MidiFXChance::MidiFXChance()
    {
        params_.addPage(4);
        encoderSelect_ = true;
    }

    int MidiFXChance::getFXType()
    {
        return MIDIFX_CHANCE;
    }

    String MidiFXChance::getName()
    {
        return String("Chance");
    }

    void MidiFXChance::onEnabled()
    {
    }

    void MidiFXChance::onDisabled()
    {
    }

    void MidiFXChance::noteInput(MidiNoteGroup note)
    {
        if(note.noteOff)
        {
            processNoteOff(note);
            return;
        }

        // Serial.println("MidiFXChance::noteInput");
        // note.noteNumber += 7;

        uint8_t r = random(255);

        if(r <= chancePerc_)
        {
            processNoteOn(note.noteNumber, note);
            sendNoteOut(note);
        }
    }

    // MidiFXNoteFunction MidiFXChance::getInputFunc()
    // {
    //     return &MidiFXChance::noteInput;
    // }

    void MidiFXChance::loopUpdate()
    {
    }

    void MidiFXChance::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        auto amt = enc.accel(5); 

        if(page == CHPAGE_1)
        {
            if (param == 0)
            {
                chancePerc_ = constrain(chancePerc_ + amt, 0, 255);
            }
        }
        omxDisp.setDirty();
    }

    void MidiFXChance::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        int8_t page = params_.getSelPage();

        switch (page)
        {
        case CHPAGE_1:
        {
            omxDisp.legends[0] = "PERC";
            omxDisp.legends[1] = "";
            omxDisp.legends[2] = "";
            omxDisp.legends[3] = "";
            omxDisp.legendVals[0] = -127;
            omxDisp.legendVals[1] = -127;
            omxDisp.legendVals[2] = -127;
            omxDisp.legendVals[3] = -127;

            uint8_t perc = ((chancePerc_ / 255.0f) * 100);
            String msg = String(perc) + "%";
            omxDisp.legendText[0] = msg.c_str();
        }
        break;
        default:
            break;
        }

        omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
    }
}