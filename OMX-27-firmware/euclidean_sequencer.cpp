#include "euclidean_sequencer.h"
#include "MM.h"
#include "noteoffs.h"

namespace euclidean
{
    // const float kEuclidNoteLengths[] = {0.1, 0.25, 0.5, 0.75, 1, 1.5, 2, 4, 8, 16};
    // const uint8_t kNumEuclidNoteLengths = 10;

    EuclideanMath::EuclideanMath()
    {
    }

    // bool array should be of length kPatternSize
    void EuclideanMath::generateEuclidPattern(bool *pattern, uint8_t events, uint8_t steps)
    {
        clearPattern(pattern);

        // a value of true for each array element indicates a pulse

        uint8_t bucket = 0; // out variable to add pulses together for each step

        // fill array with rhythm
        for (uint8_t i = 0; i < steps; i++)
        {
            bucket += events;
            if (bucket >= steps)
            {
                bucket -= steps;
                pattern[i] = true;
            }
        }

        flipPattern(pattern, steps);
        //   rotatePattern(pattern, steps, rotation);
    }
    // bool array should be of length kPatternSize
    void EuclideanMath::clearPattern(bool *pattern)
    {
        for (int i = 0; i < kPatternSize; i++)
        {
            pattern[i] = false;
        }
    }
    // bool array should be of length kPatternSize
    void EuclideanMath::flipPattern(bool *pattern, uint8_t steps)
    {
        bool temp[steps];

        for (int i = 0; i < steps; i++)
        {
            temp[i] = pattern[steps - 1 - i];
        }

        for (int i = 0; i < steps; i++)
        {
            pattern[i] = temp[i];
        }
    }
    // bool array should be of length kPatternSize
    void EuclideanMath::rotatePattern(bool *pattern, uint8_t steps, uint8_t rotation)
    {
        bool temp[steps];

        uint8_t val = steps - rotation;

        for (uint8_t i = 0; i < steps; i++)
        {
            temp[i] = pattern[abs((i + val) % steps)];
        }
        for (int i = 0; i < steps; i++)
        {
            pattern[i] = temp[i];
        }
    }

    EuclideanSequencer::EuclideanSequencer()
    {
        for (uint8_t i = 0; i < EuclideanMath::kPatternSize; i++)
        {
            pattern_[i] = false;
        }

        regeneratePattern();
        tickCount_ = 0;

        divider_ = 0;
        multiplier_ = 1;
        running_ = false;
    }

    void EuclideanSequencer::regeneratePattern()
    {
        EuclideanMath::generateEuclidPattern(pattern_, events_, steps_);
        EuclideanMath::rotatePattern(pattern_, steps_, rotation_);

        // printEuclidPattern();
    }

    uint32_t EuclideanSequencer::randomValue(uint32_t init)
    {
        uint32_t val = 0x12345;
        if (init)
        {
            val = init;
            return 0;
        }
        val = val * 214013 + 2531011;
        return val;
    }

    void EuclideanSequencer::start()
    {
        tickCount_ = 0;
        seqPos_ = 0;
        running_ = true;

        nextStepTimeP_ = seqConfig.currentFrameMicros;
        lastStepTimeP_ = seqConfig.currentFrameMicros;
        startMicros = seqConfig.currentFrameMicros;
    }

    void EuclideanSequencer::stop()
    {
        running_ = false;
        pendingNoteOffs.allOff();
    }

    void EuclideanSequencer::proceed()
    {
        running_ = true;
    }

    bool EuclideanSequencer::isDirty()
    {
        return patternDirty_;
    }

    bool EuclideanSequencer::isRunning()
    {
        return running_;
    }

    void EuclideanSequencer::setNoteOutputFunc(void (*fptr)(void *, uint8_t, MidiNoteGroup), void *context, u_int8_t euclidIndex)
    {
        onNoteOnFuncPtr_ = fptr;
        onNoteOnFuncPtrContext_ = context;
        euclidIndex_ = euclidIndex;
    }

    void EuclideanSequencer::onNoteOn(uint8_t channel, uint8_t noteNumber, uint8_t velocity, float stepLength, bool sendMidi, bool sendCV, uint32_t noteOnMicros)
    {
        if (onNoteOnFuncPtrContext_ == nullptr)
            return;

        MidiNoteGroup noteGroup;
        noteGroup.channel = channel;
        noteGroup.noteNumber = noteNumber;
        noteGroup.velocity = velocity;
        noteGroup.stepLength = stepLength;
        noteGroup.sendMidi = sendMidi;
        noteGroup.sendCV = sendCV;
        noteGroup.noteonMicros = noteOnMicros;

        onNoteOnFuncPtr_(onNoteOnFuncPtrContext_, euclidIndex_, noteGroup);
    }

    void EuclideanSequencer::setClockDivMult(uint8_t m)
    {
        uint8_t prevDiv = clockDivMultP_;

        clockDivMultP_ = m;
        multiplier_ = multValues[m];

        if (clockDivMultP_ != prevDiv)
        {
            // Serial.println((String)"clockDivMultP_: " + clockDivMultP_);
            patternDirty_ = true;
        }
    }

    uint8_t EuclideanSequencer::getClockDivMult()
    {
        return clockDivMultP_;
    }

    void EuclideanSequencer::setPolyRClockDivMult(uint8_t m)
    {
        uint8_t prevDiv = polyRClockDivMultP_;

        polyRClockDivMultP_ = m;
        multiplierPR_ = multValues[m];

        if (polyRClockDivMultP_ != prevDiv)
        {
            patternDirty_ = true;
        }
    }
    uint8_t EuclideanSequencer::getPolyRClockDivMult()
    {
        return polyRClockDivMultP_;
    }

    void EuclideanSequencer::setRotation(uint8_t newRotation)
    {
        if (newRotation != rotation_)
            patternDirty_ = true;
        rotation_ = newRotation;
    }
    uint8_t EuclideanSequencer::getRotation()
    {
        return rotation_;
    }
    void EuclideanSequencer::setEvents(uint8_t newEvents)
    {
        if (newEvents != events_)
            patternDirty_ = true;
        events_ = newEvents;
    }
    uint8_t EuclideanSequencer::getEvents()
    {
        return events_;
    }

    void EuclideanSequencer::setSteps(uint8_t newSteps)
    {
        if (newSteps != steps_)
            patternDirty_ = true;
        steps_ = newSteps;
    }
    uint8_t EuclideanSequencer::getSteps()
    {
        return steps_;
    }
    void EuclideanSequencer::setNoteNumber(uint8_t newNoteNumber)
    {
        noteNumber_ = newNoteNumber;
    }
    uint8_t EuclideanSequencer::getNoteNumber()
    {
        return noteNumber_;
    }
    void EuclideanSequencer::setMidiChannel(uint8_t newMidiChannel)
    {
        midiChannel_ = newMidiChannel;
    }
    uint8_t EuclideanSequencer::getMidiChannel()
    {
        return midiChannel_;
    }

    void EuclideanSequencer::setVelocity(uint8_t newVelocity)
    {
        velocity_ = newVelocity;
    }
    uint8_t EuclideanSequencer::getVelocity()
    {
        return velocity_;
    }

    void EuclideanSequencer::setSwing(uint8_t newSwing)
    {
        swing_ = newSwing;
    }
    uint8_t EuclideanSequencer::getSwing()
    {
        return swing_;
    }

    void EuclideanSequencer::setNoteLength(uint8_t newNoteLength)
    {
        noteLength_ = newNoteLength;
    }
    uint8_t EuclideanSequencer::getNoteLength()
    {
        return noteLength_;
    }

    void EuclideanSequencer::setPolyRhythmMode(bool enable)
    {
        polyRhythmMode_ = enable;
    }
    bool EuclideanSequencer::getPolyRhythmMode()
    {
        return polyRhythmMode_;
    }

    uint8_t EuclideanSequencer::getSeqPos()
    {
        return seqPos_;
    }
    uint8_t EuclideanSequencer::getLastSeqPos()
    {
        return lastSeqPos_;
    }

    float EuclideanSequencer::getSeqPerc()
    {
        return seqPerc_;
    }

    bool *EuclideanSequencer::getPattern()
    {
        return pattern_;
    }

    void EuclideanSequencer::printEuclidPattern()
    {
        String sOut = "";
        for (uint8_t i = 0; i < steps_; i++)
        {
            sOut += (pattern_[i] ? "X" : "-");
        }
        Serial.println(sOut.c_str());
    }
    EuclidSave EuclideanSequencer::getSave()
    {
        EuclidSave save;

        save.rotation_ = rotation_;
        save.events_ = events_;
        save.steps_ = steps_;
        save.noteNumber_ = noteNumber_;
        save.midiChannel_ = midiChannel_ - 1;
        save.velocity_ = velocity_;
        save.swing_ = swing_;
        save.noteLength_ = noteLength_;
        save.clockDivMultP_ = clockDivMultP_;
        save.polyRClockDivMultP_ = polyRClockDivMultP_;
        save.polyRhythmMode_ = polyRhythmMode_;
        return save;
    }

    void EuclideanSequencer::loadSave(EuclidSave save)
    {
        rotation_ = save.rotation_;
        events_ = save.events_;
        steps_ = save.steps_;
        noteNumber_ = save.noteNumber_;
        midiChannel_ = save.midiChannel_ + 1;
        velocity_ = save.velocity_;
        swing_ = save.swing_;
        noteLength_ = save.noteLength_;
        polyRhythmMode_ = save.polyRhythmMode_;

        setClockDivMult(save.clockDivMultP_);
        setPolyRClockDivMult(save.polyRClockDivMultP_);

        patternDirty_ = true;

        tickCount_ = 0;
        seqPos_ = 0;

        nextStepTimeP_ = micros();
        lastStepTimeP_ = micros();
        startMicros = micros();
    }

    void EuclideanSequencer::clockTick(uint32_t stepmicros, uint32_t microsperstep)
    {
        if (patternDirty_)
        {
            regeneratePattern();
            patternDirty_ = false;
        }

        if (!running_)
            return;

        //   seqPerc_ = (stepmicros - startMicros) / ((float)max(stepMicroDelta_, 1) * (steps_ + 1));

        if (steps_ == 0)
        {
            seqPerc_ = 0;

            return;
        }

        //   uint32_t nextBarMicros = stepMicroDelta_ * (steps_ + 1);

        if (stepmicros >= nextStepTimeP_)
        {
            lastStepTimeP_ = nextStepTimeP_;

            if (polyRhythmMode_) // Space all triggers across a bar
            {
                //   stepMicroDelta_ = ((microsperstep * (16 * multiplierPR_)) / steps_) * multiplier_;
                stepMicroDelta_ = ((microsperstep * 16) / steps_) * multiplierPR_;
                //   stepMicroDelta_ = ((microsperstep * (16 * multiplierPR_)) / steps_) * multiplier_;
            }
            else
            {
                stepMicroDelta_ = microsperstep * multiplier_;
            }

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

    void EuclideanSequencer::advanceStep(uint32_t stepmicros)
    {

        if (steps_ == 0)
        {
            seqPos_ = 0;
            lastSeqPos_ = seqPos_;

            return;
        }
        lastSeqPos_ = seqPos_;

        seqPos_ = (seqPos_ + 1) % steps_;

        if (seqPos_ == 0)
        {
            startMicros = stepmicros;
        }
    }

    void EuclideanSequencer::autoReset()
    {
    }

    void EuclideanSequencer::playNote()
    {
        bool sendnoteCV = false;
        // if (sequencer.getPattern(patternNum)->sendCV) {
        // 	sendnoteCV = true;
        // }

        // regular note on trigger
        // uint8_t note = 60;
        // uint8_t channel = 1;
        // uint8_t vel = 100;
        float stepLength = kNoteLengths[noteLength_];
        // uint8_t swing = 0;

        // uint32_t noteoff_micros = micros() + (stepLength) * clockConfig.step_micros;
        // pendingNoteOffs.insert(noteNumber_, channel, noteoff_micros, sendnoteCV);

        uint32_t noteon_micros = seqConfig.currentFrameMicros;

        if (swing_ > 0 && seqPos_ % 2 == 0)
        {
            if (swing_ < 99)
            {
                noteon_micros = seqConfig.currentFrameMicros + ((clockConfig.ppqInterval * multiplier_) / (PPQ / 24) * swing_); // full range swing
            }
            else if (swing_ == 99)
            {                                        // random drunken swing
                uint8_t rnd_swing = rand() % 95 + 1; // rand 1 - 95 // randomly apply swing value
                noteon_micros = seqConfig.currentFrameMicros + ((clockConfig.ppqInterval * multiplier_) / (PPQ / 24) * rnd_swing);
            }
        }
        else
        {
            // noteon_micros = micros();
        }

        // Queue note-on
        onNoteOn(midiChannel_, noteNumber_, velocity_, stepLength, true, sendnoteCV, noteon_micros);
    }

}