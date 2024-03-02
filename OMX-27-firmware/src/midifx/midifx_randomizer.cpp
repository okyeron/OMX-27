#include "midifx_randomizer.h"
#include "../hardware/omx_disp.h"
#include "../utils/omx_util.h"

namespace midifx
{
    enum RandPage {
        RZPAGE_CHANCE,
        RZPAGE_1,
        RZPAGE_2,
        RZPAGE_3
    };

    MidiFXRandomizer::MidiFXRandomizer()
    {
        params_.addPage(1); // RZPAGE_CHANCE
        params_.addPage(4); // RZPAGE_1
        params_.addPage(4); // RZPAGE_2
        params_.addPage(4); // RZPAGE_3
        encoderSelect_ = true;

        noteMinus_ = 0;
		notePlus_ = 0;
		octMinus_ = 0;
		octPlus_ = 0;
		velMinus_ = 0;
		velPlus_ = 0;
		lengthPerc_ = 0;
		chancePerc_ = 100;

        midiChan_ = 0;
        delayMin_ = 0;
        delayMax_ = 0;
    }

    int MidiFXRandomizer::getFXType()
    {
        return MIDIFX_RANDOMIZER;
    }

    const char* MidiFXRandomizer::getName()
    {
        return "Randomizer";
    }

    const char* MidiFXRandomizer::getDispName()
    {
        return "RAND";
    }

    MidiFXInterface* MidiFXRandomizer::getClone()
    {
        auto clone = new MidiFXRandomizer();

        clone->noteMinus_ = noteMinus_;
        clone->notePlus_ = notePlus_;
        clone->octMinus_ = octMinus_;
        clone->octPlus_ = octPlus_;
        clone->velMinus_ = velMinus_;
        clone->velPlus_ = velPlus_;
        clone->lengthPerc_ = lengthPerc_;
        clone->chancePerc_ = chancePerc_;
        clone->midiChan_ = midiChan_;
        clone->delayMin_ = delayMin_;
        clone->delayMax_ = delayMax_;

        return clone;
    }

    void MidiFXRandomizer::onEnabled()
    {
    }

    void MidiFXRandomizer::onDisabled()
    {
    }

    

    

    void MidiFXRandomizer::noteInput(MidiNoteGroup note)
    {
        if(note.noteOff)
        {
            removeFromDelayQueue(&note);
            processNoteOff(note);
            return;
        }

        // Serial.println("MidiFXRandomNote::noteInput");

        // Probability that we randomize the note
        if(chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
        {
            sendNoteOut(note);
            return;
        }

        // int8_t origNote = note.noteNumber;

        int8_t octaveMax = octMinus_ + octPlus_ + 1;
        int8_t octave = random(0, octaveMax) - octMinus_;

        note.noteNumber = getRand(note.noteNumber, noteMinus_, notePlus_);
        note.noteNumber = constrain(note.noteNumber + (octave * 12), 0, 127);
        note.velocity = getRand(note.velocity, velMinus_, velPlus_);
        note.stepLength = note.stepLength * map(random(lengthPerc_), 0, 100, 1, 16);

        // if(midiChan_ != 0)
        // {
        //     note.channel = constrain(random(note.channel, note.channel + midiChan_), 1, 16);
        // }

        if(delayMin_ > 0 || delayMax_ > 0)
        {
		    processDelayedNote(&note);
        }
        else
        {
            processNoteOn(note);
        }
    }

    void MidiFXRandomizer::processNoteOff(MidiNoteGroup note)
    {
        bool foundNote = false;
        if (trackedNotes.size() > 0)
        {
            auto it = trackedNotes.begin();
            while (it != trackedNotes.end())
            {
                if (it->prevNoteNumber == note.prevNoteNumber && it->origChannel == note.channel)
                {
                    foundNote = true;
                    // sendNoteOut(note);

                    note.channel = it->channel;
                    // note.noteNumber = it->noteNumber;

                    sendNoteOut(note);

                    it = trackedNotes.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

        if(!foundNote)
        {
    		sendNoteOut(note);
        }
    }

    void MidiFXRandomizer::processNoteOn(MidiNoteGroup note)
    {
        // Tracking not needed if not randomizing midi chan
        if(midiChan_ == 0)
        {
		    sendNoteOut(note);
            return;
        }

        if (trackedNotes.size() > 0)
        {
            for(auto tNt : trackedNotes)
            {
                if (tNt.noteNumber == note.noteNumber && tNt.origChannel == note.channel)
                {
                    // same note is being tracked already, kill note
                    return;
                }
            }
        }

        if (trackedNotes.size() < queueSize)
        {
            RandTrackedNote trackedNote;
            trackedNote.setFromNoteGroup(&note);

            // Keep track of original channel
            trackedNote.origChannel = note.channel;

            note.channel = constrain(random(note.channel, note.channel + midiChan_), 1, 16);

            trackedNote.channel = note.channel;

            trackedNotes.push_back(trackedNote);

            sendNoteOut(note);
        }
        // Kill if queue is full
    }

    void MidiFXRandomizer::removeFromDelayQueue(MidiNoteGroup *note)
    {
        if (delayedNoteQueue.size() > 0)
        {
            auto it = delayedNoteQueue.begin();
            while (it != delayedNoteQueue.end())
            {
                // Serial.println("activeNoteQueue: " + String(it->noteNumber) + " Chan: " + String(it->channel));

                // TODO track note off midichannels and compare with random one
                // Might cause problems
                if (it->noteNumber == note->noteNumber)
                {
                    // `erase()` invalidates the iterator, use returned iterator
                    it = delayedNoteQueue.erase(it);
                    // foundNoteToRemove = true;
                }
                else
                {
                    ++it;
                }
            }
        }

        // return foundNoteToRemove;
    }

    void MidiFXRandomizer::processDelayedNote(MidiNoteGroup *note)
    {
        if (delayedNoteQueue.capacity() > queueSize)
        {
            delayedNoteQueue.shrink_to_fit();
        }

        if(delayedNoteQueue.size() < queueSize)
        {
            float mult = 0.0f;
            // Assume one of these is greater than 0
            if (delayMin_ == 0 || delayMax_ == 0)
            {
                uint8_t rate = delayMin_ == 0 ? getDelayLength(delayMax_) : getDelayLength(delayMin_);
                mult = (1.0f / max((float)rate, __FLT_EPSILON__)) * omxUtil.randFloat();
            }
            else if (delayMin_ > 0 && delayMax_ > 0)
            {
                uint8_t lMin = getDelayLength(delayMin_);
                uint8_t lMax = getDelayLength(delayMax_);

                float rmin = (float)min(lMin, lMax);
                float rmax = (float)max(lMin, lMax);

                float rate = omxUtil.lerp(rmin, rmax, omxUtil.randFloat());

                mult = 1.0f / rate;
            }

            note->noteonMicros = seqConfig.lastClockMicros + (clockConfig.step_micros * 16 * mult);

            delayedNoteQueue.push_back(*note);
        }
        else
        {
            // queue is filled, send note out
            processNoteOn(*note);
        }
    }

    uint8_t MidiFXRandomizer::getRand(uint8_t v, uint8_t minus, uint8_t plus)
    {
        uint8_t minV = max(v - minus, 0);
        uint8_t maxV = min(v + plus + 1, 127);
        return random(minV, maxV);
    }

    void MidiFXRandomizer::loopUpdate()
    {
        if(delayedNoteQueue.size() == 0)
        {
            return;
        }

        uint32_t stepmicros = seqConfig.currentFrameMicros;

        auto it = delayedNoteQueue.begin();
        while (it != delayedNoteQueue.end())
        {
            if (stepmicros >= it->noteonMicros)
            {
                // Send out and remove
                processNoteOn(*it);
                it = delayedNoteQueue.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void MidiFXRandomizer::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        auto amtSlow = enc.accel(1);
        auto amtFast = enc.accel(5);

        switch (page)
        {
        case RZPAGE_CHANCE:
        {
            switch (param)
            {
            case 0:
                chancePerc_ = constrain(chancePerc_ + amtSlow, 0, 100);
                break;
            }
        }
        break;
        case RZPAGE_1:
        {
            switch (param)
            {
            case 0:
                noteMinus_ = constrain(noteMinus_ + amtSlow, 0, 12);
                break;
            case 1:
                notePlus_ = constrain(notePlus_ + amtSlow, 0, 12);
                break;
            case 2:
                octMinus_ = constrain(octMinus_ + amtSlow, 0, 12);
                break;
            case 3:
                octPlus_ = constrain(octPlus_ + amtSlow, 0, 12);
                break;
            }
        }
        break;
        case RZPAGE_2:
        {
            switch (param)
            {
            case 0:
                velMinus_ = constrain(velMinus_ + amtFast, 0, 127);
                break;
            case 1:
                velPlus_ = constrain(velPlus_ + amtFast, 0, 127);
                break;
            case 2:
                lengthPerc_ = constrain(lengthPerc_ + amtFast, 0, 100);
                break;
            case 3:
                // midiChan_ = constrain(midiChan_ + amtSlow, 0, 16);
                break;
            }
        }
        break;
        case RZPAGE_3:
        {
            switch (param)
            {
            case 0:
                delayMin_ = constrain(delayMin_ + amtSlow, 0, kNumArpRates); // 0 = off
                break;
            case 1:
                delayMax_ = constrain(delayMax_ + amtSlow, 0, kNumArpRates);
                break;
            }
        }
        break;
        }

        omxDisp.setDirty();
    }
    uint8_t MidiFXRandomizer::getDelayLength(uint8_t delayIndex)
    {
        if(delayIndex == 0 || delayIndex - 1 >= kNumArpRates)
        {
            return 0;
        }

        uint8_t i = delayIndex - 1;

        // Reverse order
        i = (kNumArpRates - 1) - i;

        return kArpRates[i];
    }

    void MidiFXRandomizer::onDisplayUpdate(uint8_t funcKeyMode)
    {
        omxDisp.clearLegends();

        bool genDisplay = true;

        switch (params_.getSelPage())
        {
        case RZPAGE_CHANCE:
        {
            omxDisp.dispParamBar(chancePerc_, chancePerc_, 0, 100, !getEncoderSelect(), false, "Rand", "Chance");
            genDisplay = false;
        }
        break;
        case RZPAGE_1:
        {
            omxDisp.setLegend(0, "NT-", noteMinus_);
            omxDisp.setLegend(1, "NT+", notePlus_);
            omxDisp.setLegend(2, "OCT-", octMinus_);
            omxDisp.setLegend(3, "OCT+", octPlus_);
        }
        break;
        case RZPAGE_2:
        {
            omxDisp.setLegend(0, "VEL-", velMinus_);
            omxDisp.setLegend(1, "VEL+", velPlus_);
            omxDisp.setLegend(2, "LEN%", lengthPerc_);
            // omxDisp.setLegend(3, "CHAN", midiChan_ == 0, midiChan_);
        }
        break;
        case RZPAGE_3:
        {
            omxDisp.setLegend(0, "DEL-", delayMin_ == 0, "1/" + String(getDelayLength(delayMin_)));
            omxDisp.setLegend(1, "DEL+", delayMax_ == 0, "1/" + String(getDelayLength(delayMax_)));
        }
        break;
        }

        if (genDisplay)
        {
            omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
        }
    }

    int MidiFXRandomizer::saveToDisk(int startingAddress, Storage *storage)
    {
        RandomSave save;
        save.noteMinus = noteMinus_;
        save.notePlus = notePlus_;
        save.octMinus = octMinus_;
        save.octPlus = octPlus_;
        save.velMinus = velMinus_;
        save.velPlus = velPlus_;
        save.lengthPerc = lengthPerc_;
        save.chancePerc = chancePerc_;
        save.midiChan = midiChan_;
        save.delayMin = delayMin_;
        save.delayMax = delayMax_;

        int saveSize = sizeof(RandomSave);

		auto saveBytesPtr = (byte *)(&save);
		for (int j = 0; j < saveSize; j++)
		{
			storage->write(startingAddress + j, *saveBytesPtr++);
		}

		return startingAddress + saveSize;
    }

    int MidiFXRandomizer::loadFromDisk(int startingAddress, Storage *storage)
    {
        int saveSize = sizeof(RandomSave);

		auto save = RandomSave{};
		auto current = (byte *)&save;
		for (int j = 0; j < saveSize; j++)
		{
			*current = storage->read(startingAddress + j);
			current++;
		}

        noteMinus_ = save.noteMinus;
        notePlus_ = save.notePlus;
        octMinus_ = save.octMinus;
        octPlus_ = save.octPlus;
        velMinus_ = save.velMinus;
        velPlus_ = save.velPlus;
        lengthPerc_ = save.lengthPerc;
        chancePerc_ = save.chancePerc;
        midiChan_ = save.midiChan;
        delayMin_ = save.delayMin;
        delayMax_ = save.delayMax;

        return startingAddress + saveSize;
    }
}
