#include "midifx_chord.h"
#include "../hardware/omx_disp.h"

namespace midifx
{
	enum MFXChordPage
	{
		MFXCHORDPAGE_1
	};

	MidiFXChord::MidiFXChord()
	{
		params_.addPage(4);
		encoderSelect_ = true;
	}

	int MidiFXChord::getFXType()
	{
		return MIDIFX_CHANCE;
	}

	const char *MidiFXChord::getName()
	{
		return "Chord";
	}

	const char *MidiFXChord::getDispName()
	{
		return "CHRD";
	}

	uint32_t MidiFXChord::getColor()
	{
		return CYAN;
	}

	MidiFXInterface *MidiFXChord::getClone()
	{
		auto clone = new MidiFXChord();
		clone->chancePerc_ = chancePerc_;
		return clone;
	}

	void MidiFXChord::onEnabled()
	{
	}

	void MidiFXChord::onDisabled()
	{
	}

	void MidiFXChord::noteInput(MidiNoteGroup note)
	{
		if (note.noteOff)
		{
			processNoteOff(note);
			return;
		}

		// Serial.println("MidiFXChord::noteInput");
		// note.noteNumber += 7;

		uint8_t r = random(255);

		if (r <= chancePerc_)
		{
			processNoteOn(note.noteNumber, note);
			sendNoteOut(note);
		}
	}

	// MidiFXNoteFunction MidiFXChord::getInputFunc()
	// {
	//     return &MidiFXChord::noteInput;
	// }

	void MidiFXChord::loopUpdate()
	{
	}

	void MidiFXChord::onEncoderChangedEditParam(Encoder::Update enc)
	{
		int8_t page = params_.getSelPage();
		int8_t param = params_.getSelParam();

		auto amt = enc.accel(5);

		if (page == MFXCHORDPAGE_1)
		{
			if (param == 0)
			{
				chancePerc_ = constrain(chancePerc_ + amt, 0, 255);
			}
		}
		omxDisp.setDirty();
	}

	void MidiFXChord::onDisplayUpdate(uint8_t funcKeyMode)
	{
		omxDisp.clearLegends();

		int8_t page = params_.getSelPage();

		switch (page)
		{
		case MFXCHORDPAGE_1:
		{
			omxDisp.legends[0] = "CHC%";
			omxDisp.legends[1] = "";
			omxDisp.legends[2] = "";
			omxDisp.legends[3] = "";
			omxDisp.legendVals[0] = -127;
			omxDisp.legendVals[1] = -127;
			omxDisp.legendVals[2] = -127;
			omxDisp.legendVals[3] = -127;
			omxDisp.useLegendString[0] = true;
			uint8_t perc = ((chancePerc_ / 255.0f) * 100);
			omxDisp.legendString[0] = String(perc) + "%";
		}
		break;
		default:
			break;
		}

		omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
	}

	int MidiFXChord::saveToDisk(int startingAddress, Storage *storage)
	{
		// Serial.println((String)"Saving mfx chance: " + startingAddress); // 5969
		// Serial.println((String)"chancePerc_: " + chancePerc_);
		storage->write(startingAddress, chancePerc_);
		return startingAddress + 1;
	}

	int MidiFXChord::loadFromDisk(int startingAddress, Storage *storage)
	{
		// Serial.println((String)"Loading mfx chance: " + startingAddress); // 5969

		chancePerc_ = storage->read(startingAddress);
		// Serial.println((String)"chancePerc_: " + chancePerc_);

		return startingAddress + 1;
	}
}
