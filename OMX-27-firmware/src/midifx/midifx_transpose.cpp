#include "midifx_transpose.h"
#include "../hardware/omx_disp.h"

namespace midifx
{
	MidiFXTranspose::MidiFXTranspose()
	{
		params_.addPage(4);

		octave_ = 0;
		transpose_ = 0;

		encoderSelect_ = true;
	}

	int MidiFXTranspose::getFXType()
	{
		return MIDIFX_TRANSPOSE;
	}

	const char *MidiFXTranspose::getName()
	{
		return "Transpose";
	}

	const char *MidiFXTranspose::getDispName()
	{
		return "TRAN";
	}

	uint32_t MidiFXTranspose::getColor()
	{
		return PURPLE;
	}

	MidiFXInterface *MidiFXTranspose::getClone()
	{
		auto clone = new MidiFXTranspose();

		clone->chancePerc_ = chancePerc_;
		clone->transpose_ = transpose_;

		return clone;
	}

	void MidiFXTranspose::onEnabled()
	{
	}

	void MidiFXTranspose::onDisabled()
	{
	}

	void MidiFXTranspose::noteInput(MidiNoteGroup note)
	{
		if (note.noteOff)
		{
			processNoteOff(note);
			return;
		}

		if (chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
		{
			sendNoteOut(note);
			return;
		}

		int8_t origNote = note.noteNumber;

		int newNoteNumber = origNote + transpose_ + (octave_ * 12);

		if (newNoteNumber >= 0 && newNoteNumber <= 127)
		{
			note.noteNumber = newNoteNumber;
			sendNoteOut(note);
		}
	}

	void MidiFXTranspose::loopUpdate()
	{
	}

	void MidiFXTranspose::onEncoderChangedEditParam(Encoder::Update enc)
	{
		int8_t page = params_.getSelPage();
		int8_t param = params_.getSelParam();

		auto amt = enc.accel(1);

		if (page == 0)
		{
			if (param == 0)
			{
				transpose_ = constrain(transpose_ + amt, -24, 24);
			}
			else if (param == 1)
			{
				octave_ = constrain(octave_ + amt, -6, 6);
			}
			else if (param == 3)
			{
				chancePerc_ = constrain(chancePerc_ + amt, 0, 100);
			}
		}

		omxDisp.setDirty();
	}

	void MidiFXTranspose::onDisplayUpdate(uint8_t funcKeyMode)
	{
		omxDisp.clearLegends();

		int8_t page = params_.getSelPage();

		// uint8_t starti = 0;
		// uint8_t endi = 0;

		switch (page)
		{
		case 0:
		{
			omxDisp.legends[0] = "ST";
			omxDisp.legends[1] = "OCT";
			omxDisp.legends[3] = "CHC%";
			omxDisp.useLegendString[0] = true;
			omxDisp.useLegendString[1] = true;
			omxDisp.useLegendString[3] = true;
			omxDisp.legendString[0] = transpose_ == 0 ? "-" : (transpose_ >= 0 ? ("+" + String(transpose_)) : (String(transpose_)));
			omxDisp.legendString[1] = octave_ == 0 ? "-" : (octave_ >= 0 ? ("+" + String(octave_)) : (String(octave_)));
			omxDisp.legendString[3] = String(chancePerc_) + "%";
		}
		break;
		default:
			break;
		}

		omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
	}

	int MidiFXTranspose::saveToDisk(int startingAddress, Storage *storage)
	{
		TransposeSave save;
		save.transpose = transpose_;
		save.octave = octave_;

		int saveSize = sizeof(TransposeSave);

		auto saveBytesPtr = (byte *)(&save);
		for (int j = 0; j < saveSize; j++)
		{
			storage->write(startingAddress + j, *saveBytesPtr++);
		}

		return startingAddress + saveSize;
	}

	int MidiFXTranspose::loadFromDisk(int startingAddress, Storage *storage)
	{
		int saveSize = sizeof(TransposeSave);

		auto save = TransposeSave{};
		auto current = (byte *)&save;
		for (int j = 0; j < saveSize; j++)
		{
			*current = storage->read(startingAddress + j);
			current++;
		}

		transpose_ = save.transpose;
		octave_ = save.octave;

		return startingAddress + saveSize;
	}
}
