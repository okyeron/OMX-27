#include "midifx_selector.h"
#include "../hardware/omx_disp.h"

namespace midifx
{
    enum MidiFXSelectorModes
    {
        MFXSELMODE_RAND,
        MFXSELMODE_UP,
        MFXSELMODE_DOWN,
        MFXSELMODE_COUNT
    };

    const char* modeLabels[3] = {"RND", "UP", "DOWN"};


	MidiFXSelector::MidiFXSelector()
	{
		params_.addPage(4);

		mode_ = MFXSELMODE_RAND;
		length_ = 2;
        chancePerc_ = 100;

		encoderSelect_ = true;
	}

	int MidiFXSelector::getFXType()
	{
		return MIDIFX_SELECTOR;
	}

	const char *MidiFXSelector::getName()
	{
		return "Selector";
	}

	const char *MidiFXSelector::getDispName()
	{
		return "SEL";
	}

	MidiFXInterface *MidiFXSelector::getClone()
	{
		auto clone = new MidiFXSelector();

		clone->chancePerc_ = chancePerc_;
		clone->mode_ = mode_;
		clone->length_ = length_;

		return clone;
	}

	void MidiFXSelector::onEnabled()
	{
	}

	void MidiFXSelector::onDisabled()
	{
	}

    void MidiFXSelector::handleNoteOff(MidiNoteGroup note)
    {
        processNoteOff(note);
    }

    bool MidiFXSelector::chanceShouldSkip()
    {
        return (chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_));
    }

    uint8_t MidiFXSelector::getFinalMidiFXIndex(uint8_t thisMFXIndex)
    {
        return thisMFXIndex + length_ + 1; // +1 to account for this index, mfxIndex: 0 with length 2 should return index 3
    }

    uint8_t MidiFXSelector::getSelectedMidiFXIndex(uint8_t thisMFXIndex)
    {
        if(length_ <= 1)
        {
            return thisMFXIndex + 1;
        }

        switch (mode_)
        {
        case MFXSELMODE_RAND:
        {
            return thisMFXIndex + random(1, length_ + 1);
        }
        case MFXSELMODE_UP:
        {
            uint8_t selIndex = thisMFXIndex + selPos_ + 1;
            selPos_ = (selPos_ + length_ + 1) % length_;
            return selIndex;
        }
        case MFXSELMODE_DOWN:
        {
            uint8_t selIndex = thisMFXIndex + selPos_ + 1;
            selPos_ = (selPos_ + length_ - 1) % length_;
            return selIndex;
        }
        default:
            break;
        }

        return thisMFXIndex + 1;
    }

    void MidiFXSelector::setNoteInputFunc(uint8_t slotIndex, void (*fptr)(void *, midifx::MidiFXSelector *, uint8_t, MidiNoteGroup), void *context)
	{
        setSlotIndex(slotIndex);
		noteInputContext_ = context;
		noteInputFunctionPtr_ = fptr;
	}

    void MidiFXSelector::noteInput(MidiNoteGroup note)
	{
        // NoteInput handled by function on MidiFX group
        if (noteInputContext_ != nullptr)
		{
			noteInputFunctionPtr_(noteInputContext_, this, mfxSlotIndex_, note);
            return;
		}

        sendNoteOut(note);


		// if (note.noteOff)
		// {
		// 	processNoteOff(note);
		// 	return;
		// }

		// if (chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
		// {
		// 	sendNoteOut(note);
		// 	return;
		// }

        // this->setNoteOutput()





		// int8_t origNote = note.noteNumber;

		// int newNoteNumber = origNote + transpose_ + (octave_ * 12);

		// if (newNoteNumber >= 0 && newNoteNumber <= 127)
		// {
		// 	note.noteNumber = newNoteNumber;
		// 	sendNoteOut(note);
		// }
	}

	void MidiFXSelector::loopUpdate()
	{
	}

	void MidiFXSelector::onEncoderChangedEditParam(Encoder::Update enc)
	{
		int8_t page = params_.getSelPage();
		int8_t param = params_.getSelParam();

		auto amt = enc.accel(1);

		if (page == 0)
		{
			if (param == 0)
			{
				mode_ = constrain(mode_ + amt, 0, MFXSELMODE_COUNT - 1);
			}
			else if (param == 1)
			{
				length_ = constrain(length_ + amt, 0, 7);
			}
			else if (param == 3)
			{
				chancePerc_ = constrain(chancePerc_ + amt, 0, 100);
			}
		}

		omxDisp.setDirty();
	}

	void MidiFXSelector::onDisplayUpdate(uint8_t funcKeyMode)
	{
		omxDisp.clearLegends();

		int8_t page = params_.getSelPage();

		switch (page)
		{
		case 0:
		{
			omxDisp.legends[0] = "MODE";
			omxDisp.legends[1] = "LEN";
			omxDisp.legends[3] = "CHC%";
			omxDisp.legendText[0] = modeLabels[mode_];
			omxDisp.legendVals[1] = length_;
			omxDisp.useLegendString[3] = true;
			omxDisp.legendString[3] = String(chancePerc_) + "%";
		}
		break;
		default:
			break;
		}

		omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
	}

	int MidiFXSelector::saveToDisk(int startingAddress, Storage *storage)
	{
		SelectorSave save;
		save.chancePerc = chancePerc_;
		save.mode = mode_;
		save.length = length_;

		int saveSize = sizeof(SelectorSave);

		auto saveBytesPtr = (byte *)(&save);
		for (int j = 0; j < saveSize; j++)
		{
			storage->write(startingAddress + j, *saveBytesPtr++);
		}

		return startingAddress + saveSize;
	}

	int MidiFXSelector::loadFromDisk(int startingAddress, Storage *storage)
	{
		int saveSize = sizeof(SelectorSave);

		auto save = SelectorSave{};
		auto current = (byte *)&save;
		for (int j = 0; j < saveSize; j++)
		{
			*current = storage->read(startingAddress + j);
			current++;
		}

		chancePerc_ = save.chancePerc;
		mode_ = save.mode;
		length_ = save.length;

		return startingAddress + saveSize;
	}
}
