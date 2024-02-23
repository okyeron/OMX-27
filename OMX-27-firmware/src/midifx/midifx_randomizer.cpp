#include "midifx_randomizer.h"
#include "../hardware/omx_disp.h"

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

    const char* MidiFXRandomizer::getName()
    {
        return "Randomizer";
    }

    const char* MidiFXRandomizer::getDispName()
    {
        return "RAND";
    }

    MidiFXInterface* MidiFXRandomizer::getClone()
    {
        auto clone = new MidiFXRandomizer();

        clone->noteMinus_ = noteMinus_;
        clone->notePlus_ = notePlus_;
        clone->octMinus_ = octMinus_;
        clone->octPlus_ = octPlus_;
        clone->velMinus_ = velMinus_;
        clone->velPlus_ = velPlus_;
        clone->lengthPerc_ = lengthPerc_;
        clone->chancePerc_ = chancePerc_;

        return clone;
    }

    void MidiFXRandomizer::onEnabled()
    {
    }

    void MidiFXRandomizer::onDisabled()
    {
    }

    void MidiFXRandomizer::noteInput(MidiNoteGroup note)
    {
        if(note.noteOff)
        {
            processNoteOff(note);
            return;
        }

        // Serial.println("MidiFXRandomNote::noteInput");

        // Probability that we randomize the note
        if(chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
        {
            sendNoteOut(note);
            return;
        }

        int8_t origNote = note.noteNumber;

        int8_t octaveMax = octMinus_ + octPlus_ + 1;
        int8_t octave = random(0, octaveMax) - octMinus_;

        note.noteNumber = getRand(note.noteNumber, noteMinus_, notePlus_);
        note.noteNumber = constrain(note.noteNumber + (octave * 12), 0, 127);
        note.velocity = getRand(note.velocity, velMinus_, velPlus_);
        note.stepLength = note.stepLength * map(random(lengthPerc_), 0, 100, 1, 16);


        processNoteOn(origNote, note);

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

    void MidiFXRandomizer::onDisplayUpdate(uint8_t funcKeyMode)
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
            omxDisp.legendVals[0] = noteMinus_;
            omxDisp.legendVals[1] = notePlus_;
            omxDisp.legendVals[2] = octMinus_;
            omxDisp.legendVals[3] = octPlus_;

            // omxDisp.legendVals[0] = -127;
            // omxDisp.legendVals[1] = -127;
            // omxDisp.legendVals[2] = -127;
            // omxDisp.legendVals[3] = -127;
            // String msg1 = "-"+String(noteMinus_);
            // String msg2 = "+"+String(notePlus_);
            // String msg3 = "-"+String(octMinus_);
            // String msg4 = "+"+String(octPlus_);
            // omxDisp.legendText[0] = msg1.c_str();
            // omxDisp.legendText[1] = msg2.c_str();
            // omxDisp.legendText[2] = msg3.c_str();
            // omxDisp.legendText[3] = msg4.c_str();
        }
        break;
        case RZPAGE_2:
        {
            omxDisp.legends[0] = "VEL-";
            omxDisp.legends[1] = "VEL+";
            omxDisp.legends[2] = "LEN%";
            omxDisp.legends[3] = "CHC%";
            omxDisp.legendVals[0] = velMinus_;
            omxDisp.legendVals[1] = velPlus_;
            omxDisp.legendVals[2] = lengthPerc_;
            omxDisp.legendVals[3] = -127;
            omxDisp.useLegendString[3] = true;
            omxDisp.legendString[3] = String(chancePerc_) + "%";

            // omxDisp.legendVals[0] = -127;
            // omxDisp.legendVals[1] = -127;
            // omxDisp.legendVals[2] = -127;
            // omxDisp.legendVals[3] = -127;
            // String msg1 = "-" + String(velMinus_);
            // String msg2 = "+" + String(velPlus_);
            // String msg3 = "+" + String(lengthPerc_) + "%";
            // String msg4 = String(chancePerc_) + "%";
            // omxDisp.legendText[0] = msg1.c_str();
            // omxDisp.legendText[1] = msg2.c_str();
            // omxDisp.legendText[2] = msg3.c_str();
            // omxDisp.legendText[3] = msg4.c_str();
        }
        break;
        default:
            break;
        }

        omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
    }

    int MidiFXRandomizer::saveToDisk(int startingAddress, Storage *storage)
    {
        // Serial.println((String) "Saving mfx randomizer: " + startingAddress); // 5969
        storage->write(startingAddress + 0, noteMinus_);
        storage->write(startingAddress + 1, notePlus_);
        storage->write(startingAddress + 2, octMinus_);
        storage->write(startingAddress + 3, octPlus_);
        storage->write(startingAddress + 4, velMinus_);
        storage->write(startingAddress + 5, velPlus_);
        storage->write(startingAddress + 6, lengthPerc_);
        storage->write(startingAddress + 7, chancePerc_);

        return startingAddress + 8;
    }

    int MidiFXRandomizer::loadFromDisk(int startingAddress, Storage *storage)
    {
        // Serial.println((String) "Loading mfx randomizer: " + startingAddress); // 5969

        noteMinus_ = storage->read(startingAddress + 0);
        notePlus_ = storage->read(startingAddress + 1);
        octMinus_ = storage->read(startingAddress + 2);
        octPlus_ = storage->read(startingAddress + 3);
        velMinus_ = storage->read(startingAddress + 4);
        velPlus_ = storage->read(startingAddress + 5);
        lengthPerc_ = storage->read(startingAddress + 6);
        chancePerc_ = storage->read(startingAddress + 7);

        return startingAddress + 8;
    }
}
