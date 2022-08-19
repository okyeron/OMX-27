#include "midifx_arpeggiator.h"
#include "omx_disp.h"
#include <bits/stdc++.h>

namespace midifx
{
    enum ArpPage {
        ARPPAGE_1
    };

    MidiFXArpeggiator::MidiFXArpeggiator()
    {
        params_.addPage(4);
        encoderSelect_ = true;
    }

    int MidiFXArpeggiator::getFXType()
    {
        return MIDIFX_CHANCE;
    }

    const char* MidiFXArpeggiator::getName()
    {
        return "Arp";
    }

    const char* MidiFXArpeggiator::getDispName()
    {
        return "ARP";
    }

    MidiFXInterface* MidiFXArpeggiator::getClone()
    {
        auto clone = new MidiFXArpeggiator();
        return clone;
    }

    void MidiFXArpeggiator::onEnabled()
    {
    }

    void MidiFXArpeggiator::onDisabled()
    {
    }

    void MidiFXArpeggiator::noteInput(MidiNoteGroup note)
    {
        if(note.noteOff)
        {
            processNoteOff(note);
            return;
        }

        if(note.unknownLength)
        {

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

    void MidiFXArpeggiator::startArp()
    {
        arpRunning_ = true;
    }

    void MidiFXArpeggiator::stopArp()
    {
        arpRunning_ = false;
    }

    bool MidiFXArpeggiator::insertMidiNoteQueue(MidiNoteGroup note)
    {
        if(playedNoteQueue.capacity() > queueSize)
        {
            playedNoteQueue.shrink_to_fit();
        }

        if(playedNoteQueue.size() >= queueSize)
        {
            return false; 
        }

        playedNoteQueue.push_back(ArpNote(note));

        // for (int i = 0; i < queueSize; ++i)
        // {
        //     if (playedNoteQueue[i].inUse)
        //         continue;

        //     playedNoteQueue[i].inUse = true;
        //     playedNoteQueue[i].noteNumber = note.noteNumber;
        //     playedNoteQueue[i].velocity = note.velocity;
        //     playedNoteQueue[i].sendMidi = note.sendMidi;
        //     playedNoteQueue[i].sendCV = note.sendCV;
        //     return true;
        // }
        // return false; // couldn't find room!
    }
    bool MidiFXArpeggiator::removeMidiNoteQueue(MidiNoteGroup note)
    {
        bool foundNoteToRemove = false;
        auto it = playedNoteQueue.begin();
        while (it != playedNoteQueue.end())
        {
            // remove matching note numbers
            if(it->noteNumber == note.noteNumber)
            {
                // `erase()` invalidates the iterator, use returned iterator
                it = playedNoteQueue.erase(it);
            }
            else
            {
                ++it;
            }
        }

        return foundNoteToRemove;
    }

    bool MidiFXArpeggiator::hasMidiNotes()
    {
        return playedNoteQueue.size() > 0;
    }

    // Copies notes from played note queue and sorts them
    void MidiFXArpeggiator::sortNotes()
    {
        sortedNoteQueue.clear();

        for(ArpNote a : playedNoteQueue)
        {
            sortedNoteQueue.push_back(a);
        }

        if(sortedNoteQueue.capacity() > queueSize)
        {
            sortedNoteQueue.shrink_to_fit();
        }

        std::sort(sortedNoteQueue.begin(), sortedNoteQueue.end(), compareArpNote);
    }

    void MidiFXArpeggiator::arpNoteOn(MidiNoteGroup note)
    {
        if(!arpRunning_)
        {
            startArp();
        }

        insertMidiNoteQueue(note);
        sortNotes();
    }

    void MidiFXArpeggiator::arpNoteOff(MidiNoteGroup note)
    {
        removeMidiNoteQueue(note);

        sortNotes();

        if(hasMidiNotes())
        {
            
        }
        else
        {
            stopArp();
        }
    }

    // MidiFXNoteFunction MidiFXChance::getInputFunc()
    // {
    //     return &MidiFXChance::noteInput;
    // }

    void MidiFXArpeggiator::loopUpdate()
    {
        // if (patternDirty_)
        // {
        //     regeneratePattern();
        //     patternDirty_ = false;
        // }

        if (!arpRunning_)
        {
            return;
        }

        //   seqPerc_ = (stepmicros - startMicros) / ((float)max(stepMicroDelta_, 1) * (steps_ + 1));

        // if (steps_ == 0)
        // {
        //     seqPerc_ = 0;

        //     return;
        // }

        //   uint32_t nextBarMicros = stepMicroDelta_ * (steps_ + 1);

        uint32_t stepmicros = micros();

        if (stepmicros >= nextStepTimeP_)
        {
            lastStepTimeP_ = nextStepTimeP_;

            uint8_t rate = kArpRates[rateIndex];
            multiplier_ = 1.0f / (float)rate;

            // if (polyRhythmMode_) // Space all triggers across a bar
            // {
            //     //   stepMicroDelta_ = ((microsperstep * (16 * multiplierPR_)) / steps_) * multiplier_;
            //     stepMicroDelta_ = ((microsperstep * 16) / steps_) * multiplierPR_;
            //     //   stepMicroDelta_ = ((microsperstep * (16 * multiplierPR_)) / steps_) * multiplier_;
            // }
            // else
            // {
            //     stepMicroDelta_ = clockConfig. * multiplier_;
            // }

            stepMicroDelta_ = clockConfig.step_micros * multiplier_;

            nextStepTimeP_ += stepMicroDelta_; // calc step based on rate

            bool trigger = pattern_[seqPos_];

            if (trigger)
            {
                playNote();
                // pendingNoteOns.insert(60, 100, 1, stepmicros, false);
                //   Serial.print((String) "X  ");
            }
            else
            {
                //   Serial.print((String) "-  ");
            }

            //   lastPosP_ = (seqPos_ + 15) % 16;

            advanceStep(stepmicros);

            if (seqPos_ == 0)
            {

                //   Serial.print("\n\n\n");
            }
        }
    }

    void MidiFXArpeggiator::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        auto amt = enc.accel(5); 

        if(page == ARPPAGE_1)
        {
            // if (param == 0)
            // {
            //     chancePerc_ = constrain(chancePerc_ + amt, 0, 255);
            // }
        }
        omxDisp.setDirty();
    }

    void MidiFXArpeggiator::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        int8_t page = params_.getSelPage();

        switch (page)
        {
        case ARPPAGE_1:
        {
            // omxDisp.legends[0] = "CHC%";
            // omxDisp.legends[1] = "";
            // omxDisp.legends[2] = "";
            // omxDisp.legends[3] = "";
            // omxDisp.legendVals[0] = -127;
            // omxDisp.legendVals[1] = -127;
            // omxDisp.legendVals[2] = -127;
            // omxDisp.legendVals[3] = -127;
            // omxDisp.useLegendString[0] = true;
            // uint8_t perc = ((chancePerc_ / 255.0f) * 100);
            // omxDisp.legendString[0] = String(perc) + "%";
        }
        break;
        default:
            break;
        }

        omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
    }

    int MidiFXArpeggiator::saveToDisk(int startingAddress, Storage *storage) 
    {
        return startingAddress;
        // // Serial.println((String)"Saving mfx chance: " + startingAddress); // 5969
        // // Serial.println((String)"chancePerc_: " + chancePerc_);
        // storage->write(startingAddress, chancePerc_);
        // return startingAddress + 1;
    }

    int MidiFXArpeggiator::loadFromDisk(int startingAddress, Storage *storage)
    {
        return startingAddress;
        // // Serial.println((String)"Loading mfx chance: " + startingAddress); // 5969

        // chancePerc_ = storage->read(startingAddress);
        // // Serial.println((String)"chancePerc_: " + chancePerc_);

        // return startingAddress + 1;
    }
}