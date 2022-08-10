#include "midifx_interface.h"
#include "omx_disp.h"

namespace midifx
{
    MidiFXInterface::~MidiFXInterface()
    {
        // std::vector<MidiNoteGroup>().swap(triggeredNotes);
        // Serial.println("Deleted vector");
    }

    void MidiFXInterface::setEnabled(bool newEnabled)
    {
        enabled_ = newEnabled;
        if (enabled_)
        {
            onEnabled();
        }
        else
        {
            onDisabled();
        }
    }

    bool MidiFXInterface::getEnabled()
    {
        return enabled_;
    }

    void MidiFXInterface::onEncoderChanged(Encoder::Update enc)
    {
        if (encoderSelect_)
        {
            onEncoderChangedSelectParam(enc);
        }
        else
        {
            onEncoderChangedEditParam(enc);
        }
    }

    // Handles selecting params using encoder
    void MidiFXInterface::onEncoderChangedSelectParam(Encoder::Update enc)
    {
        params_.changeParam(enc.dir());
        omxDisp.setDirty();
    }

    void MidiFXInterface::onEncoderButtonDown()
    {
        encoderSelect_ = !encoderSelect_;
        omxDisp.setDirty();
    }

    void MidiFXInterface::processNoteOff(MidiNoteGroup note)
    {
        // // See if note was previously effected
        // // Adjust note number if it was and remove from vector
        // for (size_t i = 0; i < triggeredNotes.size(); i++)
        // {
        //     if (triggeredNotes[i].prevNoteNumber == note.noteNumber)
        //     {
        //         note.noteNumber = triggeredNotes[i].noteNumber;
        //         triggeredNotes.erase(triggeredNotes.begin() + i);
        //         // Serial.println("Found previous triggered note");
        //         break;
        //     }
        // }

        // Serial.println("TriggeredNotesSize: " + String(triggeredNotes.size()));

        sendNoteOut(note);
    }

    void MidiFXInterface::processNoteOn(uint8_t origNoteNumber, MidiNoteGroup note)
    {
        // From a keyboard source, length is 0
        // if(note.stepLength == 0)
        // {
        //     note.prevNoteNumber = origNoteNumber;

        //     bool alreadyExists = false;
        //     // See if orig note alread exists
        //     for (size_t i = 0; i < triggeredNotes.size(); i++)
        //     {
        //         if (triggeredNotes[i].prevNoteNumber == origNoteNumber)
        //         {
        //             triggeredNotes[i] = note;
        //             alreadyExists = true;
        //             // Serial.println("Orig note already existed");
        //             break;
        //         }
        //     }
            
        //     if (!alreadyExists)
        //     {
        //         triggeredNotes.push_back(note);
        //     }
        // }
    }

    void MidiFXInterface::setNoteOutput(void (*fptr)(void *, MidiNoteGroup), void *context)
    {
        outFunctionContext_ = context;
        outFunctionPtr_ = fptr;
    }

    void MidiFXInterface::sendNoteOut(MidiNoteGroup note)
    {
        if(outFunctionContext_ != nullptr){
            outFunctionPtr_(outFunctionContext_, note);
        }
    }

    void MidiFXInterface::sendNoteOff(MidiNoteGroupCache noteCache)
    {
        // Serial.println("Note off from cache: " + String(noteCache.noteNumber));

        sendNoteOff(noteCache.toMidiNoteGroup());
    }

    void MidiFXInterface::sendNoteOff(MidiNoteGroup note)
    {
        // Serial.println("Note off: " + String(note.noteNumber));

        note.velocity = 0;
        note.noteOff = true;

        if(outFunctionContext_ != nullptr){
            // Serial.println("Note off sent");
            outFunctionPtr_(outFunctionContext_, note);
        }
    }
}