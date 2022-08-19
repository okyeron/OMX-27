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

    const char* kModeDisp_[] = {"OFF", "ON", "1-ST", "HOLD"};

    const char *kPatMsg_[] = {
        "Up",
        "Down",
        "UpDown",
        "DownUp",
        "Up & Down",
        "Down & Up",
        "Converge",
        "Diverge",
        "Con-Div",
        "Hi-Up",
        "Hi-UpDown",
        "Low-Up",
        "Low-UpDown",
        "As Played",
        "Random",
        "Rand Other",
        "Rand Once"};

    const char *kPatDisp_[] = {
        "UP",
        "DN",
        "UPDN",
        "DNUP",
        "U&D",
        "D&U",
        "CON",
        "DIV",
        "C-V",
        "HI 1",
        "HI 2",
        "LO 1",
        "LO 2",
        "ASP",
        "RAND",
        "ROTH",
        "RONC"};

    MidiFXArpeggiator::MidiFXArpeggiator()
    {
        arpMode_ = 1;
        arpPattern_ = 0;
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
        // patPos_ = 0;
        // notePos_ = 0;
        // octavePos_ = 0;

        resetArpSeq();

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

        // Copy played or held notes to sorted note queue
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

        // Sort low to high. 
        if(arpPattern_ != ARPPAT_AS_PLAYED)
        {
            std::sort(sortedNoteQueue.begin(), sortedNoteQueue.end(), compareArpNote);
        }

        // Serial.println("sortedNoteQueue capacity: " + String(sortedNoteQueue.capacity()));

        // Alternate sorted with upper high note or lower note. 
        if(arpPattern_ == ARPPAT_HI_UP || arpPattern_ == ARPPAT_HI_UP_DOWN || arpPattern_ == ARPPAT_LOW_UP || arpPattern_ == ARPPAT_LOW_UP_DOWN)
        {
            tempNoteQueue.clear();

            auto rootNote = sortedNoteQueue[sortedNoteQueue.size() - 1]; // High note

            if(arpPattern_ == ARPPAT_LOW_UP || arpPattern_ == ARPPAT_LOW_UP_DOWN)
            {
                rootNote = sortedNoteQueue[0]; // Low note
            }
            // CEGB
            // BCBEBG-BE-BCBEBG // on updown, down will need to end at index 2

            for(uint8_t i = 0; i < sortedNoteQueue.size(); i++)
            {
                auto note = sortedNoteQueue[i];

                // add root than note if note is not the base
                if(note.noteNumber != rootNote.noteNumber)
                {
                    tempNoteQueue.push_back(rootNote);
                    tempNoteQueue.push_back(note);
                }
            }

            if(tempNoteQueue.size() == 0)
            {
                tempNoteQueue.push_back(rootNote);
            }

            sortedNoteQueue.clear();

            for (ArpNote a : tempNoteQueue)
            {
                sortedNoteQueue.push_back(a);
            }
        }

        // Randomize notes, playing each note in sorted list only once
        if(arpPattern_ == ARPPAT_RAND_ONCE)
        {
            tempNoteQueue.clear();

            int queueSize = sortedNoteQueue.size();

            for(uint8_t i = 0; i < queueSize; i++)
            {
                int randIndex = rand() % sortedNoteQueue.size();

                auto note = sortedNoteQueue[randIndex];
                tempNoteQueue.push_back(note); // Store in temp

                sortedNoteQueue.erase(sortedNoteQueue.begin() + randIndex); // Remove note from sorted
            }

            // Put temp back in sorted
            sortedNoteQueue.clear();

            for (ArpNote a : tempNoteQueue)
            {
                sortedNoteQueue.push_back(a);
            }
        }

        // Alternate pattern converging in center
        if (arpPattern_ == ARPPAT_CONVERGE || arpPattern_ == ARPPAT_CONVERGE_DIVERGE || arpPattern_ == ARPPAT_DIVERGE)
        {
            uint8_t front = 0;
            uint8_t back = sortedNoteQueue.size() - 1;

            tempNoteQueue.clear();
            for(uint8_t i = 0; i < sortedNoteQueue.size(); i++)
            {
                uint8_t noteIndex = 0;

                // c,e,g,b,d
                // c,d,e,b,g

                if(i % 2 == 0)
                {
                    noteIndex = front;
                    front++;
                }
                else
                {
                    noteIndex = back;
                    back--;
                }

                tempNoteQueue.push_back(sortedNoteQueue[noteIndex]);
            }

            sortedNoteQueue.clear();

            for (ArpNote a : tempNoteQueue)
            {
                sortedNoteQueue.push_back(a);
            }
        }

        // Flip pattern
        if (arpPattern_ == ARPPAT_DOWN || arpPattern_ == ARPPAT_DOWN_AND_UP || arpPattern_ == ARPPAT_DOWN_UP || arpPattern_ == ARPPAT_DIVERGE)
        {
            tempNoteQueue.clear();
            for (ArpNote a : sortedNoteQueue)
            {
                tempNoteQueue.push_back(a);
            }

            sortedNoteQueue.clear();

            for(int8_t i = tempNoteQueue.size() - 1; i >= 0; i--)
            {
                sortedNoteQueue.push_back(tempNoteQueue[i]);
            }

            // auto it = tempNoteQueue.end();
            // while (it != tempNoteQueue.begin())
            // {
            //     auto note = *it;
            //     sortedNoteQueue.push_back(note);
            //     it--;
            // }
        }

        // Keep vectors in check
        if (sortedNoteQueue.capacity() > queueSize)
        {
            sortedNoteQueue.shrink_to_fit();
        }

        if (tempNoteQueue.capacity() > queueSize)
        {
            tempNoteQueue.shrink_to_fit();
        }
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

            resetArpSeq();

            holdNoteQueue.clear();

            if(arpMode_ == ARPMODE_ONESHOT) // Only start when no notes for oneshot
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

    void MidiFXArpeggiator::resetArpSeq()
    {
        patPos_ = 0;
        notePos_ = 0;
        octavePos_ = 0;

        randPrevNote_ = 255;

        goingUp_ = true;
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
        bool incrementOctave = false;
        int currentNotePos = notePos_;
        int nextNotePos = notePos_;
        int qLength = sortedNoteQueue.size();

        switch (arpPattern_)
        {
            case ARPPAT_UP:
            case ARPPAT_DOWN:
            case ARPPAT_CONVERGE:
            case ARPPAT_DIVERGE:
            case ARPPAT_HI_UP:
            case ARPPAT_LOW_UP:
            case ARPPAT_AS_PLAYED:
            {
                if (currentNotePos >= qLength)
                {
                    currentNotePos = 0;
                    incrementOctave = true;
                }
                nextNotePos = currentNotePos + 1;
            }
            break;
            case ARPPAT_UP_DOWN:
            case ARPPAT_DOWN_UP:
            case ARPPAT_CONVERGE_DIVERGE:
            case ARPPAT_HI_UP_DOWN:
            case ARPPAT_LOW_UP_DOWN:
            {
                // Get down
                if(goingUp_)
                {
                    // Turn around
                    if (currentNotePos >= qLength)
                    {
                        goingUp_ = false;
                        currentNotePos = qLength - 2;
                        // incrementOctave = true;
                    }
                }
                // go to town
                else
                {
                    int endIndex = 1;
                    //Boot scootin' boogie
                    
                    if (arpPattern_ == ARPPAT_HI_UP_DOWN || arpPattern_ == ARPPAT_LOW_UP_DOWN)
                    {
                        // CEGB
                        // BCBEBG-BE-BCBEBG // on updown, down will need to end at index 2
                        // CECGCB-CG
                        // CEG
                        // CECG-CE //

                        endIndex = 1;
                    }

                    if (currentNotePos < endIndex)
                    {
                        currentNotePos = 0;
                        goingUp_ = true;
                        incrementOctave = true;
                    }
                }

                if(goingUp_)
                {
                    nextNotePos = currentNotePos + 1;
                }
                else
                {
                    nextNotePos = currentNotePos - 1;
                }
            }
            break;
            case ARPPAT_UP_AND_DOWN:
            case ARPPAT_DOWN_AND_UP:
            {
                // Get down
                if(goingUp_)
                {
                    // Turn around
                    if (currentNotePos >= qLength)
                    {
                        goingUp_ = false;
                        currentNotePos = qLength - 1;
                        // incrementOctave = true;
                    }
                }
                // go to town
                else
                {
                    //Boot scootin' boogie
                    if (currentNotePos < 0)
                    {
                        currentNotePos = 0;
                        goingUp_ = true;
                        incrementOctave = true;
                    }
                }

                if(goingUp_)
                {
                    nextNotePos = currentNotePos + 1;
                }
                else
                {
                    nextNotePos = currentNotePos - 1;
                }
            }
            break;
            case ARPPAT_RAND:
            {
                currentNotePos = rand() % qLength;
                if (notePos_ >= qLength)
                {
                    notePos_ = 0;
                    incrementOctave = true;
                }
                nextNotePos = notePos_ + 1;
            }
            break;
            case ARPPAT_RAND_OTHER:
            {
                if(qLength == 1)
                {
                    currentNotePos = 0;
                }
                else
                {
                    // search up to 4 times the queue size for a note that's not the previous
                    for(uint8_t i = 0; i < queueSize * 4; i++)
                    {
                        currentNotePos = rand() % qLength;

                        if(sortedNoteQueue[currentNotePos].noteNumber != randPrevNote_)
                        {
                            break;
                        }
                    }
                }
                if (notePos_ >= qLength)
                {
                    notePos_ = 0;
                    incrementOctave = true;
                }
                nextNotePos = notePos_ + 1;
            }
            break;
            case ARPPAT_RAND_ONCE:
            {
                if (currentNotePos >= qLength)
                {
                    currentNotePos = 0;
                    incrementOctave = true;
                    sortNotes(); // Resort every time octave increments
                }
                nextNotePos = currentNotePos + 1;
            }
            break;
        }

        if(incrementOctave)
        {
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

        currentNotePos = constrain(currentNotePos, 0, qLength-1);

        ArpNote arpNote = sortedNoteQueue[currentNotePos];

        randPrevNote_ = arpNote.noteNumber;

        arpNote.noteNumber = arpNote.noteNumber + (octavePos_ * 12);

        playNote(noteon_micros, arpNote);

        notePos_ = nextNotePos;

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
                uint8_t prevArpPat = arpPattern_;
                arpPattern_ = constrain(arpPattern_ + amtSlow, 0, ARPPAT_NUM_OF_PATS - 1);
                if(prevArpPat != arpPattern_)
                {
                    omxDisp.displayMessage(kPatMsg_[arpPattern_]);
                    sortNotes();
                }
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

    

    void MidiFXArpeggiator::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        int8_t page = params_.getSelPage();

        if(page == ARPPAGE_1) // Hold, rate, octave range, gate
        {
            omxDisp.legends[0] = "MODE";
            omxDisp.legends[1] = "RATE";
            omxDisp.legends[2] = "RANG";
            omxDisp.legends[3] = "GATE";
            omxDisp.legendText[0] = kModeDisp_[arpMode_];
            omxDisp.useLegendString[1] = true;
            omxDisp.legendString[1] = "1/" + String(kArpRates[rateIndex_]);
            omxDisp.legendVals[2] = (octaveRange_ + 1);
            omxDisp.legendVals[3] = gate;
        }
        else if(page == ARPPAGE_2) // Pattern, Sort, , BPM
        {
            omxDisp.legends[0] = "PAT";
            omxDisp.legends[1] = "";
            omxDisp.legends[2] = "";
            omxDisp.legends[3] = "BPM";
            omxDisp.legendText[0] = kPatDisp_[arpPattern_];
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