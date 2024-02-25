#include "midifx_repeat.h"
#include "../hardware/omx_disp.h"
#include "../utils/omx_util.h"

namespace midifx
{
    Micros nextRepeatTriggerTime_ = 0;

    enum MidiFXRepeatModes
    {
        MFXREPEATMODE_OFF,
        MFXREPEATMODE_1SHOT,
        MFXREPEATMODE_ON,
        MFXREPEATMODE_HOLD,
        MFXREPEATMODE_ONCE,
        MFXREPEATMODE_COUNT
    };

	MidiFXRepeat::MidiFXRepeat()
	{
		params_.addPage(4);
	
        chancePerc_ = 100;

        numOfRepeats_ = 4;
        rateIndex_ = 9;
        rateHz_ = 40;
        gate_ = 90;
        velStart_ = 10;
        velEnd_ = 115;

        quantizeSync_ = true;

        changeRepeatMode(MFXREPEATMODE_ON);

		encoderSelect_ = true;
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
        clone->rateHz_ = rateHz_;
        clone->gate_ = gate_;
        clone->velStart_ = velStart_;
        clone->velEnd_ = velEnd_;

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
            Serial.println("MFXREPEATMODE_OFF");
            sendNoteOut(note);
            return;
        }

        if (note.unknownLength)
        {
            if (note.noteOff)
            {
                repeatNoteOff(&note);
            }
            else
            {
                repeatNoteOn(&note);
            }
        }
        // else
        // {
        //     bool canInsert = true;

        //     if (pendingNotes.size() < queueSize)
        //     {
        //         for (uint8_t i = 0; i < pendingNotes.size(); i++)
        //         {
        //             PendingArpNote p = pendingNotes[i];

        //             // Note already exists
        //             if (p.noteCache.noteNumber == note.noteNumber && p.noteCache.channel == note.channel)
        //             {
        //                 // Update note off time
        //                 pendingNotes[i].offTime = seqConfig.currentFrameMicros + (note.stepLength * clockConfig.step_micros);
        //                 canInsert = false;
        //                 break;
        //             }
        //         }
        //     }
        //     else
        //     {
        //         canInsert = false;
        //     }

        //     if (canInsert)
        //     {
        //         // Serial.println("Inserting pending note");
        //         PendingArpNote pendingNote;
        //         pendingNote.noteCache.setFromNoteGroup(note);
        //         pendingNote.offTime = seqConfig.currentFrameMicros + (note.stepLength * clockConfig.step_micros);
        //         pendingNotes.push_back(pendingNote);
        //         arpNoteOn(note);
        //     }
        //     else
        //     {
        //         // Remove from tracking notes
        //         for (uint8_t i = 0; i < 8; i++)
        //         {
        //             if (trackingNoteGroups[i].prevNoteNumber != 255)
        //             {
        //                 if (trackingNoteGroups[i].channel == note.channel && trackingNoteGroups[i].prevNoteNumber == note.prevNoteNumber)
        //                 {
        //                     trackingNoteGroups[i].prevNoteNumber = 255; // mark empty
        //                 }
        //             }
        //         }
        //         sendNoteOut(note);
        //     }
        // }
    }

    bool MidiFXRepeat::hasMidiNotes()
    {
        return activeNoteQueue.size() > 0;
    }

    void MidiFXRepeat::updateMultiplier()
    {
        if(!multiplierCalculated_)
        {
            uint8_t rate = kArpRates[rateIndex_]; // 8
            multiplier_ = 1.0f / (float)rate; // 1 / 8 = 0.125 // Only need to recalculate this if rate changes yo

            multiplierCalculated_ = true;
        }
    }

    bool MidiFXRepeat::insertMidiNoteQueue(MidiNoteGroup *note)
    {
        if (activeNoteQueue.capacity() > queueSize)
        {
            activeNoteQueue.shrink_to_fit();
        }

        bool noteAdded = false;

        if (activeNoteQueue.size() < queueSize)
        {
            auto newNote = RepeatNote(note);

            // If quantizeSync_is true, this note won't start triggering until the next clock tick. 
            if(quantizeSync_)
            {
                newNote.playing = true;
                auto delta = seqConfig.currentFrameMicros - last16thTime_;

                if(delta < (clockConfig.step_micros / 3.0f))
                {
                    newNote.nextTriggerTime = last16thTime_;
                }
                else
                {
                    newNote.nextTriggerTime = next16thTime_;
                }

                // newNote.nextTriggerTime = next16thTime_;
                // updateMultiplier();
                // newNote.nextTriggerTime = seqConfig.lastClockMicros + (clockConfig.step_micros * 16 * multiplier_);
            }
            else
            {
                // Trigger note asap
                newNote.playing = true;
                newNote.nextTriggerTime = seqConfig.currentFrameMicros;
            }

            activeNoteQueue.push_back(newNote);
            noteAdded = true;
        }

        Serial.println("Note Added: " + String(noteAdded));
        return noteAdded;

        // // Serial.println("playedNoteQueue capacity: " + String(playedNoteQueue.capacity()));
        // if (playedNoteQueue.capacity() > queueSize)
        // {
        //     playedNoteQueue.shrink_to_fit();
        // }
        // if (holdNoteQueue.capacity() > queueSize)
        // {
        //     holdNoteQueue.shrink_to_fit();
        // }

        // bool noteAdded = false;

        // if (playedNoteQueue.size() < queueSize)
        // {
        //     playedNoteQueue.push_back(RepeatNote(note));
        //     noteAdded = true;
        // }

        // if (holdNoteQueue.size() < queueSize)
        // {
        //     holdNoteQueue.push_back(RepeatNote(note));
        //     noteAdded = true;
        // }

        // Serial.println("Note Added: " + String(noteAdded));
        // return noteAdded;
    }

    bool MidiFXRepeat::removeMidiNoteQueue(MidiNoteGroup *note)
    {
        bool foundNoteToRemove = false;
        auto it = activeNoteQueue.begin();
        while (it != activeNoteQueue.end())
        {
            // Serial.println("playedNoteQueue: note " + String(it->noteNumber));
            // Serial.println("MidiNoteGroup: note " + String(note->noteNumber));
            // remove matching note numbers
            // if (it->noteNumber == note->noteNumber && it->channel == note->channel - 1)
            if (it->noteNumber == note->noteNumber)
            {
                Serial.println("removing note: " + String(it->noteNumber));
                // `erase()` invalidates the iterator, use returned iterator
                it = activeNoteQueue.erase(it);
                foundNoteToRemove = true;
            }
            else
            {
                ++it;
            }
        }

        // Serial.println("foundNoteToRemove " + String(foundNoteToRemove));

        return foundNoteToRemove;

        // bool foundNoteToRemove = false;
        // auto it = playedNoteQueue.begin();
        // while (it != playedNoteQueue.end())
        // {
        //     Serial.println("playedNoteQueue: note " + String(it->noteNumber));
        //     Serial.println("MidiNoteGroup: note " + String(note->noteNumber));
        //     // remove matching note numbers
        //     // if (it->noteNumber == note->noteNumber && it->channel == note->channel - 1)
        //     if (it->noteNumber == note->noteNumber)
        //     {
        //         // `erase()` invalidates the iterator, use returned iterator
        //         it = playedNoteQueue.erase(it);
        //         foundNoteToRemove = true;
        //     }
        //     else
        //     {
        //         ++it;
        //     }
        // }

        // Serial.println("foundNoteToRemove " + String(foundNoteToRemove));

        // return foundNoteToRemove;
    }

    void MidiFXRepeat::changeRepeatMode(uint8_t newMode)
    {
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
    }

    void MidiFXRepeat::resync()
    {
        playedNoteQueue.clear();
        holdNoteQueue.clear();
        tempNoteQueue.clear();
        activeNoteQueue.clear();

        resetArpSeq();

        for (uint8_t i = 0; i < 8; i++)
        {
            trackingNoteGroups[i].prevNoteNumber = 255;
        }
    }

    void MidiFXRepeat::repeatNoteOn(MidiNoteGroup *note)
    {
        Serial.println("repeatNoteOn");

        bool seqReset = false;

        if (!seqRunning_)
        {
            startSeq();
            resetArpSeq();
            seqReset = true;
        }

        if (hasMidiNotes() == false)
        {
            // velocity_ = note.velocity;
            // sendMidi_ = note.sendMidi;
            // sendCV_ = note.sendCV;
            // midiChannel_ = note.channel - 1; // note.channel is 1-16, sub 1 for 0-15


            resetArpSeq();
            seqReset = true;

            holdNoteQueue.clear();
        }
        else
        {
            // if (resetMode_ == ARPRESET_NOTE)
            // {
            //     resetArpSeq();
            //     seqReset = true;
            // }
        }

        insertMidiNoteQueue(note);
        // sortNotes();

        if (seqReset)
        {
            // nextNotePos_ = notePos_;
            // prevQLength_ = sortedNoteQueue.size();
        }

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
        Serial.println("repeatNoteOff");
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
        Serial.println("startArp");
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

        if (omxUtil.areClocksRunning() == false)
        {
            omxUtil.restartClocks();
            omxUtil.startClocks();
            uint8_t rate = kArpRates[rateIndex_];
            multiplier_ = 1.0f / (float)rate;
            stepMicroDelta_ = (clockConfig.step_micros * 16) * multiplier_;
            nextStepTimeP_ = seqConfig.lastClockMicros; // Should be current time, start now.
            nextRepeatTriggerTime_ = nextStepTimeP_;

            next16thTime_ = seqConfig.lastClockMicros;
            last16thTime_ = seqConfig.lastClockMicros;
        }
        else
        {
            nextStepTimeP_ = nextRepeatTriggerTime_;
        }

        lastStepTimeP_ = nextStepTimeP_;

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
            if (seqConfig.numOfActiveArps <= 0)
            {
                omxUtil.stopClocks();
            }
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

        playNote(seqConfig.currentFrameMicros, note.noteNumber, note.velocity, note.channel);
    }

    void MidiFXRepeat::repeatNoteTrigger()
    {
        Serial.println("repeatNoteTrigger");

        if (activeNoteQueue.size() == 0)
        {
            Serial.println("no sorted notes");

            return;
        }

        uint32_t noteon_micros = seqConfig.currentFrameMicros;

        // if (resetNextTrigger_)
        // {
        //     resetArpSeq();
        // }

        // bool incrementOctave = false;
        // int currentNotePos = notePos_;
        // int nextNotePos = notePos_;
        // int qLength = sortedNoteQueue.size();

        // prevNotePos_ = notePos_;
        
        // prevQLength_ = qLength;

        // syncPos_ = syncPos_ + 1 % 16;

        // currentNotePos = constrain(currentNotePos, 0, qLength - 1);

        for (RepeatNote r : activeNoteQueue)
        {
            if (r.noteNumber >= 0 && r.noteNumber <= 127)
            {
                playNote(noteon_micros, r.noteNumber, r.velocity, r.channel);
            }
        }

        // ArpNote arpNote = sortedNoteQueue[currentNotePos];
        // randPrevNote_ = arpNote.noteNumber;

        // int16_t noteNumber = arpNote.noteNumber;

        // noteNumber = applyModPattern(noteNumber, arpNote.channel);
        // stepLength_ = findStepLength(); // Can be changed by ties in mod pattern

        // if (noteNumber != -127)
        // {
        //     noteNumber = applyTranspPattern(noteNumber);

        //     // Add octave
        //     noteNumber = noteNumber + (octavePos_ * octDistance_);
        //     playNote(noteon_micros, noteNumber, velocity_, arpNote.channel);
        // }

        // bool seqReset = false;

        // // Advance mod pattern
        // modPos_++;
        // if (modPos_ >= modPatternLength_ + 1)
        // {
        //     if (resetMode_ == ARPRESET_MODPAT)
        //     {
        //         resetArpSeq();
        //         seqReset = true;
        //     }
        //     modPos_ = 0;
        // }

        // // Advance transpose pattern
        // transpPos_++;
        // if (transpPos_ >= transpPatternLength_ + 1)
        // {
        //     if (resetMode_ == ARPRESET_TRANSPOSEPAT)
        //     {
        //         resetArpSeq();
        //         seqReset = true;
        //     }
        //     transpPos_ = 0;
        // }
        
        // if (!seqReset)
        // {
        //     notePos_ = nextNotePos;

        //     nextNotePos_ = (notePos_ + qLength) % qLength;
        // }
        // else
        // {
        //     nextNotePos_ = notePos_;
        // }

        // prevSortedNoteQueue.clear();

        // for (ArpNote a : sortedNoteQueue)
        // {
        //     prevSortedNoteQueue.push_back(a);
        // }

        // playNote(noteon_micros, notePat_[patPos_]);

        // patPos_++;
    }

    void MidiFXRepeat::playNote(uint32_t noteOnMicros, int16_t noteNumber, uint8_t velocity, uint8_t channel)
    {
        Serial.println("SeqRunning: " + String(seqRunning_));
        Serial.println("PlayNote: " + String(noteNumber) + String(velocity) + String(velocity));
        if (noteNumber < 0 || noteNumber > 127)
            return;

        MidiNoteGroup noteOut;

        noteOut.channel = channel + 1;
        noteOut.noteNumber = (uint8_t)noteNumber;
        noteOut.prevNoteNumber = (uint8_t)noteNumber;
        noteOut.velocity = velocity;
        noteOut.stepLength = ((float)gate_ * 0.01f) * (16.0f * multiplier_) * (float)stepLength_;
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
        // for(uint8_t i = 0; i < activeNoteQueue.size(); i++)
        // {
        //     if(activeNoteQueue[i].playing == false)
        //     {
        //         updateMultiplier();
        //         activeNoteQueue[i].playing = true;
        //         activeNoteQueue[i].nextTriggerTime = seqConfig.lastClockMicros;
        //     }
        // }
    }

    void MidiFXRepeat::loopUpdate()
	{
        // auto now = seqConfig.currentFrameMicros;

        // Send arp offs for notes that had fixed lengths
        // auto it = pendingNotes.begin();
        // while (it != pendingNotes.end())
        // {
        //     // remove matching note numbers
        //     if (it->offTime <= now)
        //     {

        //         // Serial.println("Removing pending note");
        //         arpNoteOff(it->noteCache.toMidiNoteGroup());
        //         // `erase()` invalidates the iterator, use returned iterator
        //         it = pendingNotes.erase(it);
        //     }
        //     else
        //     {
        //         ++it;
        //     }
        // }

        // if (patternDirty_)
        // {
        //     regeneratePattern();
        //     patternDirty_ = false;
        // }

        // if (pendingStart_ && !omxUtil.areClocksRunning() && micros() - pendingStartTime_ >= 15000)
        // {
        //     omxUtil.resetClocks();
        //     doPendingStart();
        // }

        if (!seqRunning_)
        {
            return;
        }

        if (sysSettings.omxMode == MODE_MIDI && !selected_)
        {
            Serial.println("Not selected");
            return;
        }

        updateMultiplier();

        uint32_t stepmicros = seqConfig.currentFrameMicros;

        if(stepmicros >= next16thTime_)
        {
            last16thTime_ = next16thTime_;
            next16thTime_ = next16thTime_ + (clockConfig.step_micros * 16 * (1.0f / 16.0f));

            // for (uint8_t i = 0; i < activeNoteQueue.size(); i++)
            // {
            //     if (activeNoteQueue[i].playing == false)
            //     {
            //         activeNoteQueue[i].playing = true;
            //         activeNoteQueue[i].nextTriggerTime = next16thTime_;
            //     }
            // }

            // next16thTime_ = next16thTime_ + (clockConfig.step_micros * 16 * (1.0f / 16.0f));
        }

        tempNoteQueue.clear();

        // Loop through all the notes and see which notes should be triggered this frame
        // if a note should be triggered it gets added to the tempNoteQueue
        for(uint8_t i = 0; i < activeNoteQueue.size(); i++)
        {
            // The time has come to
            if(stepmicros >= activeNoteQueue[i].nextTriggerTime && activeNoteQueue[i].playing)
            {
                activeNoteQueue[i].nextTriggerTime = activeNoteQueue[i].nextTriggerTime + (clockConfig.step_micros * 16 * multiplier_);

                // sync with the clock
                // if (quantizeSync_)
                // {
                //     activeNoteQueue[i].nextTriggerTime = seqConfig.lastClockMicros + (clockConfig.step_micros * 16 * multiplier_);
                // }
                // else
                // {
                //     activeNoteQueue[i].nextTriggerTime = seqConfig.currentFrameMicros + (clockConfig.step_micros * 16 * multiplier_);
                // }

                // Don't hold back
                tempNoteQueue.push_back(activeNoteQueue[i]);
            }
        }

        // Trigger any notes that should be triggered this frame
        for (RepeatNote n : tempNoteQueue)
        {
            triggerNote(n);
        }

        // if (stepmicros >= nextStepTimeP_)
        // {
        //     lastStepTimeP_ = nextStepTimeP_;

        //     uint8_t rate = kArpRates[rateIndex_]; // 8
        //     multiplier_ = 1.0f / (float)rate; // 1 / 8 = 0.125 // Only need to recalculate this if rate changes yo

        //     // clockConfig.step_micros = 16th note step in microseconds

        //     stepMicroDelta_ = (clockConfig.step_micros * 16) * multiplier_;

        //     nextStepTimeP_ = seqConfig.currentFrameMicros + stepMicroDelta_; // calc step based on rate

        //     nextRepeatTriggerTime_ = nextStepTimeP_;

        //     repeatNoteTrigger();

        //     // Keeps arp running for a bit on stop so if you play new notes they will be in sync
        //     // if (pendingStop_)
        //     // {
        //     //     pendingStopCount_--;
        //     //     if (pendingStopCount_ == 0)
        //     //     {
        //     //         doPendingStop();
        //     //     }
        //     // }
        // }
	}

	void MidiFXRepeat::onEncoderChangedEditParam(Encoder::Update enc)
	{
		int8_t page = params_.getSelPage();
		int8_t param = params_.getSelParam();

		auto amt = enc.accel(1);

		// if (page == 0)
		// {
		// 	if (param == 0)
		// 	{
		// 		mode_ = constrain(mode_ + amt, 0, MFXSELMODE_COUNT - 1);
		// 	}
		// 	else if (param == 1)
		// 	{
		// 		uint8_t oldLength = length_;
		// 		length_ = constrain(length_ + amt, 0, 7);
		// 		lengthChanged_ = oldLength != length_;
		// 	}
		// 	else if (param == 3)
		// 	{
		// 		chancePerc_ = constrain(chancePerc_ + amt, 0, 100);
		// 	}
		// }

		omxDisp.setDirty();
	}

	void MidiFXRepeat::onDisplayUpdate(uint8_t funcKeyMode)
	{
		omxDisp.clearLegends();

		int8_t page = params_.getSelPage();

		// switch (page)
		// {
		// case 0:
		// {
		// 	omxDisp.legends[0] = "MODE";
		// 	omxDisp.legends[1] = "LEN";
		// 	omxDisp.legends[3] = "CHC%";
		// 	omxDisp.legendText[0] = modeLabels[mode_];
		// 	omxDisp.legendVals[1] = length_;
		// 	omxDisp.useLegendString[3] = true;
		// 	omxDisp.legendString[3] = String(chancePerc_) + "%";
		// }
		// break;
		// default:
		// 	break;
		// }

		// omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
	}

	int MidiFXRepeat::saveToDisk(int startingAddress, Storage *storage)
	{
        RepeatSave save;
        save.chancePerc = chancePerc_;
        save.numOfRepeats = numOfRepeats_;
        save.mode = mode_;
        save.rateIndex = rateIndex_;
        save.rateHz = rateHz_;
        save.gate = gate_;
        save.velStart = velStart_;
        save.velEnd = velEnd_;

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
        gate_ = save.gate;
        velStart_ = save.velStart;
        velEnd_ = save.velEnd;

        return startingAddress + saveSize;
	}
}
