#include "cvNote_util.h"
#include "../config.h"
#include "../consts/consts.h"

const char *cvModeDispNames[] = {
	"LEG",
	"RTRG"
};

CVNoteUtil::CVNoteUtil()
{
}
CVNoteUtil::~CVNoteUtil()
{
}

const char *CVNoteUtil::getTriggerModeDispName()
{
    return cvModeDispNames[triggerMode];
}

bool CVNoteUtil::isNoteValid(uint8_t midiNoteNum)
{
    return midiNoteNum >= cvLowestNote && midiNoteNum < cvHightestNote;
}

uint8_t CVNoteUtil::midi2CVNote(uint8_t noteNumber)
{
    return noteNumber - cvLowestNote;
}
uint8_t CVNoteUtil::cv2MidiNote(uint8_t noteNumber)
{
    return noteNumber + cvLowestNote;
}

void CVNoteUtil::cvNoteOn(uint8_t notenum)
{
    if (isNoteValid(notenum) == false)
        return;

    uint8_t cvNoteNum = midi2CVNote(notenum);

    bool areNotesHeld = cvNotes_.size() > 0;

    if (areNotesHeld)
    {
        // See if note is already in queue and remove it
        auto it = cvNotes_.begin();
        while (it != cvNotes_.end())
        {
            if (it->cvNote == cvNoteNum)
            {
                it = cvNotes_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // if the queue is too large, remove the oldest note at the front
    if (cvNotes_.size() >= trackedSize)
    {
        cvNotes_.erase(cvNotes_.begin());
    }

    CVNoteTracker cvTrackedNote;
    cvTrackedNote.cvNote = cvNoteNum;
    cvNotes_.push_back(cvTrackedNote);

    // How should gate be handled?
    switch (triggerMode)
    {
    case CVTRIGMODE_LEGATO:
    {
        // Keep gate high while notes are played
        setGate(true);
    }
    break;
    case CVTRIGMODE_RETRIG:
    {
        if (areNotesHeld)
        {
            pulseGate = true;
            setGate(false); // Set gate low for period of time
            lastNoteOnTime = sysSettings.timeElasped;
        }
        else
        {
            // turn gate on with first note
            setGate(true);
        }
    }
    break;
    }

    setPitch(cvNoteNum);
    // Send the latest pitch
}

void CVNoteUtil::cvNoteOff(uint8_t notenum)
{
    if (isNoteValid(notenum) == false)
        return;

    uint8_t cvNoteNum = midi2CVNote(notenum);

    bool areNotesHeld = cvNotes_.size() > 0;

    // This should not happen, note on should have been tracked
    if (!areNotesHeld)
    {
        pulseGate = false;
        setGate(false);
        return;
    }

    // See if note is already in queue and remove it
    auto it = cvNotes_.begin();
    while (it != cvNotes_.end())
    {
        if (it->cvNote == cvNoteNum)
        {
            it = cvNotes_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    areNotesHeld = cvNotes_.size() > 0;

    // No more held notes, turn gate off
    if (areNotesHeld == false)
    {
        pulseGate = false;
        setGate(false);
    }
}

void CVNoteUtil::loopUpdate(unsigned long elapsedTime)
{
    // If notes are held and pulseGate is true, turn gate back on after some time
    if(cvNotes_.size() > 0 && pulseGate)
    {
        if(elapsedTime >= lastNoteOnTime + secs2micros * 0.01f)
        {
            pulseGate = false;
            setGate(true);
        }
    }
}

void CVNoteUtil::setGate(bool high)
{
    digitalWrite(CVGATE_PIN, high ? HIGH : LOW);
}

void CVNoteUtil::setPitch(uint8_t cvNoteNum)
{
    cvPitch = static_cast<int>(roundf(cvNoteNum * stepsPerSemitone)); // map (adjnote, 36, 91, 0, 4080);
#if T4
    dac.setVoltage(cvPitch, false);
#else
    analogWrite(CVPITCH_PIN, cvPitch);
#endif
}

CVNoteUtil cvNoteUtil;