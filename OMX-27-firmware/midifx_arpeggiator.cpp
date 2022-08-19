#include "midifx_arpeggiator.h"
#include "omx_disp.h"
#include "omx_util.h"
#include <algorithm>
// #include <bits/stdc++.h>

namespace midifx
{
    enum ArpPage {
        ARPPAGE_1,
        ARPPAGE_2,
        ARPPAGE_3
    };

    MidiFXArpeggiator::MidiFXArpeggiator()
    {
        arpMode_ = 1;
        midiChannel_ = 0;
        swing_ = 0;
        rateIndex_ = 6;
        octaveRange_ = 1; // 2 Octaves

        params_.addPage(4);
        params_.addPage(4);
        params_.addPage(4);

        encoderSelect_ = true;
    }

    int MidiFXArpeggiator::getFXType()
    {
        return MIDIFX_ARP;
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

    void MidiFXArpeggiator::onModeChanged()
    {
        stopArp();
        playedNoteQueue.clear();
        holdNoteQueue.clear();
        sortedNoteQueue.clear();
    }

    void MidiFXArpeggiator::onEnabled()
    {
        // stopArp();
        // playedNoteQueue.clear();
        // holdNoteQueue.clear();
        // sortedNoteQueue.clear();
    }

    void MidiFXArpeggiator::onDisabled()
    {
        // stopArp();
        // playedNoteQueue.clear();
        // holdNoteQueue.clear();
        // sortedNoteQueue.clear();
    }

    void MidiFXArpeggiator::noteInput(MidiNoteGroup note)
    {
        if(arpMode_ == ARPMODE_OFF)
        {
            processNoteOff(note);
            return;
        }

        if(note.channel != (midiChannel_ + 1))
        {
            sendNoteOut(note);
        }

        if(note.unknownLength)
        {
            if(note.noteOff)
            {
                arpNoteOff(note);
            }
            else
            {
                arpNoteOn(note);
            }
        }

        // Serial.println("MidiFXChance::noteInput");
        // note.noteNumber += 7;

        // uint8_t r = random(255);

        // if(r <= chancePerc_)
        // {
        //     processNoteOn(note.noteNumber, note);
        //     sendNoteOut(note);
        // }
    }

    void MidiFXArpeggiator::startArp()
    {
        Serial.println("startArp");
        omxUtil.resetClocks();
        nextStepTimeP_ = Micros();

        // tickCount_ = 0;
        patPos_ = 0;
        notePos_ = 0;
        octavePos_ = 0;
        // running_ = true;

        nextStepTimeP_ = micros();
        lastStepTimeP_ = micros();
        // startMicros = micros();

        arpRunning_ = true;
    }

    void MidiFXArpeggiator::stopArp()
    {
        Serial.println("stopArp");
        arpRunning_ = false;
    }

    bool MidiFXArpeggiator::insertMidiNoteQueue(MidiNoteGroup note)
    {
        Serial.println("playedNoteQueue capacity: " + String(playedNoteQueue.capacity()));
        if(playedNoteQueue.capacity() > queueSize)
        {
            playedNoteQueue.shrink_to_fit();
        }
        if(holdNoteQueue.capacity() > queueSize)
        {
            holdNoteQueue.shrink_to_fit();
        }

        bool noteAdded = false;

        if(playedNoteQueue.size() < queueSize)
        {
            playedNoteQueue.push_back(ArpNote(note));
            noteAdded = true;
        }

        if(holdNoteQueue.size() < queueSize)
        {
            holdNoteQueue.push_back(ArpNote(note));
            noteAdded = true;
        }

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
        return noteAdded;
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

        if (arpMode_ != ARPMODE_ON)
        {
            for (ArpNote a : holdNoteQueue)
            {
                sortedNoteQueue.push_back(a);
            }
        }
        else
        {
            for (ArpNote a : playedNoteQueue)
            {
                sortedNoteQueue.push_back(a);
            }
        }

        Serial.println("sortedNoteQueue capacity: " + String(sortedNoteQueue.capacity()));

        if(sortedNoteQueue.capacity() > queueSize)
        {
            sortedNoteQueue.shrink_to_fit();
        }

        std::sort(sortedNoteQueue.begin(), sortedNoteQueue.end(), compareArpNote);
    }

    void MidiFXArpeggiator::generatePattern()
    {
        int index = 0;

        Serial.print("Pat: ");

        for(uint8_t o = 0; o < (octaveRange_ + 1); o++)
        {
            for(uint8_t n = 0; n < sortedNoteQueue.size(); n++)
            {
                notePat_[index].noteNumber = sortedNoteQueue[n].noteNumber + (12 * o);
                Serial.print(notePat_[index].noteNumber);
                Serial.print(" ");
                index++;
            }
        }

        Serial.print("\n");

        notePatLength_ = index;

        Serial.print("Length: ");
        Serial.print(notePatLength_);
        Serial.print("\n\n");
    }

    void MidiFXArpeggiator::arpNoteOn(MidiNoteGroup note)
    {
        if(arpMode_ != ARPMODE_ONESHOT && !arpRunning_ )
        {
            startArp();
        }

        if(hasMidiNotes() == false)
        {
            velocity_ = note.velocity;
            sendMidi_ = note.sendMidi;
            sendCV_ = note.sendCV;

            holdNoteQueue.clear();

            if(arpMode_ == ARPMODE_ONESHOT)
            {
                startArp();
            }
        }

        insertMidiNoteQueue(note);
        sortNotes();
        // generatePattern();
    }

    void MidiFXArpeggiator::arpNoteOff(MidiNoteGroup note)
    {
        removeMidiNoteQueue(note);

        sortNotes();
        // generatePattern();

        if(arpMode_ == ARPMODE_ON && hasMidiNotes() == false)
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

            uint8_t rate = kArpRates[rateIndex_];
            multiplier_ = 1.0f / (float)rate;

            stepMicroDelta_ = (clockConfig.step_micros * 16) * multiplier_;

            nextStepTimeP_ += stepMicroDelta_; // calc step based on rate

            arpNoteTrigger();
        }
    }

    void MidiFXArpeggiator::arpNoteTrigger()
    {
        if(sortedNoteQueue.size() == 0)
        {
            return;
        }

        uint32_t noteon_micros = micros();

        // if (swing_ > 0 && patPos_ % 2 == 0)
        // {
        //     if (swing_ < 99)
        //     {
        //         noteon_micros = micros() + ((clockConfig.ppqInterval * multiplier_) / (PPQ / 24) * swing_); // full range swing
        //     }
        //     else if (swing_ == 99)
        //     {                                        // random drunken swing
        //         uint8_t rnd_swing = rand() % 95 + 1; // rand 1 - 95 // randomly apply swing value
        //         noteon_micros = micros() + ((clockConfig.ppqInterval * multiplier_) / (PPQ / 24) * rnd_swing);
        //     }
        // }
        // else
        // {
        //     // noteon_micros = micros();
        // }

        // if(patPos_ >= notePatLength_)
        // {
        //     // reset pattern
        //     patPos_ = 0;
        // }

        if(notePos_ >= sortedNoteQueue.size())
        {
            // reset note position
            notePos_ = 0;
            octavePos_++;
        }

        if(octavePos_ > octaveRange_)
        {
            // reset octave
            octavePos_ = 0;
            if(arpMode_ == ARPMODE_ONESHOT)
            {
                stopArp();
                return;
            }
        }

        ArpNote arpNote = sortedNoteQueue[notePos_];

        arpNote.noteNumber = arpNote.noteNumber + (octavePos_ * 12);

        playNote(noteon_micros, arpNote);

        notePos_++;

        // playNote(noteon_micros, notePat_[patPos_]);


        patPos_++;
    }

    void MidiFXArpeggiator::playNote(uint32_t noteOnMicros, ArpNote note)
    {
        Serial.println("PlayNote: " + String(note.noteNumber));
        if(note.noteNumber > 127) return;

        MidiNoteGroup noteOut;

        noteOut.channel = midiChannel_ + 1;
        noteOut.noteNumber = note.noteNumber;
        noteOut.prevNoteNumber = note.noteNumber;
        noteOut.velocity = velocity_;
        noteOut.stepLength = ((float)gate * 0.01f) * (16.0f * multiplier_);
        noteOut.sendMidi = sendMidi_;
        noteOut.sendCV = sendCV_;
        noteOut.noteonMicros = noteOnMicros;
        noteOut.unknownLength = false;
        noteOut.noteOff = false;

        sendNoteOut(noteOut);
    }

    void MidiFXArpeggiator::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        // auto amt = enc.accel(5);

        auto amtSlow = enc.accel(1);
        auto amtFast = enc.accel(5);

        if(page == ARPPAGE_1) // Hold, rate, octave range, gate
        {
            if (param == 0)
            {
                uint8_t prevArpMode = arpMode_;
                arpMode_ = constrain(arpMode_ + amtSlow, 0, 3);
                if(prevArpMode != arpMode_ && arpMode_ != ARPMODE_HOLD)
                {
                    if((arpMode_ == ARPMODE_ON && hasMidiNotes() == false) || arpMode_ == ARPMODE_OFF)
                    {
                        stopArp();
                    }
                    // omxDisp.displayMessage(tempString_.c_str());
                }
            }
            else if (param == 1)
            {
                rateIndex_ = constrain(rateIndex_ + amtSlow, 0, kNumArpRates - 1);
            }
            else if (param == 2)
            {
                octaveRange_ = constrain(octaveRange_ + amtSlow, 0, 7);
            }
            else if (param == 3)
            {
                gate = constrain(gate + amtFast, 2, 200);
            }
        }
        if(page == ARPPAGE_2) // Pattern, Sort, , BPM
        {
            if (param == 0)
            {
                // holdNotes_ = constrain(holdNotes_ + amt, 0, 1);
            }
            else if (param == 1)
            {
                // rateIndex_ = constrain(rateIndex_ + amt, 0, kNumArpRates - 1);
            }
            else if (param == 3)
            {
                clockConfig.newtempo = constrain(clockConfig.clockbpm + amtFast, 40, 300);
                if (clockConfig.newtempo != clockConfig.clockbpm)
                {
                    // SET TEMPO HERE
                    clockConfig.clockbpm = clockConfig.newtempo;
                    omxUtil.resetClocks();
                }
                // rateIndex_ = constrain(rateIndex_ + amt, 0, kNumArpRates - 1);
            }
        }
        if(page == ARPPAGE_3) // Velocity, midiChannel_, sendMidi, sendCV
        {
            // if (param == 0)
            // {
            //     midiChannel_ = constrain(midiChannel_ + amtSlow, 0, 15);

            //     // velocity_ = constrain(velocity_ + amtFast, 0, 127);
            // }
            // else if (param == 1)
            // {
            //     midiChannel_ = constrain(midiChannel_ + amtSlow, 0, 15);
            // }
            // else if (param == 2)
            // {
            //     sendMidi_ = constrain(sendMidi_ + amtSlow, 0, 1);
            // }
            // else if (param == 2)
            // {
            //     sendCV_ = constrain(sendCV_ + amtSlow, 0, 1);
            // }
        }
        omxDisp.setDirty();
    }

    const char* arpModeDisp_[] = {"OFF", "ON", "1-ST", "HOLD"};

    void MidiFXArpeggiator::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        int8_t page = params_.getSelPage();

        if(page == ARPPAGE_1) // Hold, rate, octave range, gate
        {
            omxDisp.legends[0] = "HOLD";
            omxDisp.legends[1] = "RATE";
            omxDisp.legends[2] = "RANG";
            omxDisp.legends[3] = "GATE";
            omxDisp.legendText[0] = arpModeDisp_[arpMode_];
            omxDisp.useLegendString[1] = true;
            omxDisp.legendString[1] = "1/" + String(kArpRates[rateIndex_]);
            omxDisp.legendVals[2] = (octaveRange_ + 1);
            omxDisp.legendVals[3] = gate;
        }
        else if(page == ARPPAGE_2) // Pattern, Sort, , BPM
        {
            omxDisp.legends[0] = "PAT";
            omxDisp.legends[1] = "SORT";
            omxDisp.legends[2] = "";
            omxDisp.legends[3] = "BPM";
            omxDisp.legendVals[3] = (int)clockConfig.clockbpm;
        }
        else if(page == ARPPAGE_3) // Velocity, midiChannel_, sendMidi, sendCV
        {
            omxDisp.legends[0] = "VEL";
            omxDisp.legends[1] = "CHAN";
            omxDisp.legends[2] = "MIDI";
            omxDisp.legends[3] = "CV";
            omxDisp.legendVals[0] = velocity_;
            omxDisp.legendVals[1] = midiChannel_ + 1;
            omxDisp.legendVals[2] = sendMidi_;
            omxDisp.legendVals[3] = sendCV_;
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