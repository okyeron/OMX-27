#include "midifx_chord.h"
#include "../hardware/omx_disp.h"
#include "../utils/chord_util.h"

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

		if (chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
		{
			sendNoteOut(note);
			return;
		}

        onChordOn(note);

		// if (playOrigin_)
		// {
		// 	sendNoteOut(note);
		// }

		// int8_t origNote = note.noteNumber;

		// int8_t sentNoteNumbers[7] = {0, 0, 0, 0, 0, 0, 0};

		// for (uint8_t i = 0; i < 7; i++)
		// {
		// 	if (notes_[i] != 0)
		// 	{
		// 		int8_t newNoteNumber = constrain(origNote + notes_[i], 0, 127);

		// 		bool noteAlreadyPlayed = false;

		// 		for (uint8_t j = 0; j < 7; j++)
		// 		{
		// 			if (sentNoteNumbers[j] == newNoteNumber)
		// 			{
		// 				noteAlreadyPlayed = true;
		// 				break;
		// 			}
		// 		}

		// 		if (!noteAlreadyPlayed)
		// 		{
		// 			note.noteNumber = constrain(origNote + notes_[i], 0, 127);
		// 			sendNoteOut(note);
		// 			sentNoteNumbers[i] = newNoteNumber;
		// 		}
		// 	}
		// }
	}

    void MidiFXChord::onChordOn(MidiNoteGroup inNote)
    {
        if (useGlobalScale_)
		{
			rootNote_ = scaleConfig.scaleRoot;
			scaleIndex_ = scaleConfig.scalePattern;
		}
        // Serial.println("onChordOn: " + String(chordIndex));
        // if (chordNotes_[chordIndex].active)
        // {
        //     // Serial.println("chord already active");
        //     return; // This shouldn't happen
        // }

        chord_.note = inNote.noteNumber % 12;
        chord_.basicOct = (inNote.noteNumber / 12) - 5;
        chord_.octave = chord_.basicOct;

        // if (constructChord(chordIndex))
        if (chordUtil.constructChord(&chord_, &chordNotes_, rootNote_, scaleIndex_))
        {
            chordNotes_.active = true;
            chordNotes_.channel = chord_.mchan + 1;

            // Prevent stuck notes
            // playedChordNotes_[chordIndex].CopyFrom(chordNotes_[chordIndex]);
            // uint8_t velocity = chords_[chordIndex].velocity;

            // uint32_t noteOnMicros = micros();

            // Serial.print("Chord: ");
            for (uint8_t i = 0; i < 6; i++)
            {
                int noteNumber = chordNotes_.notes[i];

                if (noteNumber < 0 || noteNumber > 127)
                {
                    continue;
                }
                // uint8_t velocity = chordNotes_.velocities[i];

                // Serial.print("Note: " + String(note));
                // Serial.print(" Vel: " + String(velocity));
                // Serial.print("\n");

                // if(note >= 0 && note <= 127)
                // {
                //     // MM::sendNoteOn(note, velocity, chordNotes_[chordIndex].channel);
                //     pendingNoteOns.insert(note, velocity, chordNotes_[chordIndex].channel, noteOnMicros, false);
                // }

                inNote.noteNumber = chordNotes_.notes[i];
                inNote.velocity = chordNotes_.velocities[i];

                sendNoteOut(inNote);

                // doNoteOn(note, chordNotes_[chordIndex].midifx, velocity, chordNotes_[chordIndex].channel);
            }
            // Serial.print("\n");
        }
        else
        {
            // Serial.println("constructChord failed");
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
