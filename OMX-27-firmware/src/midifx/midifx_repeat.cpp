#include "midifx_repeat.h"
#include "../hardware/omx_disp.h"
#include "../utils/omx_util.h"

namespace midifx
{
    Micros nextRepeatTriggerTime_ = 0;

    enum MidiFXRepeatModes
    {
        MFXREPEATMODE_OFF,
        MFXREPEATMODE_ON,
        MFXREPEATMODE_1SHOT,
        MFXREPEATMODE_ONCE,
        MFXREPEATMODE_HOLD,
        MFXREPEATMODE_COUNT
    };

    const char *kRepeatModeDisp_[] = {"OFF", "ON", "1-ST", "ONCE", "HOLD"};

    enum MidiFXRepeatPages
    {
        MFXREPEATPAGE_CHANCE, // Chance Perc
        MFXREPEATPAGE_MODERATE, // Mode, Rate, RateHz, Gate
        MFXREPEATPAGE_QUANT, // Quant Rate
        MFXREPEATPAGE_FADEVEL,
        MFXREPEATPAGE_FADERATE
    };

	MidiFXRepeat::MidiFXRepeat()
	{
		params_.addPage(1); // MFXREPEATPAGE_CHANCE
		params_.addPage(4); // MFXREPEATPAGE_MODERATE
		params_.addPage(4); // MFXREPEATPAGE_QUANT
		params_.addPage(4); // MFXREPEATPAGE_FADEVEL
		params_.addPage(4); // MFXREPEATPAGE_FADERATE
	
        chancePerc_ = 100;

        mode_ = MFXREPEATMODE_ON;
        numOfRepeats_ = 3;
        rateIndex_ = 6;
        quantizedRateIndex_ = -1; // Use global
        rateHz_ = 100;
        gate_ = 90;
        velStart_ = 50;
        velEnd_ = 100;
        rateStart_ = 6;
        rateEnd_ = 6;
        rateStartHz_ = 100;
        rateEndHz_ = 100;

        fadeVel_ = false;
        fadeRate_ = false;

        quantizeSync_ = quantizedRateIndex_ >= -1; // -2 for off

        noteMaster.setContext(this);
        noteMaster.setProcessNoteFptr(&processNoteForwarder);
        noteMaster.setSendNoteOutFptr(&sendNoteOutForwarder);

        recalcVariables();
        resync();

        changeRepeatMode(MFXREPEATMODE_ON);

		encoderSelect_ = true;
	}

    MidiFXRepeat::~MidiFXRepeat()
    {
        if (seqRunning_)
        {
            seqConfig.numOfActiveArps--;
        }
    }

    int MidiFXRepeat::getFXType()
	{
		return MIDIFX_REPEAT;
	}

	const char *MidiFXRepeat::getName()
	{
		return "Repeat";
	}

	const char *MidiFXRepeat::getDispName()
	{
		return "RPT";
	}

	MidiFXInterface *MidiFXRepeat::getClone()
	{
		auto clone = new MidiFXRepeat();

		clone->chancePerc_ = chancePerc_;
        clone->numOfRepeats_ = numOfRepeats_;
        clone->mode_ = mode_;
        clone->rateIndex_ = rateIndex_;
        clone->quantizedRateIndex_ = quantizedRateIndex_;
        clone->rateHz_ = rateHz_;
        clone->gate_ = gate_;
        clone->velStart_ = velStart_;
        clone->velEnd_ = velEnd_;

        clone->fadeVel_ = fadeVel_;
        clone->fadeRate_ = fadeRate_;

        clone->rateStart_ = rateStart_;
        clone->rateEnd_ = rateEnd_;

        clone->rateStartHz_ = rateStartHz_;
        clone->rateEndHz_ = rateEndHz_;

        clone->recalcVariables();

		return clone;
	}

	void MidiFXRepeat::onEnabled()
	{
	}

	void MidiFXRepeat::onDisabled()
	{
	}

    void MidiFXRepeat::noteInput(MidiNoteGroup note)
    {
        if (mode_ == MFXREPEATMODE_OFF)
        {
            // Serial.println("MFXREPEATMODE_OFF");
            sendNoteOut(note);
            return;
        }

        if (chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
        {
            noteMaster.trackNoteInputPassthrough(&note);
            // // sendNoteOut(note);
            // if(note.unknownLength || note.noteOff)
            // {
            //     trackNoteInputPassthrough(&note, false);
            // }
            // else
            // {
            //     sendNoteOut(note);
            // }
            
            return;
        }


        noteMaster.trackNoteInput(&note);

        // if (note.unknownLength || note.noteOff)
        // {
        //     // only notes of unknown lengths need to be tracked
        //     // notes with fixed lengths will turn off automatically.
        //     trackNoteInputPassthrough(&note, true);
        //     trackNoteInput(&note);
        // }
        // else
        // {
        //     processNoteInput(&note);
        // }
    }

    // If chance is less than 100% and passing through, notes need to be tracked
    // and if the same note comes in without passthrough for a noteoff event, it needs to 
    // be passed through the effect to send noteoff to prevent stuck notes
    // void MidiFXRepeat::trackNoteInputPassthrough(MidiNoteGroup *note, bool ignoreNoteOns)
    // {
    //     // Serial.println("trackNoteInputPassthrough");
    //     // Note on, not ignored
    //     if (!ignoreNoteOns && !note->noteOff)
    //     {
    //         // Serial.println("trackNoteInputPassthrough note on");

    //         // Search for an empty slot in trackingNoteGroupsPassthrough
    //         // If no slots are available/more than 8 notes/ note gets killed. 
    //         for (uint8_t i = 0; i < 8; i++)
    //         {
    //             // Found empty slot
    //             if (trackingNoteGroupsPassthrough[i].prevNoteNumber == 255)
    //             {
    //                 trackingNoteGroupsPassthrough[i].channel = note->channel;
    //                 trackingNoteGroupsPassthrough[i].prevNoteNumber = note->prevNoteNumber;
    //                 trackingNoteGroupsPassthrough[i].noteNumber = note->noteNumber;

    //                 // Send it forward through chain
    //                 sendNoteOut(*note);
    //                 return;
    //             }
    //         }
    //     }

    //     // Note off
    //     if (note->noteOff)
    //     {
    //         // Serial.println("trackNoteInputPassthrough note off");

    //         // bool noteFound = false;

    //         // Search to see if this note is in trackingNoteGroupsPassthrough
    //         // Meaning it was previously passed through
    //         // If it is found, send it through chain
    //         // PrevNoteNumber should be the origin note number before being modified by MidiFX
    //         for (uint8_t i = 0; i < 8; i++)
    //         {
    //             if (trackingNoteGroupsPassthrough[i].prevNoteNumber != 255)
    //             {
    //                 if (trackingNoteGroupsPassthrough[i].channel == note->channel && trackingNoteGroupsPassthrough[i].prevNoteNumber == note->prevNoteNumber)
    //                 {
    //                     note->noteNumber = trackingNoteGroupsPassthrough[i].noteNumber;
    //                     // processNoteInput(note);
    //                     sendNoteOut(*note);
    //                     trackingNoteGroupsPassthrough[i].prevNoteNumber = 255; // mark empty
    //                     // noteFound = true;
    //                 }
    //             }
    //         }

    //         // Should be false if note getting sent to arp
    //         // Avoid double trackNoteInput call
    //         if(!ignoreNoteOns)
    //         {
    //             trackNoteInput(note);
    //         }
    //     }
    // }

    // void MidiFXRepeat::trackNoteInput(MidiNoteGroup *note)
    // {
    //     // Same implementation with more comments in submode_midifx
    //     // Keeps track of previous note ons and and adjusts note number
    //     // for note offs using the prevNoteNumber parameter.
    //     // Why is this necessary?
    //     // If the note is modified by midifx like randomize before the arp
    //     // Then the arp can end up having notes stuck on
    //     // This ensures that notes don't get stuck on.
    //     if (note->noteOff)
    //     {
    //         bool noteFound = false;

    //         for (uint8_t i = 0; i < 8; i++)
    //         {
    //             if (trackingNoteGroups[i].prevNoteNumber != 255)
    //             {
    //                 if (trackingNoteGroups[i].channel == note->channel && trackingNoteGroups[i].prevNoteNumber == note->prevNoteNumber)
    //                 {
    //                     // Serial.println("trackNoteInput note off found in trackingNoteGroups");
    //                     note->noteNumber = trackingNoteGroups[i].noteNumber;
    //                     processNoteInput(note);
    //                     trackingNoteGroups[i].prevNoteNumber = 255; // mark empty
    //                     noteFound = true;
    //                 }
    //             }
    //         }

    //         if (!noteFound)
    //         {
    //             // Serial.println("trackNoteInput note off not found in trackingNoteGroups");

    //             processNoteInput(note);
    //         }
    //     }
    //     else if (!note->noteOff) // Note on
    //     {
    //         for (uint8_t i = 0; i < 8; i++)
    //         {
    //             // Find empty slot
    //             if (trackingNoteGroups[i].prevNoteNumber == 255)
    //             {
    //                 trackingNoteGroups[i].channel = note->channel;
    //                 trackingNoteGroups[i].prevNoteNumber = note->prevNoteNumber;
    //                 trackingNoteGroups[i].noteNumber = note->noteNumber;

    //                 processNoteInput(note);
    //                 return;
    //             }
    //         }
    //     }
    // }

    void MidiFXRepeat::processNoteInput(MidiNoteGroup *note)
    {
        // Unknown length notes, played by human on keyboard
        if (note->unknownLength)
        {
            if (note->noteOff)
            {
                repeatNoteOff(note);
            }
            else
            {
                repeatNoteOn(note);
            }
        }
        // Fixed length notes, generated by sequencer, arp or MFX Repeat
        else // 
        {
            bool canInsert = true;

            if (fixedLengthNotes.size() < queueSize)
            {
                for (uint8_t i = 0; i < fixedLengthNotes.size(); i++)
                {
                    FixedLengthNote f = fixedLengthNotes[i];
                    
                    // Note already exists
                    if (f.noteCache.noteNumber == note->noteNumber && f.noteCache.channel == note->channel)
                    {
                        // TODO: This should actually probably send a note off, then a note on for this note

                        // Update note off time
                        fixedLengthNotes[i].offTime = seqConfig.currentFrameMicros + (note->stepLength * clockConfig.step_micros);
                        canInsert = false;
                        break;
                    }
                }
            }
            else
            {
                canInsert = false;
            }

            if (canInsert)
            {
                // Serial.println("Inserting fixed length note");
                // Insert the note into the queue, calculate when it should turn off
                // And send it through the repeat on
                FixedLengthNote fixedNote;
                fixedNote.noteCache.setFromNoteGroup(*note);
                fixedNote.offTime = seqConfig.currentFrameMicros + (note->stepLength * clockConfig.step_micros);
                fixedLengthNotes.push_back(fixedNote);
                repeatNoteOn(note);
            }
            else
            {
                // Remove from tracking notes
                noteMaster.removeFromTracking(note);
                // for (uint8_t i = 0; i < 8; i++)
                // {
                //     if (trackingNoteGroups[i].prevNoteNumber != 255)
                //     {
                //         if (trackingNoteGroups[i].channel == note->channel && trackingNoteGroups[i].prevNoteNumber == note->prevNoteNumber)
                //         {
                //             trackingNoteGroups[i].prevNoteNumber = 255; // mark empty
                //         }
                //     }
                // }
                // Too many notes, note gets killed. 
                // sendNoteOut(*note);
            }
        }
    }

    bool MidiFXRepeat::hasMidiNotes()
    {
        return playedNoteQueue.size() > 0;
    }

    bool MidiFXRepeat::useRateHz()
    {
        return rateIndex_ < 0;
    }

    float MidiFXRepeat::rateToHz(uint8_t rateHz)
    {
        float hertz = 1.0f;

        if (rateHz < 100)
        {
            hertz = map((float)rateHz, 0.0f, 100.0f, 0.1f, 1.0f);
        }
        else if (rateHz == 100)
        {
            hertz = 1.0f;
        }
        else if (rateHz > 100)
        {
            hertz = map((float)rateHz, 100.0f, 255.0f, 1.0f, 50.0f);
        }

        return hertz;
    }


    void MidiFXRepeat::updateMultiplier()
    {
        if (!multiplierCalculated_)
        {
            // Use Hertz
            if (useRateHz())
            {
                multiplier_ = 1;

                rateInHz_ = rateToHz(rateHz_);
                rateStartInHz_ = rateToHz(rateStartHz_);
                rateEndInHz_ = rateToHz(rateEndHz_);
            }
            else
            {
                uint8_t rate = kArpRates[rateIndex_]; // 8
                // uint8_t rate = 16; // 8
                multiplier_ = 1.0f / (float)rate; // 1 / 8 = 0.125 // Only need to recalculate this if rate changes yo

                rateStartMult_ = 1.0f / ((float)kArpRates[rateStart_]);
                rateEndMult_ = 1.0f / ((float)kArpRates[rateEnd_]);
            }

            multiplierCalculated_ = true;
        }
    }

    bool MidiFXRepeat::insertMidiNoteQueue(MidiNoteGroup *note)
    {
        // Serial.println("insertMidiNoteQueue");
        // Serial.println("playedNoteQueue.capacity(): " + String(playedNoteQueue.capacity()));
        // Serial.println("activeNoteQueue.capacity(): " + String(activeNoteQueue.capacity()));
        // Serial.println("pendingNoteQueue.capacity(): " + String(pendingNoteQueue.capacity()));
        // Serial.println("tempNoteQueue.capacity(): " + String(tempNoteQueue.capacity()));
        // Serial.println("fixedLengthNotes.capacity(): " + String(fixedLengthNotes.capacity()));

        if (playedNoteQueue.capacity() > queueSize)
        {
            playedNoteQueue.shrink_to_fit();
        }

        bool noteAdded = false;

        // Played note queue simply tracks which notes are being played. 
        // These notes do not get played
        if (playedNoteQueue.size() < queueSize)
        {
            playedNoteQueue.push_back(RepeatNote(note));
            noteAdded = true;
        }

        if (activeNoteQueue.capacity() > queueSize)
        {
            activeNoteQueue.shrink_to_fit();
        }

        if (pendingNoteQueue.capacity() > queueSize)
        {
            activeNoteQueue.shrink_to_fit();
        }

        

        // if(!noteAdded)
        // {
        //     Serial.println("Could not add note");
        // }

        // Room up to 16 in playedNoteQueue, can add to pending or active
        if (noteAdded)
        {
            // In these modes the note will be on
            // Remove the note and readd to avoid weird overlapping repeats
            if (mode_ == MFXREPEATMODE_1SHOT || mode_ == MFXREPEATMODE_HOLD)
            {
                removeFromQueue(&activeNoteQueue, note);
                removeFromQueue(&pendingNoteQueue, note);
            }

            if (activeNoteQueue.size() + pendingNoteQueue.size() < queueSize)
            {
                // Add to pending queue, note will be added to active queue on clock
                if (quantizeSync_)
                {
                    // Serial.println("adding quantize note");

                    auto newNote = RepeatNote(note);

                    newNote.repeatCounter = numOfRepeats_ + 1;

                    newNote.velocityStart = note->velocity * velStartPerc_;
                    newNote.velocityEnd = note->velocity * velEndPerc_;

                    newNote.nextTriggerTime = seqConfig.lastClockMicros;

                    pendingNoteQueue.push_back(newNote);
                }
                // No quantization, play immediately by adding to active note queue
                else
                {
                    // Serial.println("adding non-quantize note");

                    auto newNote = RepeatNote(note);

                    newNote.repeatCounter = numOfRepeats_ + 1;

                    newNote.velocityStart = note->velocity * velStartPerc_;
                    newNote.velocityEnd = note->velocity * velEndPerc_;

                    newNote.nextTriggerTime = seqConfig.lastClockMicros;

                    activeNoteQueue.push_back(newNote);
                }
            }
        }

        // Serial.println("Note Added: " + String(noteAdded));
        return noteAdded;
    }

    bool MidiFXRepeat::removeFromQueue(std::vector<RepeatNote> *queue, MidiNoteGroup *note)
    {
        bool foundNoteToRemove = false;

        if (queue->size() > 0)
        {
            auto it = queue->begin();
            while (it != queue->end())
            {
                // Serial.println("activeNoteQueue: " + String(it->noteNumber) + " Chan: " + String(it->channel));

                if (it->noteNumber == note->noteNumber && it->channel == note->channel - 1)
                {
                    // `erase()` invalidates the iterator, use returned iterator
                    it = queue->erase(it);
                    foundNoteToRemove = true;
                }
                else
                {
                    ++it;
                }
            }
        }

        return foundNoteToRemove;
    }


    bool MidiFXRepeat::removeMidiNoteQueue(MidiNoteGroup *note)
    {
        bool foundNoteToRemove = false;

        // Always remove from played notes
        if(removeFromQueue(&playedNoteQueue, note))
        {
            foundNoteToRemove = true;
        }

        switch (mode_)
        {
        // Should not get here
        case MFXREPEATMODE_OFF:
        break;
        case MFXREPEATMODE_ON:
        {
            removeFromQueue(&activeNoteQueue, note);
            removeFromQueue(&pendingNoteQueue, note);
        }
        break;
        // Don't remove from active or pending
        // each note gets removed after playing for number of times
        case MFXREPEATMODE_1SHOT:
        break;
        // Don't remove from active or pending
        // queue will be reset once playedNoteQueue is empty and a new noteon comes in
        case MFXREPEATMODE_HOLD:
        break;
        }

        return foundNoteToRemove;
    }

    void MidiFXRepeat::changeRepeatMode(uint8_t newMode)
    {
        uint8_t prevMode = mode_;
        mode_ = newMode;

        if ((mode_ == MFXREPEATMODE_ON && hasMidiNotes() == false) || (mode_ == MFXREPEATMODE_ONCE && hasMidiNotes() == false) || mode_ == MFXREPEATMODE_OFF)
        {
            stopSeq();
        }

        switch (mode_)
        {
        case MFXREPEATMODE_OFF:
        case MFXREPEATMODE_ON:
            resync();
            break;
        }

        if(prevMode != newMode && newMode == MFXREPEATMODE_OFF)
        {
            seqRunning_ = false;
            recalcVariables();
            resync();
        }
    }

    void MidiFXRepeat::resync()
    {
        playedNoteQueue.clear();
        tempNoteQueue.clear();
        activeNoteQueue.clear();
        pendingNoteQueue.clear();
        fixedLengthNotes.clear();

        resetArpSeq();

        noteMaster.clear();

        // for (uint8_t i = 0; i < 8; i++)
        // {
        //     trackingNoteGroups[i].prevNoteNumber = 255;
        //     trackingNoteGroupsPassthrough[i].prevNoteNumber = 255;
        // }
    }

    void MidiFXRepeat::repeatNoteOn(MidiNoteGroup *note)
    {
        // Serial.println("repeatNoteOn");

        // bool seqReset = false;

        if (!seqRunning_)
        {
            startSeq();
            resetArpSeq();
            // seqReset = true;

            // Serial.println("playedNoteQueue: " + String(playedNoteQueue.size()));
            // Serial.println("activeNoteQueue: " + String(activeNoteQueue.size()));
            // Serial.println("pendingNoteQueue: " + String(pendingNoteQueue.size()));
            // Serial.println("tempNoteQueue: " + String(tempNoteQueue.size()));
            // Serial.println("fixedLengthNotes: " + String(fixedLengthNotes.size()));
        }

        if (hasMidiNotes() == false)
        {
            // Serial.println("No Midi Notes");



            // velocity_ = note.velocity;
            // sendMidi_ = note.sendMidi;
            // sendCV_ = note.sendCV;
            // midiChannel_ = note.channel - 1; // note.channel is 1-16, sub 1 for 0-15


            // resetArpSeq();
            // seqReset = true;

            // For one shot mode, notes are removed from active
            // once they have played a certain number of times
            // Important to clear this for hold mode
            if (mode_ != MFXREPEATMODE_1SHOT)
            {
                // Reset the active note queue
                activeNoteQueue.clear();
            }
        }

        // Serial.println("seqRunning_: " + String(seqRunning_));
        
        // else
        // {
        //     // if (resetMode_ == ARPRESET_NOTE)
        //     // {
        //     //     resetArpSeq();
        //     //     seqReset = true;
        //     // }
        // }

        insertMidiNoteQueue(note);
        // sortNotes();

        // if (seqReset)
        // {
        //     // nextNotePos_ = notePos_;
        //     // prevQLength_ = sortedNoteQueue.size();
        // }

        // if (pendingStop_)
        // {
        //     pendingStop_ = false;
        // }

        // if (!seqReset && !pendingStart_)
        // {
        //     findIndexOfNextNotePos();
        // }
    }
    void MidiFXRepeat::repeatNoteOff(MidiNoteGroup *note)
    {
        // Serial.println("repeatNoteOff");
        removeMidiNoteQueue(note);
        // sortNotes();

        if ((mode_ == MFXREPEATMODE_ON || mode_ == MFXREPEATMODE_ONCE) && hasMidiNotes() == false)
        {
            stopSeq();
        }
        // if (hasMidiNotes())
        // {
        //     findIndexOfNextNotePos();
        // }
    }

    void MidiFXRepeat::startSeq()
    {
        // Serial.println("startArp");
        if (seqRunning_)
            return;

        // pendingStart_ = true;
        // sortOrderChanged_ = false;
        // resetNextTrigger_ = false;

        // pendingStartTime_ = micros();

        // notePos_ = 0;
        // prevNotePos_ = 0;
        // nextNotePos_ = 0;

        // if (omxUtil.areClocksRunning() == false)
        // {
        //     pendingStart_ = true;
        // }
        // else
        // {
        //     doPendingStart();
        // }

        updateMultiplier();

        if(seqConfig.numOfActiveArps <= 0)
        {
            // omxUtil.resetPPQCounter();
        }

        // if (omxUtil.areClocksRunning() == false)
        // {
        //     omxUtil.restartClocks();
        //     omxUtil.startClocks();
        //     // uint8_t rate = kArpRates[rateIndex_];
        //     // multiplier_ = 1.0f / (float)rate;
        //     stepMicroDelta_ = (clockConfig.step_micros * 16) * multiplier_;
        //     nextStepTimeP_ = seqConfig.lastClockMicros; // Should be current time, start now.
        //     nextRepeatTriggerTime_ = nextStepTimeP_;

        //     next16thTime_ = seqConfig.lastClockMicros;
        //     last16thTime_ = seqConfig.lastClockMicros;
        // }
        // else
        // {
        //     nextStepTimeP_ = nextRepeatTriggerTime_;
        // }

        // lastStepTimeP_ = nextStepTimeP_;

        seqRunning_ = true;
        // pendingStart_ = false;
        // pendingStop_ = false;

        seqConfig.numOfActiveArps++;
    }

    void MidiFXRepeat::stopSeq()
    {
        // pendingStart_ = false;
        // pendingStop_ = false;
        // arpRunning_ = false;
        // pendingStopCount_ = 0;

        // doPendingStop();

        if (seqRunning_)
        {
            // Stop clocks if last arp
            seqConfig.numOfActiveArps--;
            // if (seqConfig.numOfActiveArps <= 0)
            // {
            //     omxUtil.stopClocks();
            // }
        }

        seqRunning_ = false;
        // pendingStart_ = false;
        // pendingStop_ = false;

        // // Serial.println("stopArp");
        // arpRunning_ = false;
        // pendingStart_ = false;
    }

    void MidiFXRepeat::resetArpSeq()
    {
        // Serial.println("resetArpSeq");
        // patPos_ = 0;
        // transpPos_ = 0;
        // modPos_ = 0;
        // notePos_ = 0;
        // octavePos_ = 0;
        // syncPos_ = 0;

        // lastPlayedNoteNumber_ = -127;

        // randPrevNote_ = 255;

        // goingUp_ = true;
        // resetNextTrigger_ = false;

        // prevNotePos_ = 0;
        // nextNotePos_ = 0;
    }

    // void MidiFXRepeat::sortNotes()
    // {
    //     activeNoteQueue.clear();

    //     // Copy played or held notes to sorted note queue
    //     if (mode_ != MFXREPEATMODE_ON && mode_ != MFXREPEATMODE_ONCE)
    //     {
    //         for (RepeatNote a : holdNoteQueue)
    //         {
    //             activeNoteQueue.push_back(a);
    //         }
    //     }
    //     else
    //     {
    //         for (RepeatNote a : playedNoteQueue)
    //         {
    //             activeNoteQueue.push_back(a);
    //         }
    //     }

    //     if (activeNoteQueue.size() == 0)
    //         return; // Not much to do without any notes

    //     // Keep vectors in check
    //     if (activeNoteQueue.capacity() > queueSize)
    //     {
    //         activeNoteQueue.shrink_to_fit();
    //     }

    //     if (tempNoteQueue.capacity() > queueSize)
    //     {
    //         tempNoteQueue.shrink_to_fit();
    //     }
    // }

    void MidiFXRepeat::triggerNote(RepeatNote note)
    {
        if(note.noteNumber > 127) return;

        playNote(seqConfig.currentFrameMicros, note.nextTriggerDelta, note.noteNumber, note.velocity, note.channel);
    }

    // void MidiFXRepeat::repeatNoteTrigger()
    // {
    //     // Serial.println("repeatNoteTrigger");

    //     if (activeNoteQueue.size() == 0)
    //     {
    //         // Serial.println("no sorted notes");

    //         return;
    //     }

    //     uint32_t noteon_micros = seqConfig.currentFrameMicros;

    //     // if (resetNextTrigger_)
    //     // {
    //     //     resetArpSeq();
    //     // }

    //     // bool incrementOctave = false;
    //     // int currentNotePos = notePos_;
    //     // int nextNotePos = notePos_;
    //     // int qLength = sortedNoteQueue.size();

    //     // prevNotePos_ = notePos_;
        
    //     // prevQLength_ = qLength;

    //     // syncPos_ = syncPos_ + 1 % 16;

    //     // currentNotePos = constrain(currentNotePos, 0, qLength - 1);

    //     for (RepeatNote r : activeNoteQueue)
    //     {
    //         if (r.noteNumber >= 0 && r.noteNumber <= 127)
    //         {
    //             playNote(noteon_micros, r.noteNumber, r.velocity, r.channel);
    //         }
    //     }

    //     // ArpNote arpNote = sortedNoteQueue[currentNotePos];
    //     // randPrevNote_ = arpNote.noteNumber;

    //     // int16_t noteNumber = arpNote.noteNumber;

    //     // noteNumber = applyModPattern(noteNumber, arpNote.channel);
    //     // stepLength_ = findStepLength(); // Can be changed by ties in mod pattern

    //     // if (noteNumber != -127)
    //     // {
    //     //     noteNumber = applyTranspPattern(noteNumber);

    //     //     // Add octave
    //     //     noteNumber = noteNumber + (octavePos_ * octDistance_);
    //     //     playNote(noteon_micros, noteNumber, velocity_, arpNote.channel);
    //     // }

    //     // bool seqReset = false;

    //     // // Advance mod pattern
    //     // modPos_++;
    //     // if (modPos_ >= modPatternLength_ + 1)
    //     // {
    //     //     if (resetMode_ == ARPRESET_MODPAT)
    //     //     {
    //     //         resetArpSeq();
    //     //         seqReset = true;
    //     //     }
    //     //     modPos_ = 0;
    //     // }

    //     // // Advance transpose pattern
    //     // transpPos_++;
    //     // if (transpPos_ >= transpPatternLength_ + 1)
    //     // {
    //     //     if (resetMode_ == ARPRESET_TRANSPOSEPAT)
    //     //     {
    //     //         resetArpSeq();
    //     //         seqReset = true;
    //     //     }
    //     //     transpPos_ = 0;
    //     // }
        
    //     // if (!seqReset)
    //     // {
    //     //     notePos_ = nextNotePos;

    //     //     nextNotePos_ = (notePos_ + qLength) % qLength;
    //     // }
    //     // else
    //     // {
    //     //     nextNotePos_ = notePos_;
    //     // }

    //     // prevSortedNoteQueue.clear();

    //     // for (ArpNote a : sortedNoteQueue)
    //     // {
    //     //     prevSortedNoteQueue.push_back(a);
    //     // }

    //     // playNote(noteon_micros, notePat_[patPos_]);

    //     // patPos_++;
    // }

    void MidiFXRepeat::playNote(uint32_t noteOnMicros, uint32_t lengthDelta, int16_t noteNumber, uint8_t velocity, uint8_t channel)
    {
        // Serial.println("SeqRunning: " + String(seqRunning_));
        // Serial.println("PlayNote: " + String(noteNumber) + " " + String(velocity) + " " + String(channel));
        if (noteNumber < 0 || noteNumber > 127)
            return;

        MidiNoteGroup noteOut;

        noteOut.channel = channel + 1;
        noteOut.noteNumber = (uint8_t)noteNumber;
        noteOut.prevNoteNumber = (uint8_t)noteNumber;
        noteOut.velocity = velocity;
        // noteOut.stepLength = ((float)gate_ * 0.01f) * (16.0f * multiplier_);

        noteOut.stepLength = (lengthDelta * ((float)gate_ * 0.01f)) / clockConfig.step_micros;

        // Serial.println("stepLength: " + String(noteOut.stepLength));

        // noteOut.stepLength = ((float)gate_ * 0.01f) * (16.0f * multiplier_);

        // noteOut.sendMidi = sendMidi_;
        // noteOut.sendCV = sendCV_;

        noteOut.sendMidi = true;
        noteOut.sendCV = true;
        noteOut.noteonMicros = noteOnMicros;
        noteOut.unknownLength = false;
        noteOut.noteOff = false;

        // lastPlayedNoteNumber_ = noteNumber;

        sendNoteOut(noteOut);
    }

    void MidiFXRepeat::onClockTick()
    {
        if(pendingNoteQueue.size() == 0) return;

        uint8_t quantIndex = quantizedRateIndex_ < 0 ? clockConfig.globalQuantizeStepIndex : quantizedRateIndex_; // Use global or local quantize rate?

        bool isQuantizedStep = seqConfig.currentClockTick % (96 * 4 / kArpRates[quantIndex]) == 0;

        // Move pending notes to active
        if(isQuantizedStep)
        {
            for (uint8_t i = 0; i < pendingNoteQueue.size(); i++)
            {
                pendingNoteQueue[i].nextTriggerTime = seqConfig.lastClockMicros;
                activeNoteQueue.push_back(pendingNoteQueue[i]);
            }

            pendingNoteQueue.clear();
        }
    }

    void MidiFXRepeat::loopUpdate()
	{
        auto now = seqConfig.currentFrameMicros;

        // Send arp offs for notes that had fixed lengths
        auto it = fixedLengthNotes.begin();
        while (it != fixedLengthNotes.end())
        {
            // remove matching note numbers
            if (it->offTime <= now)
            {
                auto noteGroup = it->noteCache.toMidiNoteGroup();

                // Serial.println("Removing pending note");
                repeatNoteOff(&noteGroup);
                // `erase()` invalidates the iterator, use returned iterator
                it = fixedLengthNotes.erase(it);
            }
            else
            {
                ++it;
            }
        }

        if (!seqRunning_)
        {
            return;
        }

        if (sysSettings.omxMode == MODE_MIDI && !selected_)
        {
            // Serial.println("Not selected");
            return;
        }

        updateMultiplier();

        uint32_t stepmicros = seqConfig.currentFrameMicros;

        tempNoteQueue.clear();

        // Loop through all the notes and see which notes should be triggered this frame
        // if a note should be triggered it gets added to the tempNoteQueue
        for(uint8_t i = 0; i < activeNoteQueue.size(); i++)
        {
            // The time has come to
            if(stepmicros >= activeNoteQueue[i].nextTriggerTime)
            {
                if(fadeVel_)
                {
                    activeNoteQueue[i].velocity = map(activeNoteQueue[i].repeatCounter, 0, numOfRepeats_, activeNoteQueue[i].velocityEnd, activeNoteQueue[i].velocityStart);
                }
                
                if (useRateHz()) // Use hertz
                {
                    float hzRate = fadeRate_ ? map((float)activeNoteQueue[i].repeatCounter, 0.0f, (float)numOfRepeats_, rateEndInHz_, rateStartInHz_) : rateInHz_;

                    activeNoteQueue[i].nextTriggerDelta = (1.0f / hzRate) * secs2micros;
                }
                else // Synced
                {
                    float mult = fadeRate_ ? map((float)activeNoteQueue[i].repeatCounter, 0.0f, (float)numOfRepeats_, rateEndMult_, rateStartMult_) : multiplier_;

                    activeNoteQueue[i].nextTriggerDelta = clockConfig.step_micros * 16 * mult; // 1/8th note, 120 bpm, 124992 * 16 * 0.125 = 249984 / 124992 = 2 steps, 16 / 2 = 8
                }

                activeNoteQueue[i].nextTriggerTime = activeNoteQueue[i].nextTriggerTime + activeNoteQueue[i].nextTriggerDelta;

                if(activeNoteQueue[i].repeatCounter > 0)
                {
                    activeNoteQueue[i].repeatCounter -= 1;
                }

                // Don't hold back
                tempNoteQueue.push_back(activeNoteQueue[i]);
            }
        }

        // Trigger any notes that should be triggered this frame
        for (RepeatNote n : tempNoteQueue)
        {
            triggerNote(n);
        }

        // Once and 1shot modes will use repeat counter
        // if any of these counters reached 0, remove the note
        if ((mode_ == MFXREPEATMODE_1SHOT || mode_ == MFXREPEATMODE_ONCE) && activeNoteQueue.size() > 0)
        {
            auto it = activeNoteQueue.begin();
            while (it != activeNoteQueue.end())
            {
                if(it->repeatCounter <= 0)
                {
                    it = activeNoteQueue.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
	}

    void MidiFXRepeat::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        auto amtSlow = enc.accel(1);
        auto amtFast = enc.accel(5);

        switch (page)
        {
        case MFXREPEATPAGE_CHANCE:
        {
            switch (param)
            {
            case 0:
                chancePerc_ = constrain(chancePerc_ + amtSlow, 0, 100);
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            }
        }
        break;
        case MFXREPEATPAGE_MODERATE:
        {
            switch (param)
            {
            case 0:
            {
                uint8_t newMode = constrain(mode_ + amtSlow, 0, MFXREPEATMODE_COUNT - 1);
                changeRepeatMode(newMode);
            }
            break;
            case 1:
                rateIndex_ = constrain(rateIndex_ + amtSlow, -1, kNumArpRates - 1);
                multiplierCalculated_ = false;
                break;
            case 2:
                rateHz_ = constrain(rateHz_ + amtFast, 0, 255);
                rateInHz_ = rateToHz(rateHz_);
                multiplierCalculated_ = false;
                break;
            case 3:
                gate_ = constrain(gate_ + amtFast, 2, 200);
                break;
            }
        }
        break;
        case MFXREPEATPAGE_QUANT:
        {
            switch (param)
            {
            case 0:
                quantizedRateIndex_ = constrain(quantizedRateIndex_ + amtSlow, -2, kNumArpRates - 1);
                quantizeSync_ = quantizedRateIndex_ >= -1; // -2 for off
                break;
            case 1:
                numOfRepeats_ = constrain(numOfRepeats_ + amtSlow, 0, 63);
                break;
            case 2:
                break;
            case 3:
                break;
            }
        }
        break;
        case MFXREPEATPAGE_FADEVEL:
        {
            switch (param)
            {
            case 0:
                fadeVel_ = constrain(fadeVel_ + amtSlow, 0, 1);
                break;
            case 1:
                velStart_ = constrain(velStart_ + amtFast, 0, 100);
                velStartPerc_ = velStart_ / 100.0f;
                break;
            case 2:
                velEnd_ = constrain(velEnd_ + amtFast, 0, 100);
                velEndPerc_ = velEnd_ / 100.0f;
                break;
            case 3:
                break;
            }
        }
        break;
        case MFXREPEATPAGE_FADERATE:
        {
            switch (param)
            {
            case 0:
                fadeRate_ = constrain(fadeRate_ + amtSlow, 0, 1);
                break;
            case 1:
                if (useRateHz())
                {
                    rateStartHz_ = constrain(rateStartHz_ + amtFast, 0, 255);
                }
                else
                {
                    rateStart_ = constrain(rateStart_ + amtSlow, 0, kNumArpRates - 1);
                }
                multiplierCalculated_ = false;
                break;
            case 2:
                if (useRateHz())
                {
                    rateEndHz_ = constrain(rateEndHz_ + amtFast, 0, 255);
                }
                else
                {
                    rateEnd_ = constrain(rateEnd_ + amtSlow, 0, kNumArpRates - 1);
                }
                multiplierCalculated_ = false;
                break;
            case 3:
                break;
            }
        }
        break;
        }

        omxDisp.setDirty();
    }

    void MidiFXRepeat::onDisplayUpdate(uint8_t funcKeyMode)
    {
        omxDisp.clearLegends();

        int8_t page = params_.getSelPage();

        switch (page)
        {
        case MFXREPEATPAGE_CHANCE:
        {
            omxDisp.dispParamBar(chancePerc_, chancePerc_, 0, 100, !getEncoderSelect(), false, "Repeat", "Chance");
        }
        break;
        case MFXREPEATPAGE_MODERATE:
        {
            omxDisp.legends[0] = "MODE";
            omxDisp.legendText[0] = kRepeatModeDisp_[mode_];

            omxDisp.legends[1] = "RATE";
            if (rateIndex_ < 0)
            {
                omxDisp.legendText[1] = "HZ";
            }
            else
            {
                omxDisp.useLegendString[1] = true;
                omxDisp.legendString[1] = "1/" + String(kArpRates[rateIndex_]);
            }

            omxDisp.legends[2] = "RTHZ";
            if (rateInHz_ < 1.0f)
            {
                omxDisp.useLegendString[2] = true;
                omxDisp.legendString[2] = String(rateInHz_, 2);
            }
            else
            {
                omxDisp.legendVals[2] = rateInHz_;
            }

            omxDisp.legends[3] = "Gate";
            omxDisp.legendVals[3] = gate_;

            omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
        }
        break;
        case MFXREPEATPAGE_QUANT:
        {
            omxDisp.legends[0] = "QUANT";

            if (quantizedRateIndex_ <= -2)
            {
                omxDisp.legendText[0] = "OFF";
            }
            else if (quantizedRateIndex_ == -1)
            {
                omxDisp.legendText[0] = "GBL";
            }
            else
            {
                omxDisp.useLegendString[0] = true;
                omxDisp.legendString[0] = "1/" + String(kArpRates[quantizedRateIndex_]);
            }

            omxDisp.legends[1] = "#RPT";
            omxDisp.legendVals[1] = numOfRepeats_ + 1;

            omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
        }
        break;
        case MFXREPEATPAGE_FADEVEL:
        {
            omxDisp.legends[0] = "FVEL";
            omxDisp.legendText[0] = fadeVel_ ? "FADE" : "OFF";
            omxDisp.legends[1] = "STAR";
            omxDisp.legendVals[1] = velStart_;
            omxDisp.legends[2] = "END";
            omxDisp.legendVals[2] = velEnd_;

            omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
        }
        break;
        case MFXREPEATPAGE_FADERATE:
        {
            omxDisp.legends[0] = "FRAT";
            omxDisp.legendText[0] = fadeRate_ ? "FADE" : "OFF";
            omxDisp.legends[1] = "STAR";
            omxDisp.legends[2] = "END";

            if(useRateHz())
            {
                float sHz = rateToHz(rateStartHz_);
                float eHz = rateToHz(rateEndHz_);

                if (sHz < 1.0f)
                {
                    omxDisp.useLegendString[1] = true;
                    omxDisp.legendString[1] = String(sHz, 2);
                }
                else
                {
                    omxDisp.legendVals[1] = sHz;
                }

                if (eHz < 1.0f)
                {
                    omxDisp.useLegendString[2] = true;
                    omxDisp.legendString[2] = String(eHz, 2);
                }
                else
                {
                    omxDisp.legendVals[2] = eHz;
                }
            }
            else
            {
                omxDisp.useLegendString[1] = true;
                omxDisp.legendString[1] = "1/" + String(kArpRates[rateStart_]);

                omxDisp.useLegendString[2] = true;
                omxDisp.legendString[2] = "1/" + String(kArpRates[rateEnd_]);
            }

            omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
        }
        break;
        }
    }

    int MidiFXRepeat::saveToDisk(int startingAddress, Storage *storage)
    {
        RepeatSave save;
        save.chancePerc = chancePerc_;
        save.numOfRepeats = numOfRepeats_;
        save.mode = mode_;
        save.rateIndex = rateIndex_;
        save.rateHz = rateHz_;
        save.quantizedRateIndex_ = quantizedRateIndex_;
        save.gate = gate_;
        save.velStart = velStart_;
        save.velEnd = velEnd_;
        save.fadeVel_ = fadeVel_;
        save.fadeRate_ = fadeRate_;
        save.rateStart_ = rateStart_;
        save.rateEnd_ = rateEnd_;
        save.rateStartHz_ = rateStartHz_;
        save.rateEndHz_ = rateEndHz_;

        int saveSize = sizeof(RepeatSave);

		auto saveBytesPtr = (byte *)(&save);
		for (int j = 0; j < saveSize; j++)
		{
			storage->write(startingAddress + j, *saveBytesPtr++);
		}

		return startingAddress + saveSize;
    }

    int MidiFXRepeat::loadFromDisk(int startingAddress, Storage *storage)
	{
		int saveSize = sizeof(RepeatSave);

		auto save = RepeatSave{};
		auto current = (byte *)&save;
		for (int j = 0; j < saveSize; j++)
		{
			*current = storage->read(startingAddress + j);
			current++;
		}

        chancePerc_ = save.chancePerc;
        numOfRepeats_ = save.numOfRepeats;
        mode_ = save.mode;
        rateHz_ = save.rateHz;
        rateIndex_ = save.rateIndex;
        quantizedRateIndex_ = save.quantizedRateIndex_;
        gate_ = save.gate;
        fadeVel_ = save.fadeVel_;
        fadeRate_ = save.fadeRate_;
        velStart_ = save.velStart;
        velEnd_ = save.velEnd;
        rateStart_ = save.rateStart_;
        rateEnd_ = save.rateEnd_;
        rateStartHz_ = save.rateStartHz_;
        rateEndHz_ = save.rateEndHz_;

        recalcVariables();
        resync();

        return startingAddress + saveSize;
	}

    void MidiFXRepeat::recalcVariables()
    {
        rateInHz_ = rateToHz(rateHz_);

        velStartPerc_ = velStart_ / 100.0f;
        velEndPerc_ = velEnd_ / 100.0f;

        quantizeSync_ = quantizedRateIndex_ >= -1; // -2 for off

        multiplierCalculated_ = false;
        updateMultiplier();
    }
}
