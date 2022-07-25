#include "midifx_monophonic.h"
#include "omx_disp.h"

namespace midifx
{
    enum ChancePage {
        CHPAGE_1
    };

    MidiFXMonophonic::MidiFXMonophonic()
    {
        params_.addPage(4);
        encoderSelect_ = true;
    }

    int MidiFXMonophonic::getFXType()
    {
        return MIDIFX_MONOPHONIC;
    }

    String MidiFXMonophonic::getName()
    {
        return String("Make Mono");
    }

    void MidiFXMonophonic::onEnabled()
    {
    }

    void MidiFXMonophonic::onDisabled()
    {
    }

    void MidiFXMonophonic::noteInput(MidiNoteGroup note)
    {
        Serial.println("Mono input: " + String(note.noteNumber) + " " + String(note.channel));

        uint8_t midiChannel = constrain(note.channel - 1, 0, 15);

        if(note.noteOff)
        {
            // if (note.unknownLength)
            // {
            //     if (prevNoteOn[midiChannel].noteNumber == note.noteNumber)
            //     {
            //         // mark empty
            //         prevNoteOn[midiChannel].noteNumber = 255;
            //     }
            // }

            if (prevNoteOn[midiChannel].noteNumber == note.noteNumber)
            {
                // mark empty
                prevNoteOn[midiChannel].noteNumber = 255;
            }

            processNoteOff(note);
            return;
        }

        // Probability that effect happens
        if(chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
        {
            Serial.println("Skipping mono");
            sendNoteOut(note);
            return;
        }

        // int s = sizeof(MidiNoteGroupCache);
        // int s2 = sizeof(MidiNoteGroup);

        if (prevNoteOn[midiChannel].noteNumber != 255)
        {
            Serial.println("Prev note found");

            // turn previous note on channel off
            sendNoteOff(prevNoteOn[midiChannel]);
            // mark empty
            // prevNoteOn[midiChannel].noteNumber = 255;
        }
        else
        {
            Serial.println("Prev note not found");
        }

        // Update previous note history
        prevNoteOn[midiChannel].setFromNoteGroup(note);

        sendNoteOut(note);

        // if (note.unknownLength)
        // {
        //     if (prevNoteOn[midiChannel].noteNumber != 255)
        //     {
        //         // turn previous note on channel off
        //         sendNoteOff(prevNoteOn[midiChannel]);
        //         // mark empty
        //         // prevNoteOn[midiChannel].noteNumber = 255;
        //     }

        //     // Update previous note history
        //     prevNoteOn[midiChannel].setFromNoteGroup(note);

        //     sendNoteOut(note);
        // }

        // Serial.println("MidiFXChance::noteInput");
        // note.noteNumber += 7;

        // uint8_t r = random(255);

        // if(r <= chancePerc_)
        // {
        //     processNoteOn(note.noteNumber, note);
        //     sendNoteOut(note);
        // }
    }

    void MidiFXMonophonic::loopUpdate()
    {
    }

    void MidiFXMonophonic::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        auto amt = enc.accel(5); 

        if(page == CHPAGE_1)
        {
            if (param == 0)
            {
                chancePerc_ = constrain(chancePerc_ + amt, 0, 100);
            }
        }
        omxDisp.setDirty();
    }

    void MidiFXMonophonic::onDisplayUpdate()
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
            omxDisp.legendVals[0] = chancePerc_;
            omxDisp.legendVals[1] = -127;
            omxDisp.legendVals[2] = -127;
            omxDisp.legendVals[3] = -127;

            // uint8_t perc = ((chancePerc_ / 255.0f) * 100);
            // String msg = String(perc) + "%";
            // omxDisp.legendText[0] = msg.c_str();
        }
        break;
        default:
            break;
        }

        omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
    }
}