#include "midifx_harmonizer.h"
#include "../hardware/omx_disp.h"

namespace midifx
{
	enum HarmonizerPage
	{
		HARMPAGE_1,
		HARMPAGE_2,
		HARMPAGE_3
	};

	MidiFXHarmonizer::MidiFXHarmonizer()
	{
		params_.addPage(4);
		params_.addPage(4);
		params_.addPage(1);

		encoderSelect_ = true;

		playOrigin_ = true;

		for (uint8_t i = 0; i < 7; i++)
		{
			notes_[i] = 0;
		}
	}

	int MidiFXHarmonizer::getFXType()
	{
		return MIDIFX_HARMONIZER;
	}

	const char *MidiFXHarmonizer::getName()
	{
		return "Harmonizer";
	}

	const char *MidiFXHarmonizer::getDispName()
	{
		return "HARM";
	}

	MidiFXInterface *MidiFXHarmonizer::getClone()
	{
		auto clone = new MidiFXHarmonizer();

		clone->chancePerc_ = chancePerc_;
		clone->playOrigin_ = playOrigin_;

		for (uint8_t i = 0; i < 7; i++)
		{
			clone->notes_[i] = notes_[i];
		}

		return clone;
	}

	void MidiFXHarmonizer::onEnabled()
	{
	}

	void MidiFXHarmonizer::onDisabled()
	{
	}

	void MidiFXHarmonizer::noteInput(MidiNoteGroup note)
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

		if (playOrigin_)
		{
			sendNoteOut(note);
		}

		int8_t origNote = note.noteNumber;

		int8_t sentNoteNumbers[7] = {0, 0, 0, 0, 0, 0, 0};

		for (uint8_t i = 0; i < 7; i++)
		{
			if (notes_[i] != 0)
			{
				int8_t newNoteNumber = constrain(origNote + notes_[i], 0, 127);

				bool noteAlreadyPlayed = false;

				for (uint8_t j = 0; j < 7; j++)
				{
					if (sentNoteNumbers[j] == newNoteNumber)
					{
						noteAlreadyPlayed = true;
						break;
					}
				}

				if (!noteAlreadyPlayed)
				{
					note.noteNumber = constrain(origNote + notes_[i], 0, 127);
					sendNoteOut(note);
					sentNoteNumbers[i] = newNoteNumber;
				}
			}
		}
	}

	void MidiFXHarmonizer::loopUpdate()
	{
	}

	void MidiFXHarmonizer::onEncoderChangedEditParam(Encoder::Update enc)
	{
		int8_t page = params_.getSelPage();
		int8_t param = params_.getSelParam();

		auto amt = enc.accel(1);

		bool modNote = false;
		int noteIndex = 0;

		if (page == HARMPAGE_1)
		{
			if (param == 0)
			{
				playOrigin_ = constrain(playOrigin_ + amt, 0, 1);
			}
			else
			{
				modNote = true;
				noteIndex = param - 1;
			}
		}
		else if (page == HARMPAGE_2)
		{
			modNote = true;
			noteIndex = param + 3;
		}
		else if (page == HARMPAGE_3)
		{
			amt = enc.accel(5);
			chancePerc_ = constrain(chancePerc_ + amt, 0, 100);
		}

		if (modNote)
		{
			notes_[noteIndex] = constrain(notes_[noteIndex] + amt, -126, 127);
		}

		omxDisp.setDirty();
	}

	void MidiFXHarmonizer::onDisplayUpdate(uint8_t funcKeyMode)
	{
		omxDisp.clearLegends();

		int8_t page = params_.getSelPage();

		uint8_t starti = 0;
		// uint8_t endi = 0;

		switch (page)
		{
		case HARMPAGE_1:
		{
			omxDisp.legends[0] = "ORIG";

			omxDisp.legendVals[0] = -127;
			omxDisp.legendText[0] = playOrigin_ ? "ON" : "OFF";

			starti = 0;
			// endi = 3;
		}
		break;
		case HARMPAGE_2:
		{
			starti = 3;
			// endi = 7;
		}
		break;
		case HARMPAGE_3:
		{
			omxDisp.legends[0] = "CHC%";

			omxDisp.useLegendString[0] = true;
			omxDisp.legendString[0] = String(chancePerc_) + "%";

			// // const char* perc[4 + sizeof(char)];
			// // sprintf(perc, "%d", chancePerc_);

			// tempStringVal_ = String(chancePerc_) + "%";

			// // char perc = static_cast<char>(chancePerc_);
			// omxDisp.legendVals[0] = -127;
			// // omxDisp.legendText[0] = &perc + "%";
			// omxDisp.legendText[0] = tempStringVal_.c_str();
		}
		break;
		default:
			break;
		}

		if (page == HARMPAGE_1 || page == HARMPAGE_2)
		{
			for (uint8_t i = 0; i < 4; i++)
			{
				if (page == HARMPAGE_1 && i == 0)
					continue;

				// char ch = static_cast<char>(starti + 2);

				tempStrings[i] = "NT " + String(starti + 2);
				omxDisp.legends[i] = tempStrings[i].c_str();
				if (notes_[starti] == 0)
				{
					// omxDisp.legendVals[i] = -127;
					// omxDisp.legendText[i] = "--";

					omxDisp.useLegendString[i] = true;
					omxDisp.legendString[i] = "--";
				}
				else if (notes_[starti] > 0)
				{
					omxDisp.useLegendString[i] = true;
					omxDisp.legendString[i] = "+" + String(notes_[starti]);

					// // char nt = static_cast<char>(notes_[starti]);
					// omxDisp.legendVals[i] = -127;

					// tempStringVal_ = "+" + String(notes_[starti]);
					// omxDisp.legendText[i] = tempStringVal_.c_str();
					// // omxDisp.legendText[i] = "+" + nt;
				}
				else
				{
					omxDisp.legendVals[i] = notes_[starti];
				}

				starti++;
			}
		}

		omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), getEncoderSelect());
	}

	int MidiFXHarmonizer::saveToDisk(int startingAddress, Storage *storage)
	{
		// Serial.println((String) "Saving mfx harmonizer: " + startingAddress); // 5969
		storage->write(startingAddress + 0, chancePerc_);
		storage->write(startingAddress + 1, (bool)playOrigin_);

		for (uint8_t i = 0; i < 7; i++)
		{
			storage->write(startingAddress + 2 + i, (uint8_t)notes_[i]);
		}

		return startingAddress + 9;
	}

	int MidiFXHarmonizer::loadFromDisk(int startingAddress, Storage *storage)
	{
		// Serial.println((String) "Loading mfx harmonizer: " + startingAddress); // 5969

		chancePerc_ = storage->read(startingAddress + 0);
		playOrigin_ = (bool)storage->read(startingAddress + 1);

		for (uint8_t i = 0; i < 7; i++)
		{
			notes_[i] = (int8_t)storage->read(startingAddress + 2 + i);
		}

		return startingAddress + 9;
	}
}
