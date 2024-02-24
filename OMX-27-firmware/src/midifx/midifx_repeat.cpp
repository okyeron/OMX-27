#include "midifx_repeat.h"
#include "../hardware/omx_disp.h"

namespace midifx
{
    enum MidiFXRepeatModes
    {
        MFXREPEATMODE_OFF,
        MFXREPEATMODE_1SHOT,
        MFXREPEATMODE_ON,
        MFXREPEATMODE_HOLD,
        MFXREPEATMODE_COUNT
    };

	MidiFXRepeat::MidiFXRepeat()
	{
		params_.addPage(4);
	
        chancePerc_ = 100;

        numOfRepeats_ = 4;
        mode_ = MFXREPEATMODE_1SHOT;
        rateIndex_ = 6;
        rateHz_ = 40;
        gate_ = 90;
        velStart_ = 10;
        velEnd_ = 115;

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

	void MidiFXRepeat::loopUpdate()
	{
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
