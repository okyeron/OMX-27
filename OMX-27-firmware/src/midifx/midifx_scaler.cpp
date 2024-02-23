#include "midifx_scaler.h"
#include "../hardware/omx_disp.h"
#include "../utils/music_scales.h"

namespace midifx
{
	enum ScalerPage
	{
		SCLPAGE_1
	};

	MidiFXScaler::MidiFXScaler()
	{
		params_.addPage(4);
		encoderSelect_ = true;
		calculateRemap();
	}

	int MidiFXScaler::getFXType()
	{
		return MIDIFX_SCALER;
	}

	const char *MidiFXScaler::getName()
	{
		return "Scaler";
	}

	const char *MidiFXScaler::getDispName()
	{
		return "SCAL";
	}

	uint32_t MidiFXScaler::getColor()
	{
		return YELLOW;
	}

	MidiFXInterface *MidiFXScaler::getClone()
	{
		auto clone = new MidiFXScaler();

		clone->chancePerc_ = chancePerc_;
		clone->useGlobalScale_ = useGlobalScale_;
		clone->rootNote_ = rootNote_;
		clone->scaleIndex_ = scaleIndex_;

		clone->calculateRemap();

		return clone;
	}

	void MidiFXScaler::onEnabled()
	{
	}

	void MidiFXScaler::onDisabled()
	{
	}

	void MidiFXScaler::noteInput(MidiNoteGroup note)
	{
		if (note.noteOff)
		{
			processNoteOff(note);
			return;
		}

		// Probability that we scale the note
		if (chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
		{
			sendNoteOut(note);
			return;
		}

		int8_t origNote = note.noteNumber;

		// transpose original note by rootNote
		// int8_t transposedNote = note.noteNumber + rootNote;

		int8_t transposedNote = note.noteNumber;

		int8_t noteIndex = transposedNote % 12;
		int8_t octave = transposedNote / 12;

		// offset for root note
		// noteIndex = (noteIndex + rootNote) % 12;
		// if(noteIndex + rootNote >= 12)
		// {
		//     octave++;
		// }

		// noteIndex = (noteIndex + rootNote) % 12;

		// noteIndex = (noteIndex - rootNote + 12) % 12;

		int8_t remapedNoteIndex = scaleRemapper[noteIndex];

		if (remapedNoteIndex > noteIndex)
		{
			octave--;
		}

		// remapedNoteIndex = (noteIndex + remapedNoteIndex) % 12;

		// remove root note offset
		// remapedNoteIndex = (remapedNoteIndex - rootNote + 12) % 12;

		int8_t newNoteNumber = octave * 12 + remapedNoteIndex;

		// untranspose
		// newNoteNumber = newNoteNumber - rootNote;

		// note out of range, kill
		if (newNoteNumber < 0 || newNoteNumber > 127)
			return;

		note.noteNumber = newNoteNumber;

		processNoteOn(origNote, note);

		sendNoteOut(note);
	}

	// MidiNoteGroup MidiFXScaler::findTriggeredNote(uint8_t noteNumber)
	// {
	//     for(int i = 0; i < triggeredNotes.size(); i++)
	//     {
	//         if(triggeredNotes[i].prevNoteNumber)

	//     }
	//     triggeredNotes.

	//     return nullptr;
	// }

	void MidiFXScaler::calculateRemap()
	{
		if (useGlobalScale_)
		{
			rootNote_ = scaleConfig.scaleRoot;
			scaleIndex_ = scaleConfig.scalePattern;
		}

		if (scaleIndex_ < 0)
		{
			for (uint8_t i = 0; i < 12; i++)
			{
				scaleRemapper[i] = i; // Chromatic scale
			}
			return;
		}

		auto scalePattern = MusicScales::getScalePattern(scaleIndex_);

		uint8_t scaleIndex = 0;
		uint8_t lastNoteIndex = 0;

		// looks through 12 notes, and sets each note to last note in scale
		// so notes out of scale get rounded down to the previous note in the scale.
		for (uint8_t i = 0; i < 12; i++)
		{
			if (scaleIndex < 7 && scalePattern[scaleIndex] == i)
			{
				lastNoteIndex = i;
				scaleIndex++;
			}

			// uint8_t destIndex = (i - rootNote + 12) % 12;
			// uint8_t destIndex = i + rootNote % 12;

			scaleRemapper[i] = (lastNoteIndex + rootNote_) % 12;
		}

		if (rootNote_ > 0)
		{
			// rotate the scale to root
			int8_t temp[12];

			uint8_t val = 12 - rootNote_;

			for (uint8_t i = 0; i < 12; i++)
			{
				temp[i] = scaleRemapper[(i + val) % 12];
			}
			for (int i = 0; i < 12; i++)
			{
				scaleRemapper[i] = temp[i];
			}
		}

		// String msg = "scaleRemapper: ";

		// for (int i = 0; i < 12; i++)
		// {
		//     msg += String(scaleRemapper[i]) + ", ";
		// }

		// msg += "\n\n";

		// Serial.println(msg);
	}

	void MidiFXScaler::loopUpdate()
	{
		if (useGlobalScale_)
		{
			int8_t prevRoot = rootNote_;
			int8_t prevScale = scaleIndex_;

			rootNote_ = scaleConfig.scaleRoot;
			scaleIndex_ = scaleConfig.scalePattern;

			if (rootNote_ != prevRoot || scaleIndex_ != prevScale)
			{
				calculateRemap();
			}
		}
	}

	void MidiFXScaler::onEncoderChangedEditParam(Encoder::Update enc)
	{
		int8_t page = params_.getSelPage();
		int8_t param = params_.getSelParam();

		auto amt = enc.accel(1);

		if (page == SCLPAGE_1)
		{
			if (param == 0)
			{
				useGlobalScale_ = constrain(useGlobalScale_ + amt, 0, 1);
				if (amt != 0)
				{
					if (useGlobalScale_)
					{
						omxDisp.displayMessage("Global: ON");
					}
					else
					{
						omxDisp.displayMessage("Global: OFF");
					}
					calculateRemap();
				}
			}
			else if (param == 1)
			{
				if (useGlobalScale_)
				{
					int prevRoot = scaleConfig.scaleRoot;
					scaleConfig.scaleRoot = constrain(scaleConfig.scaleRoot + amt, 0, 12 - 1);
					if (prevRoot != scaleConfig.scaleRoot)
					{
						calculateRemap();
					}
				}
				else
				{
					int prevRoot = rootNote_;
					rootNote_ = constrain(rootNote_ + amt, 0, 12 - 1);
					if (prevRoot != rootNote_)
					{
						calculateRemap();
					}
				}
			}
			else if (param == 2)
			{
				if (useGlobalScale_)
				{
					int prevPat = scaleConfig.scalePattern;
					scaleConfig.scalePattern = constrain(scaleConfig.scalePattern + amt, -1, MusicScales::getNumScales() - 1);
					if (prevPat != scaleConfig.scalePattern)
					{
						omxDisp.displayMessage(MusicScales::getScaleName(scaleConfig.scalePattern));
						calculateRemap();
					}
				}
				else
				{
					int prevPat = scaleIndex_;
					scaleIndex_ = constrain(scaleIndex_ + amt, -1, MusicScales::getNumScales() - 1);
					if (prevPat != scaleIndex_)
					{
						omxDisp.displayMessage(MusicScales::getScaleName(scaleIndex_));
						calculateRemap();
					}
				}
			}
			else if (param == 3)
			{
				chancePerc_ = constrain(chancePerc_ + amt, 0, 100);
			}
		}

		omxDisp.setDirty();
	}

	void MidiFXScaler::onDisplayUpdate(uint8_t funcKeyMode)
	{
		omxDisp.clearLegends();

		int8_t page = params_.getSelPage();

		switch (page)
		{
		case SCLPAGE_1:
		{
			omxDisp.legends[0] = "GLBL";
			omxDisp.legendText[0] = useGlobalScale_ ? "ON" : "OFF";

			omxDisp.legends[1] = "ROOT";
			omxDisp.legendVals[1] = -127;
			omxDisp.legendText[1] = MusicScales::getNoteName(rootNote_);

			omxDisp.legends[2] = "SCALE";
			if (scaleIndex_ < 0)
			{
				omxDisp.legendVals[2] = -127;
				omxDisp.legendText[2] = "Off";
			}
			else
			{
				omxDisp.legendVals[2] = scaleIndex_;
			}

			omxDisp.legends[3] = "CHC%";
			omxDisp.legendVals[3] = -127;
			omxDisp.useLegendString[3] = true;
			omxDisp.legendString[3] = String(chancePerc_) + "%";
		}
		break;
		default:
			break;
		}

		omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
	}

	int MidiFXScaler::saveToDisk(int startingAddress, Storage *storage)
	{
		// Serial.println((String) "Saving mfx scaler: " + startingAddress); // 5969
		storage->write(startingAddress + 0, chancePerc_);
		storage->write(startingAddress + 1, useGlobalScale_);
		storage->write(startingAddress + 2, (uint8_t)rootNote_);
		storage->write(startingAddress + 3, (uint8_t)scaleIndex_);

		return startingAddress + 4;
	}

	int MidiFXScaler::loadFromDisk(int startingAddress, Storage *storage)
	{
		// Serial.println((String) "Loading mfx scaler: " + startingAddress); // 5969

		chancePerc_ = storage->read(startingAddress + 0);
		useGlobalScale_ = (bool)storage->read(startingAddress + 1);
		rootNote_ = (int8_t)storage->read(startingAddress + 2);
		scaleIndex_ = (int8_t)storage->read(startingAddress + 3);

		calculateRemap();

		return startingAddress + 4;
	}
}
