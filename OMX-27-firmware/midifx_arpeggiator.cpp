#include "midifx_arpeggiator.h"
#include "omx_disp.h"
#include "omx_util.h"
#include "omx_leds.h"
#include "colors.h"
#include <algorithm>
// #include <bits/stdc++.h>

namespace midifx
{
    enum ArpPage {
        ARPPAGE_1,
        ARPPAGE_2,
        ARPPAGE_3,
        ARPPAGE_MODPAT,
        ARPPAGE_TRANSPPAT
    };

    const char* kModeDisp_[] = {"OFF", "ON", "1-ST", "ONCE", "HOLD"};

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

    const char *kResetMsg_[] = {
        "Normal",
        "Note",
        "Mod Pat",
        "Transp Pat",
        };

    const char *kResetDisp_[] = {
        "NORM",
        "NOTE",
        "MPAT",
        "TPAT",
    };

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

    const char *kArpModDisp_[] = {
        "×",
        ".",
        "-",
        "R",
        "<",
        ">",
        "\"",
        "#",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6"
       };

    const char *kArpModMsg_[] = {
        "As Played",
        "Rest",
        "Tie",
        "Repeat",
        "LoPitch -Oct",
        "HiPitch +Oct",
        "PwrChord",
        "Chord",
        "Note 1",
        "Note 2",
        "Note 3",
        "Note 4",
        "Note 5",
        "Note 6"};

    MidiFXArpeggiator::MidiFXArpeggiator()
    {
        chancePerc_ = 100;
        arpMode_ = 1;
        resetMode_ = ARPRESET_NORMAL;
        arpPattern_ = 0;
        midiChannel_ = 0;
        swing_ = 0;
        rateIndex_ = 6;
        octaveRange_ = 1; // 2 Octaves
        modPatternLength_ = 15;
        transpPatternLength_ = 15;

        heldKey16_ = -1;

        params_.addPage(4);
        params_.addPage(4);
        params_.addPage(4);
        params_.addPage(17);
        params_.addPage(17);


        encoderSelect_ = true;


        for(uint8_t i = 0; i < 16; i++)
        {
            modPattern_[i].mod = MODPAT_ARPNOTE;
            transpPattern_[i] = 0;

            // if(i % 2 == 0)
            // {
            //     modPattern_[i].mod = MODPAT_ARPNOTE;
            // }
            // else
            // {
            //     modPattern_[i].mod = MODPAT_REST;
            // }
        }
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
        heldKey16_ = -1;
    }

    void MidiFXArpeggiator::onEnabled()
    {
        heldKey16_ = -1;
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

    void MidiFXArpeggiator::onSelected()
    {
        if(arpRunning_)
        {
            resetArpSeq();
            startArp();
        }
    }

    void MidiFXArpeggiator::onDeselected()
    {

    }

    void MidiFXArpeggiator::noteInput(MidiNoteGroup note)
    {
        if(arpMode_ == ARPMODE_OFF)
        {
            sendNoteOut(note);
            return;
        }

        if(note.channel != (midiChannel_ + 1))
        {
            sendNoteOut(note);
        }

        if(chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
        {
            sendNoteOut(note);
            return;
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
        else
        {
            bool canInsert = true;

            if(pendingNotes.size() < queueSize)
            {
                for(uint8_t i = 0; i < pendingNotes.size(); i++)
                {
                    PendingArpNote p = pendingNotes[i];

                    // Note already exists
                    if(p.noteCache.noteNumber == note.noteNumber && p.noteCache.channel == note.channel)
                    {
                        // Update note off time
                        pendingNotes[i].offTime = seqConfig.currentFrameMicros + (note.stepLength * clockConfig.step_micros);
                        canInsert = false;
                        break;
                    }
                }
            }
            else
            {
                canInsert = false;
            }

            if(canInsert)
            {
                Serial.println("Inserting pending note");
                PendingArpNote pendingNote;
                pendingNote.noteCache.setFromNoteGroup(note);
                pendingNote.offTime = seqConfig.currentFrameMicros + (note.stepLength * clockConfig.step_micros);
                pendingNotes.push_back(pendingNote);
                arpNoteOn(note);
            }
            else
            {
                sendNoteOut(note);
            }
            // if(pendingNotes.si)
            // arpNoteOn(note);
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
        nextStepTimeP_ = seqConfig.currentFrameMicros;

        // tickCount_ = 0;
        // patPos_ = 0;
        // notePos_ = 0;
        // octavePos_ = 0;

        // resetArpSeq();

        nextStepTimeP_ = seqConfig.currentFrameMicros;
        lastStepTimeP_ = seqConfig.currentFrameMicros;
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
        if (arpMode_ != ARPMODE_ON && arpMode_ != ARPMODE_ONCE)
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


        if(sortedNoteQueue.size() > 0)
        {
            lowestPitch_ = sortedNoteQueue[0].noteNumber;
            highestPitch_ = sortedNoteQueue[sortedNoteQueue.size() - 1].noteNumber;
        }
        else
        {
            lowestPitch_ = -127;
            highestPitch_ = -127;
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

    // void MidiFXArpeggiator::generatePattern()
    // {
    //     int index = 0;

    //     Serial.print("Pat: ");

    //     for(uint8_t o = 0; o < (octaveRange_ + 1); o++)
    //     {
    //         for(uint8_t n = 0; n < sortedNoteQueue.size(); n++)
    //         {
    //             notePat_[index].noteNumber = sortedNoteQueue[n].noteNumber + (12 * o);
    //             Serial.print(notePat_[index].noteNumber);
    //             Serial.print(" ");
    //             index++;
    //         }
    //     }

    //     Serial.print("\n");

    //     notePatLength_ = index;

    //     Serial.print("Length: ");
    //     Serial.print(notePatLength_);
    //     Serial.print("\n\n");
    // }

    void MidiFXArpeggiator::arpNoteOn(MidiNoteGroup note)
    {
        // if(arpMode_ != ARPMODE_ONESHOT && !arpRunning_ )
        // {
        //     startArp();
        // }

        // if(arpMode_ == ARPMODE_ONESHOT && !arpRunning_)
        // {
        //     startArp(); 
        // }

        if(!arpRunning_)
        {
            startArp();
            resetArpSeq();
        }

        if(hasMidiNotes() == false)
        {
            velocity_ = note.velocity;
            sendMidi_ = note.sendMidi;
            sendCV_ = note.sendCV;

            if(arpMode_ == ARPMODE_ON || arpMode_ == ARPMODE_ONCE)
            {
                resetArpSeq();
            }
            else if(resetMode_ == ARPRESET_NOTE)
            {
                resetArpSeq();
            }

            holdNoteQueue.clear();

            // if(arpMode_ == ARPMODE_ONESHOT) // Only start when no notes for oneshot
            // {
            //     startArp();
            // }
        }
        else
        {
            if(resetMode_ == ARPRESET_NOTE)
            {
                resetArpSeq();
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

        if((arpMode_ == ARPMODE_ON || arpMode_ == ARPMODE_ONCE) && hasMidiNotes() == false)
        {
            stopArp();
        }
    }

    // MidiFXNoteFunction MidiFXChance::getInputFunc()
    // {
    //     return &MidiFXChance::noteInput;
    // }

    // Used with stoping sequencers
    void MidiFXArpeggiator::resync()
    {
        playedNoteQueue.clear();
        holdNoteQueue.clear();
        sortedNoteQueue.clear();
        tempNoteQueue.clear();
        
        resetArpSeq();
    }

    void MidiFXArpeggiator::loopUpdate()
    {
        if (messageTextTimer > 0)
        {
            messageTextTimer -= sysSettings.timeElasped;
            if (messageTextTimer <= 0)
            {
                omxDisp.setDirty();
                omxLeds.setDirty();
                messageTextTimer = 0;
            }
        }

        auto now = seqConfig.currentFrameMicros;

        // Send arp offs for notes that had fixed lengths
        auto it = pendingNotes.begin();
        while (it != pendingNotes.end())
        {
            // remove matching note numbers
            if(it->offTime <= now)
            {
                Serial.println("Removing pending note");
                arpNoteOff(it->noteCache.toMidiNoteGroup());
                // `erase()` invalidates the iterator, use returned iterator
                it = pendingNotes.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // if (patternDirty_)
        // {
        //     regeneratePattern();
        //     patternDirty_ = false;
        // }

        if (!arpRunning_ || !selected_)
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

        uint32_t stepmicros = seqConfig.currentFrameMicros;

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
        Serial.println("resetArpSeq");
        // patPos_ = 0;
        transpPos_ = 0;
        modPos_ = 0;
        notePos_ = 0;
        octavePos_ = 0;

        lastPlayedNoteNumber_ = -127;

        randPrevNote_ = 255;

        goingUp_ = true;
    }

    void MidiFXArpeggiator::arpNoteTrigger()
    {
        if(sortedNoteQueue.size() == 0)
        {
            return;
        }

        uint32_t noteon_micros = seqConfig.currentFrameMicros;

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
                        if (sortedNoteQueue.size() <= 4 && (arpPattern_ == ARPPAT_HI_UP_DOWN || arpPattern_ == ARPPAT_LOW_UP_DOWN))
                        {
                            currentNotePos = 0;
                            goingUp_ = true;
                            incrementOctave = true;
                        }
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

                        endIndex = 3;
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
            if(arpMode_ == ARPMODE_ONESHOT || arpMode_ == ARPMODE_ONCE)
            {
                stopArp();
                return;
            }
        }

        currentNotePos = constrain(currentNotePos, 0, qLength-1);

        ArpNote arpNote = sortedNoteQueue[currentNotePos];
        randPrevNote_ = arpNote.noteNumber;

        int16_t noteNumber = arpNote.noteNumber;


        noteNumber = applyModPattern(noteNumber);
        stepLength_ = findStepLength(); // Can be changed by ties in mod pattern
        
        if (noteNumber != -127)
        {
            noteNumber = applyTranspPattern(noteNumber);

            // Add octave
            noteNumber = noteNumber + (octavePos_ * 12);
            playNote(noteon_micros, noteNumber, velocity_);
        }

        bool seqReset = false;

        // Advance mod pattern
        modPos_++;
        if(modPos_ >= modPatternLength_ + 1)
        {
            if(resetMode_ == ARPRESET_MODPAT)
            {
                resetArpSeq();
                seqReset = true;
            }
            modPos_ = 0;
        }

        // Advance transpose pattern
        transpPos_++;
        if (transpPos_ >= transpPatternLength_ + 1)
        {
            if (resetMode_ == ARPRESET_TRANSPOSEPAT)
            {
                resetArpSeq();
                seqReset = true;
            }
            transpPos_ = 0;
        }

        // if (noteNumber != -127)
        // {
        //     // Add octave
        //     noteNumber = noteNumber + (octavePos_ * 12);
        //     playNote(noteon_micros, noteNumber, velocity_);
        // }

        if(!seqReset)
        {
            notePos_ = nextNotePos;
        }

        // playNote(noteon_micros, notePat_[patPos_]);


        // patPos_++;
    }

    int16_t MidiFXArpeggiator::applyModPattern(int16_t noteNumber)
    {
        uint8_t modMode = modPattern_[modPos_].mod;

        int16_t newNote = noteNumber;

        if(modMode == MODPAT_REPEAT && lastPlayedMod_ == MODPAT_PWRCHORD)
        {
            modMode = MODPAT_PWRCHORD;
        }
        else if(modMode == MODPAT_REPEAT && lastPlayedMod_ == MODPAT_CHORD)
        {
            modMode = MODPAT_CHORD;
        }

        switch (modMode)
        {
        case MODPAT_ARPNOTE:
        {
            newNote = noteNumber;
        }
        break;
        case MODPAT_REST:
        {
            newNote = -127;
        }
        break;
        case MODPAT_TIE:
        {
            newNote = -127;
        }
        break;
        case MODPAT_REPEAT:
        {
            newNote = lastPlayedNoteNumber_;
        }
        break;
        case MODPAT_LOWPITCH_OCTAVE:
        {
            newNote = lowestPitch_ - 12;
            newNote = applyTranspPattern(newNote);
            uint32_t noteon_micros = seqConfig.currentFrameMicros;
            playNote(noteon_micros, newNote, velocity_);
            lastPlayedNoteNumber_ = newNote;
            newNote = -127;
        }
        break;
        case MODPAT_HIGHPITCH_OCTAVE:
        {
            newNote = highestPitch_ + 12;
            newNote = applyTranspPattern(newNote);
            uint32_t noteon_micros = seqConfig.currentFrameMicros;
            playNote(noteon_micros, newNote, velocity_);
            lastPlayedNoteNumber_ = newNote;
            newNote = -127;
        }
        break;
        case MODPAT_PWRCHORD:
        {
            uint32_t noteon_micros = seqConfig.currentFrameMicros;
            stepLength_ = findStepLength();

            if(sortedNoteQueue.size() > 1)
            {
                newNote = lowestPitch_;
                newNote = applyTranspPattern(newNote);
                newNote = newNote + (octavePos_ * 12);
                playNote(noteon_micros, newNote, velocity_);

                newNote = highestPitch_;
                newNote = applyTranspPattern(newNote);
                newNote = newNote + (octavePos_ * 12);
                playNote(noteon_micros, newNote, velocity_);

                newNote = -127; // Don't play this note. 

                // lastPlayedNoteNumber_ = -130;
                lastPlayedMod_ = modMode;
                lastPlayedNoteNumber_ = newNote;
            }
            else // only 1 note in queue
            {
                newNote = noteNumber;
            }
        }
        break;
        case MODPAT_CHORD:
        {
            uint32_t noteon_micros = seqConfig.currentFrameMicros;
            stepLength_ = findStepLength();

            for(ArpNote n : sortedNoteQueue)
            {
                newNote = n.noteNumber;
                newNote = applyTranspPattern(newNote);
                newNote = newNote + (octavePos_ * 12);

                playNote(noteon_micros, newNote, velocity_);
            }

            lastPlayedMod_ = modMode;
            lastPlayedNoteNumber_ = newNote;

            // lastPlayedNoteNumber_ = -131;

            newNote = -127; // Don't play this note. 
        }
        break;
        case MODPAT_NOTE1:
        case MODPAT_NOTE2:
        case MODPAT_NOTE3:
        case MODPAT_NOTE4:
        case MODPAT_NOTE5:
        case MODPAT_NOTE6:
        {
            uint8_t noteIndex = modMode - MODPAT_NOTE1;

            if(arpMode_ == ARPMODE_ON || arpMode_ == ARPMODE_ONCE)
            {
                if (noteIndex < playedNoteQueue.size())
                {
                    newNote = playedNoteQueue[noteIndex].noteNumber;
                }
                else
                {
                    newNote = -127;
                }
            }
            else // Hold or one shot
            {
                if (noteIndex < holdNoteQueue.size())
                {
                    newNote = holdNoteQueue[noteIndex].noteNumber;
                }
                else
                {
                    newNote = -127;
                }
            }
        }
        break;
        }

        if(newNote != -127)
        {
            lastPlayedMod_ = modMode;
            lastPlayedNoteNumber_ = newNote;
        }

        return newNote;
    }

    uint8_t MidiFXArpeggiator::findStepLength()
    {
        uint8_t len = 1;

        for(uint8_t i = 1; i < 16; i++)
        {
            uint8_t modIndex = (modPos_ + i) % (modPatternLength_ + 1);
            uint8_t mod = modPattern_[modIndex].mod;
            if(mod == MODPAT_TIE)
            {
                // Increase length for each tie
                len++;
            }
            else
            {
                break;
            }
        }

        return len;
    }

    int16_t MidiFXArpeggiator::applyTranspPattern(int16_t noteNumber)
    {
        // Simple
        int16_t newNote = noteNumber + transpPattern_[transpPos_];

        return newNote;
    }

    void MidiFXArpeggiator::playNote(uint32_t noteOnMicros, int16_t noteNumber, uint8_t velocity)
    {
        // Serial.println("PlayNote: " + String(note.noteNumber));
        if(noteNumber < 0 || noteNumber > 127) return;

        MidiNoteGroup noteOut;

        noteOut.channel = midiChannel_ + 1;
        noteOut.noteNumber = (uint8_t)noteNumber;
        noteOut.prevNoteNumber = (uint8_t)noteNumber;
        noteOut.velocity = velocity;
        noteOut.stepLength = ((float)gate * 0.01f) * (16.0f * multiplier_) * (float)stepLength_;
        noteOut.sendMidi = sendMidi_;
        noteOut.sendCV = sendCV_;
        noteOut.noteonMicros = noteOnMicros;
        noteOut.unknownLength = false;
        noteOut.noteOff = false;

        // lastPlayedNoteNumber_ = noteNumber;

        sendNoteOut(noteOut);
    }

    void MidiFXArpeggiator::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        // auto amt = enc.accel(5);

        auto amtSlow = enc.accel(1);
        auto amtFast = enc.accel(5);

        if(page == ARPPAGE_1) // Mode, Pattern, Reset mode, Chance
        {
            if (param == 0)
            {
                uint8_t prevArpMode = arpMode_;
                arpMode_ = constrain(arpMode_ + amtSlow, 0, 4);
                if(prevArpMode != arpMode_ && arpMode_ != ARPMODE_HOLD)
                {
                    if((arpMode_ == ARPMODE_ON && hasMidiNotes() == false) || (arpMode_ == ARPMODE_ONCE && hasMidiNotes() == false) || arpMode_ == ARPMODE_OFF)
                    {
                        stopArp();
                    }
                    // omxDisp.displayMessage(tempString_.c_str());
                }
            }
            else if (param == 1)
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
            else if (param == 2)
            {
                uint8_t prevResetMode = resetMode_;
                resetMode_ = constrain(resetMode_ + amtSlow, 0, 4 - 1);
                if(prevResetMode != resetMode_)
                {
                    // omxDisp.displayMessage(kResetMsg_[resetMode_]);
                }
            }
            else if(param == 3)
            {
                chancePerc_ = constrain(chancePerc_ + amtFast, 0, 100);
            }
            
        }
        else if(page == ARPPAGE_2) // Rate, Octave Range, Gate, BPM
        {
            if (param == 0)
            {
                rateIndex_ = constrain(rateIndex_ + amtSlow, 0, kNumArpRates - 1);
            }
            else if (param == 1)
            {
                octaveRange_ = constrain(octaveRange_ + amtSlow, 0, 7);
            }
            else if (param == 2)
            {
                gate = constrain(gate + amtFast, 2, 200);
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
        else if(page == ARPPAGE_3) // Velocity, midiChannel_, sendMidi, sendCV
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
        else if(page == ARPPAGE_MODPAT)
        {
            if(param < 16)
            {
                uint8_t prevMod = modPattern_[param].mod;
                modPattern_[param].mod = constrain(modPattern_[param].mod + amtSlow, 0, MODPAT_NUM_OF_MODS - 1);

                if(prevMod != modPattern_[param].mod)
                {
                    headerMessage_ = kArpModMsg_[modPattern_[param].mod];
                    showMessage();
                }
            }
            else
            {
                modPatternLength_ = constrain(modPatternLength_ + amtSlow, 0, 15);
            }
        }
        else if(page == ARPPAGE_TRANSPPAT)
        {
            if(param < 16)
            {
                transpPattern_[param] = constrain(transpPattern_[param] + amtSlow, -48, 48);
                // transpPattern_[param] = constrain(transpPattern_[param] + amtSlow, 0, 127);
            }
            else
            {
                transpPatternLength_ = constrain(transpPatternLength_ + amtSlow, 0, 15);
            }
        }
        omxDisp.setDirty();
    }

    bool MidiFXArpeggiator::usesKeys()
    {
        return params_.getSelPage() >= ARPPAGE_MODPAT;
    }
    void MidiFXArpeggiator::onKeyUpdate(OMXKeypadEvent e, uint8_t funcKeyMode)
    {
        if(e.held()) return;

        int thisKey = e.key();

        auto page = params_.getSelPage();
        auto param = params_.getSelParam();

        if (funcKeyMode == FUNCKEYMODE_NONE || heldKey16_ >= 0)
        {
            if (e.down())
            {
                if (page == ARPPAGE_MODPAT || page == ARPPAGE_TRANSPPAT)
                {
                    if (heldKey16_ >= 0 && thisKey > 0 && thisKey < 11)
                    {
                        if (page == ARPPAGE_MODPAT)
                        {
                            modPattern_[heldKey16_].mod = thisKey - 1;
                            modCopyBuffer_ = thisKey - 1;

                            headerMessage_ = kArpModMsg_[modPattern_[param].mod];
                            showMessage();
                        }
                        else if (page == ARPPAGE_TRANSPPAT)
                        {
                            transpPattern_[heldKey16_] = thisKey - 1;
                            transpCopyBuffer_ = thisKey - 1;
                        }
                    }
                    // Select step
                    if (thisKey >= 11)
                    {
                        if (param == 16)
                        {
                            if (page == ARPPAGE_MODPAT)
                            {
                                modPatternLength_ = thisKey - 11;
                            }
                            else if (page == ARPPAGE_TRANSPPAT)
                            {
                                transpPatternLength_ = thisKey - 11;
                            }

                            heldKey16_ = -1;
                        }
                        else
                        {
                            if (page == ARPPAGE_MODPAT)
                            {
                                modCopyBuffer_ = modPattern_[thisKey - 11].mod;
                            }
                            else if (page == ARPPAGE_TRANSPPAT)
                            {
                                transpCopyBuffer_ = transpPattern_[thisKey - 11];
                            }

                            params_.setSelParam(thisKey - 11);
                            heldKey16_ = thisKey - 11;
                        }
                    }
                }
            }
            else
            {
                if (thisKey >= 11 && thisKey - 11 == heldKey16_)
                {
                    heldKey16_ = -1;
                }
            }
        }
        else if(funcKeyMode == FUNCKEYMODE_F1)
        {
            if (page == ARPPAGE_MODPAT || page == ARPPAGE_TRANSPPAT)
            {
                if (e.down())
                {
                    if (thisKey >= 11)
                    {
                        if (page == ARPPAGE_MODPAT)
                        {
                            modPattern_[thisKey - 11].mod = 0;
                            modCopyBuffer_ = 0;
                        }
                        else if (page == ARPPAGE_TRANSPPAT)
                        {
                            transpPattern_[thisKey - 11] = 0;
                            transpCopyBuffer_ = 0;
                        }

                        params_.setSelParam(thisKey - 11);

                        headerMessage_ = "Reset: " + String(thisKey - 11 + 1);
                        showMessage();
                    }
                }
            }
        }
        else if(funcKeyMode == FUNCKEYMODE_F2)
        {
            if (page == ARPPAGE_MODPAT || page == ARPPAGE_TRANSPPAT)
            {
                if (e.down())
                {
                    if (thisKey >= 11)
                    {
                        if (page == ARPPAGE_MODPAT)
                        {
                            modPattern_[thisKey - 11].mod = modCopyBuffer_;
                        }
                        else if (page == ARPPAGE_TRANSPPAT)
                        {
                            transpPattern_[thisKey - 11] = transpCopyBuffer_;
                        }

                        params_.setSelParam(thisKey - 11);

                        headerMessage_ = "Pasted: " + String(thisKey - 11 + 1);
                        showMessage();
                    }
                }
            }
        }
        else if(funcKeyMode == FUNCKEYMODE_F3)
        {
            if (page == ARPPAGE_MODPAT || page == ARPPAGE_TRANSPPAT)
            {
                if (e.down())
                {
                    if (thisKey >= 11)
                    {
                        if (page == ARPPAGE_MODPAT)
                        {
                            modCopyBuffer_ = rand() % MODPAT_NUM_OF_MODS;
                            modPattern_[thisKey - 11].mod = modCopyBuffer_;
                        }
                        else if (page == ARPPAGE_TRANSPPAT)
                        {
                            transpCopyBuffer_ = rand() % 12;
                            transpPattern_[thisKey - 11] = transpCopyBuffer_;
                        }

                        params_.setSelParam(thisKey - 11);

                        headerMessage_ = "Random: " + String(thisKey - 11 + 1);
                        showMessage();
                    }
                }
            }
        }
    }
    
    void MidiFXArpeggiator::onKeyHeldUpdate(OMXKeypadEvent e, uint8_t funcKeyMode)
    {
    }

    void MidiFXArpeggiator::updateLEDs(uint8_t funcKeyMode)
    {
        bool blinkState = omxLeds.getBlinkState();

        auto page = params_.getSelPage();
        auto param = params_.getSelParam();

        if(heldKey16_ < 0)
        {
            // Function Keys
            if (funcKeyMode == FUNCKEYMODE_F3)
            {
                auto f3Color = blinkState ? LEDOFF : FUNKTHREE;
                strip.setPixelColor(1, f3Color);
                strip.setPixelColor(2, f3Color);
            }
            else
            {
                auto f1Color = (funcKeyMode == FUNCKEYMODE_F1 && blinkState) ? LEDOFF : FUNKONE;
                strip.setPixelColor(1, f1Color);

                auto f2Color = (funcKeyMode == FUNCKEYMODE_F2 && blinkState) ? LEDOFF : FUNKTWO;
                strip.setPixelColor(2, f2Color);
            }
        }
        else // Key 16 is held, quick change value
        {
            const uint32_t vcolor = 0x101010;
            const uint32_t vcolor2 = 0xD0D0D0;

            if (page == ARPPAGE_MODPAT)
            {
                for (uint8_t i = 0; i < 10; i++)
                {
                    if(modPattern_[heldKey16_].mod == i)
                    {
                        strip.setPixelColor(i+1, blinkState ? vcolor : LEDOFF);
                    }
                    else
                    {
                        strip.setPixelColor(i+1, vcolor);
                    }
                }
            }
            else if (page == ARPPAGE_TRANSPPAT)
            {
                for (uint8_t i = 0; i < 10; i++)
                {
                    if(i <= transpPattern_[heldKey16_])
                    {
                        strip.setPixelColor(i+1, vcolor2);
                    }
                    else
                    {
                        strip.setPixelColor(i+1, vcolor);
                    }
                }
            }
        }

        if(page == ARPPAGE_MODPAT)
        {
            // const auto MSEL = 0xFFC0C0;
            const uint32_t MASP = ORANGE;
            // const uint32_t MREST = 0x440600;
            const uint32_t MREST = 0x100000;
            const uint32_t MTIE = 0x801000;
            const uint32_t MREPEAT = RED;
            const uint32_t MOTHER = 0xFF00FF;

            for (uint8_t i = 0; i < 16; i++)
            {
                if(param == i && blinkState) // Selected
                {
                    // strip.setPixelColor(11 + i, MSEL);
                }
                else
                {
                    if (i < modPatternLength_ + 1)
                    {
                        auto mod = modPattern_[i].mod;

                        if(mod == MODPAT_ARPNOTE)
                        {
                            strip.setPixelColor(11 + i, MASP);
                        }
                        else if(mod == MODPAT_REST)
                        {
                            strip.setPixelColor(11 + i, MREST);
                        }
                        else if(mod == MODPAT_TIE)
                        {
                            strip.setPixelColor(11 + i, MTIE);
                        }
                        else if(mod == MODPAT_REPEAT)
                        {
                            strip.setPixelColor(11 + i, MREPEAT);
                        }
                        else 
                        {
                            strip.setPixelColor(11 + i, MOTHER);
                        }

                    }
                }
            }
        }
        else if(page == ARPPAGE_TRANSPPAT)
        {
            // const auto TSEL = 0x9090FF;
            const uint32_t TZERO = 0x0000FF;
            const uint32_t THIGH = 0x8080FF;
            const uint32_t TLOW = 0x000020;

            for (uint8_t i = 0; i < 16; i++)
            {
                if(param == i && blinkState) // Selected
                {
                    // strip.setPixelColor(11 + i, TSEL);
                }
                else
                {
                    if (i < transpPatternLength_ + 1)
                    {
                        if(transpPattern_[i] == 0)
                        {
                            strip.setPixelColor(11 + i, TZERO);
                        }
                        else if(transpPattern_[i] > 0)
                        {
                            strip.setPixelColor(11 + i, THIGH);
                        }
                        else
                        {
                            strip.setPixelColor(11 + i, TLOW);
                        }
                    }
                }
            }
        }
    }

    void MidiFXArpeggiator::showMessage()
    {
        const uint8_t secs = 5;
        messageTextTimer = secs * 100000;
        omxDisp.setDirty();
    }

    void MidiFXArpeggiator::onDisplayUpdate(uint8_t funcKeyMode)
    {
        int8_t page = params_.getSelPage();
        bool useLabelHeader = false;

        if (messageTextTimer > 0)
        {
            tempString_ = headerMessage_;
            useLabelHeader = true;
        }

        if (!useLabelHeader && funcKeyMode != FUNCKEYMODE_NONE && (page == ARPPAGE_MODPAT || page == ARPPAGE_TRANSPPAT))
        {
            useLabelHeader = true;
            if (funcKeyMode == FUNCKEYMODE_F1)
            {
                tempString_ = "Reset";
                // omxDisp.dispGenericModeLabel("Reset", params_.getNumPages(), params_.getSelPage());
            }
            else if (funcKeyMode == FUNCKEYMODE_F2)
            {
                tempString_ = "Paste";
                // omxDisp.dispGenericModeLabel("Paste", params_.getNumPages(), params_.getSelPage());
            }
            else if (funcKeyMode == FUNCKEYMODE_F3)
            {
                tempString_ = "Random";
                // omxDisp.dispGenericModeLabel("Random", params_.getNumPages(), params_.getSelPage());
            }
        }

        if (page == ARPPAGE_MODPAT)
        {
            const char *modChars[16];
            for (uint8_t i = 0; i < 16; i++)
            {
                modChars[i] = kArpModDisp_[modPattern_[i].mod];
            }

            if(useLabelHeader)
            {
                const char *labels[1];
                labels[0] = tempString_.c_str();

                omxDisp.dispChar16(modChars, modPatternLength_ + 1, constrain(params_.getSelParam(), 0, 15), params_.getNumPages(), params_.getSelPage(), encoderSelect_, true, labels, 1);
            }
            else
            {
                const char *labels[3];

                tempString_ = "LEN: " + String(modPatternLength_ + 1);

                if (params_.getSelParam() < 16)
                {
                    tempString2_ = "SEL: " + String(params_.getSelParam() + 1);
                    tempString3_ = "MOD: " + String(kArpModDisp_[modPattern_[params_.getSelParam()].mod]);
                }
                else
                {
                    tempString2_ = "SEL: -";
                    tempString3_ = "MOD: -";
                }

                labels[0] = tempString_.c_str();
                labels[1] = tempString2_.c_str();
                labels[2] = tempString3_.c_str();
                omxDisp.dispChar16(modChars, modPatternLength_ + 1, params_.getSelParam(), params_.getNumPages(), params_.getSelPage(), encoderSelect_, true, labels, 3);
            }

            return;
        }
        else if (page == ARPPAGE_TRANSPPAT)
        {
            if (useLabelHeader)
            {
                const char *labels[1];
                labels[0] = tempString_.c_str();

                omxDisp.dispValues16(transpPattern_, transpPatternLength_ + 1, -10, 10, true, constrain(params_.getSelParam(), 0, 15), params_.getNumPages(), params_.getSelPage(), encoderSelect_, true, labels, 1);
            }
            else
            {
                const char *labels[3];

                tempString_ = "LEN: " + String(transpPatternLength_ + 1);

                if (params_.getSelParam() < 16)
                {
                    tempString2_ = "SEL: " + String(params_.getSelParam() + 1);
                    tempString3_ = "OFS: " + String(transpPattern_[params_.getSelParam()]);
                }
                else
                {
                    tempString2_ = "SEL: -";
                    tempString3_ = "OFS: -";
                }

                labels[0] = tempString_.c_str();
                labels[1] = tempString2_.c_str();
                labels[2] = tempString3_.c_str();

                omxDisp.dispValues16(transpPattern_, transpPatternLength_ + 1, -10, 10, true, params_.getSelParam(), params_.getNumPages(), params_.getSelPage(), encoderSelect_, true, labels, 3);
            }

            // omxDisp.dispValues16(transpPattern_, transpPatternLength_ + 1, 0, 127, false, params_.getSelParam(), params_.getNumPages(), params_.getSelPage(), encoderSelect_, true, labels, 3);
            return;
        }

        omxDisp.clearLegends();


        if(page == ARPPAGE_1) // Mode, Pattern, Reset mode, Chance
        {
            omxDisp.legends[0] = "MODE";
            omxDisp.legends[1] = "PAT";
            omxDisp.legends[2] = "RSET";
            omxDisp.legends[3] = "CHC%";
            omxDisp.legendText[0] = kModeDisp_[arpMode_];
            omxDisp.legendText[1] = kPatDisp_[arpPattern_];
            omxDisp.legendText[2] = kResetDisp_[resetMode_];
            omxDisp.useLegendString[3] = true;
            omxDisp.legendString[3] = String(chancePerc_) + "%";
        }
        else if(page == ARPPAGE_2) // Rate, Octave Range, Gate, BPM
        {
            omxDisp.legends[0] = "RATE";
            omxDisp.useLegendString[0] = true;
            omxDisp.legendString[0] = "1/" + String(kArpRates[rateIndex_]);

            omxDisp.legends[1] = "RANG";
            omxDisp.legendVals[1] = (octaveRange_ + 1);

            omxDisp.legends[2] = "GATE";
            omxDisp.legendVals[2] = gate;
            
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