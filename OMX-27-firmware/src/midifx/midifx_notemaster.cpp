#include "midifx_notemaster.h"
#include "../hardware/omx_disp.h"

namespace midifx
{
    MidiFXNoteMaster::MidiFXNoteMaster()
    {
        clear();
    }

    MidiFXNoteMaster::~MidiFXNoteMaster()
    {
    }

    void MidiFXNoteMaster::trackNoteInputPassthrough(MidiNoteGroup *note)
    {
        if (note->unknownLength || note->noteOff)
        {
            handleNoteInputPassthrough(note);
            if(note->noteOff)
            {
                handleNoteInput(note);
            }
        }
        else
        {
            // Notes with known lengths don't need to be tracked. 
            sendNoteOut(note);
        }
    }

    void MidiFXNoteMaster::trackNoteInput(MidiNoteGroup *note)
    {
        if (note->unknownLength || note->noteOff)
        {
            // only notes of unknown lengths need to be tracked
            // notes with fixed lengths will turn off automatically.
            if(note->noteOff)
            {
                // Whole purpose of this tracking madness is to pass note offs through the chain
                // that were previously passed through
                handleNoteInputPassthrough(note);
            }
            handleNoteInput(note);
        }
        else
        {
            processNoteInput(note);
        }
    }

    // bool MidiFXNoteMaster::findEmptySlot(MidiNoteGroup *trackingArray, uint8_t size, uint8_t *emptyIndex)
    // {
    //     for (uint8_t i = 0; i < size; i++)
    //     {
    //         // Found empty slot
    //         if (trackingArray[i].prevNoteNumber == kEmptyIndex)
    //         {
    //             *emptyIndex = i;
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    void MidiFXNoteMaster::removeFromTracking(MidiNoteGroup *note)
    {
        for (uint8_t i = 0; i < kTrackingSize; i++)
        {
            if (trackingNoteGroups[i].prevNoteNumber != kEmptyIndex)
            {
                if (trackingNoteGroups[i].channel == note->channel && trackingNoteGroups[i].prevNoteNumber == note->prevNoteNumber)
                {
                    trackingNoteGroups[i].prevNoteNumber = kEmptyIndex; // mark empty
                }
            }
        }
    }

    void MidiFXNoteMaster::clear()
    {
        for (uint8_t i = 0; i < kTrackingSize; i++)
        {
            trackingNoteGroups[i].prevNoteNumber = kEmptyIndex;
            trackingNoteGroupsPassthrough[i].prevNoteNumber = kEmptyIndex;
        }
    }

    // If chance is less than 100% and passing through, notes need to be tracked
    // and if the same note comes in without passthrough for a noteoff event, it needs to 
    // be passed through the effect to send noteoff to prevent stuck notes
    void MidiFXNoteMaster::handleNoteInputPassthrough(MidiNoteGroup *note)
    {
        // Note off
        if (note->noteOff)
        {
            // Search to see if this note is in trackingNoteGroupsPassthrough
            // If a note is found, it means the note previously had a note on that passed
            // through and the note off needs to be sent through or else notes could be stuck on. 
            // PrevNoteNumber should be the origin note number before being modified by MidiFX
            for (uint8_t i = 0; i < kTrackingSize; i++)
            {
                if (trackingNoteGroupsPassthrough[i].prevNoteNumber != kEmptyIndex)
                {
                    if (trackingNoteGroupsPassthrough[i].channel == note->channel && trackingNoteGroupsPassthrough[i].prevNoteNumber == note->prevNoteNumber)
                    {
                        note->noteNumber = trackingNoteGroupsPassthrough[i].noteNumber;
                        sendNoteOut(note);
                        trackingNoteGroupsPassthrough[i].prevNoteNumber = kEmptyIndex; // mark empty
                    }
                }
            }
        }
        // Note on
        else
        {
            // Search for an empty slot in trackingNoteGroupsPassthrough
            // If no slots are available/more than 8 notes/ note gets killed. 
            for (uint8_t i = 0; i < kTrackingSize; i++)
            {
                // Found empty slot
                if (trackingNoteGroupsPassthrough[i].prevNoteNumber == kEmptyIndex)
                {
                    trackingNoteGroupsPassthrough[i].channel = note->channel;
                    trackingNoteGroupsPassthrough[i].prevNoteNumber = note->prevNoteNumber;
                    trackingNoteGroupsPassthrough[i].noteNumber = note->noteNumber;

                    // Send it forward through chain
                    sendNoteOut(note);
                    return;
                }
            }
        }
    }

    void MidiFXNoteMaster::handleNoteInput(MidiNoteGroup *note)
    {
        // Same implementation with more comments in submode_midifx
        // Keeps track of previous note ons and and adjusts note number
        // for note offs using the prevNoteNumber parameter.
        // Why is this necessary?
        // If the note is modified by midifx like randomize before the arp
        // Then the arp can end up having notes stuck on
        // This ensures that notes don't get stuck on.
        if (note->noteOff)
        {
            bool noteFound = false;

            for (uint8_t i = 0; i < kTrackingSize; i++)
            {
                if (trackingNoteGroups[i].prevNoteNumber != kEmptyIndex)
                {
                    if (trackingNoteGroups[i].channel == note->channel && trackingNoteGroups[i].prevNoteNumber == note->prevNoteNumber)
                    {
                        // Serial.println("trackNoteInput note off found in trackingNoteGroups");
                        note->noteNumber = trackingNoteGroups[i].noteNumber;
                        processNoteInput(note);
                        trackingNoteGroups[i].prevNoteNumber = kEmptyIndex; // mark empty
                        noteFound = true;
                    }
                }
            }

            if (!noteFound)
            {
                // Serial.println("trackNoteInput note off not found in trackingNoteGroups");

                processNoteInput(note); // Not sure we need to process note if not found
            }
        }
        else // Note on
        {
            for (uint8_t i = 0; i < kTrackingSize; i++)
            {
                // Find empty slot
                if (trackingNoteGroups[i].prevNoteNumber == kEmptyIndex)
                {
                    trackingNoteGroups[i].channel = note->channel;
                    trackingNoteGroups[i].prevNoteNumber = note->prevNoteNumber;
                    trackingNoteGroups[i].noteNumber = note->noteNumber;

                    processNoteInput(note);
                    return;
                }
            }
        }
    }

    void MidiFXNoteMaster::setContext(void *context)
    {
        outFunctionContext_ = context;
    }
    void MidiFXNoteMaster::setProcessNoteFptr(void (*fptr)(void *, MidiNoteGroup *))
    {
        processNoteFptr = fptr;
    }
    void MidiFXNoteMaster::setSendNoteOutFptr(void (*fptr)(void *, MidiNoteGroup *))
    {
        sendNoteOutFptr = fptr;
    }

    void MidiFXNoteMaster::processNoteInput(MidiNoteGroup *note)
    {
        if (outFunctionContext_ != nullptr)
        {
            processNoteFptr(outFunctionContext_, note);
        }
    }

    void MidiFXNoteMaster::sendNoteOut(MidiNoteGroup *note)
    {
        if (outFunctionContext_ != nullptr)
        {
            sendNoteOutFptr(outFunctionContext_, note);
        }
    }
}