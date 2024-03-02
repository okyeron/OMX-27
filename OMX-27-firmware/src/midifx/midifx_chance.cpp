#include "midifx_chance.h"
#include "../hardware/omx_disp.h"

namespace midifx
{
	enum ChancePage
	{
		CHPAGE_1
	};

	MidiFXChance::MidiFXChance()
	{
		params_.addPage(1);
		encoderSelect_ = true;
		chancePerc_ = random(100);
	}

	int MidiFXChance::getFXType()
	{
		return MIDIFX_CHANCE;
	}

	const char *MidiFXChance::getName()
	{
		return "Chance";
	}

	const char *MidiFXChance::getDispName()
	{
		return "CHC";
	}

	MidiFXInterface *MidiFXChance::getClone()
	{
		auto clone = new MidiFXChance();
		clone->chancePerc_ = chancePerc_;
		return clone;
	}

	void MidiFXChance::onEnabled()
	{
	}

	void MidiFXChance::onDisabled()
	{
	}

	void MidiFXChance::noteInput(MidiNoteGroup note)
	{
		if (note.noteOff)
		{
			processNoteOff(note);
			return;
		}

		// Serial.println("MidiFXChance::noteInput");
		// note.noteNumber += 7;

		uint8_t r = random(100);

		if (r <= chancePerc_)
		{
			processNoteOn(note.noteNumber, note);
			sendNoteOut(note);
		}
	}

	// MidiFXNoteFunction MidiFXChance::getInputFunc()
	// {
	//     return &MidiFXChance::noteInput;
	// }

	void MidiFXChance::loopUpdate()
	{
	}

	void MidiFXChance::onEncoderChangedEditParam(Encoder::Update enc)
	{
		int8_t page = params_.getSelPage();
		int8_t param = params_.getSelParam();

		auto amt = enc.accel(5);

		if (page == CHPAGE_1)
		{
			if (param == 0)
			{
				chancePerc_ = constrain(chancePerc_ + amt, 0, 100);
			}
		}
		omxDisp.setDirty();
	}

	void MidiFXChance::onDisplayUpdate(uint8_t funcKeyMode)
	{
		omxDisp.clearLegends();

		int8_t page = params_.getSelPage();

		switch (page)
		{
		case CHPAGE_1:
		{
			omxDisp.dispParamBar(chancePerc_, chancePerc_, 0, 100, !getEncoderSelect(), false, "Trigger", "Chance");
		}
		break;
		default:
			break;
		}

		// omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
	}

	int MidiFXChance::saveToDisk(int startingAddress, Storage *storage)
	{
		// Serial.println((String)"Saving mfx chance: " + startingAddress); // 5969
		// Serial.println((String)"chancePerc_: " + chancePerc_);
		storage->write(startingAddress, chancePerc_);
		return startingAddress + 1;
	}

	int MidiFXChance::loadFromDisk(int startingAddress, Storage *storage)
	{
		// Serial.println((String)"Loading mfx chance: " + startingAddress); // 5969

		chancePerc_ = constrain(storage->read(startingAddress), 0, 100);
		// Serial.println((String)"chancePerc_: " + chancePerc_);

		return startingAddress + 1;
	}
}
