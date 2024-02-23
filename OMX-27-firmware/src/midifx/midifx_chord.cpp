#include "midifx_chord.h"
#include "../hardware/omx_disp.h"
#include "../utils/omx_util.h"
#include "../utils/chord_util.h"
#include "../utils/music_scales.h"

namespace midifx
{
    enum MfxChordsModePage
    {
        MFXCHRDPAGE_NOTES,
        MFXCHRDPAGE_GBL1, // UI Mode
        // MFXCHRDPAGE_OUTMIDI, // Oct, CH, Vel
        // MFXCHRDPAGE_POTSANDMACROS, // PotBank, Thru, Macro, Macro Channel
        // MFXCHRDPAGE_SCALES, // Root, Scale, Lock Scale Notes, Group notes. 
        MFXCHRDPAGE_1,	   // Chord Type, MidiFX, 0, Midi Channel
        MFXCHRDPAGE_2,	   // Note, Octave, Chord,         | numNotes, degree, octave, transpose
        MFXCHRDPAGE_3,	   //                              | spread, rotate, voicing
        MFXCHRDPAGE_4,	   //                              | spreadUpDown, quartalVoicing
    };

	enum MFXChordPage
	{
		MFXCHORDPAGE_1
	};

	MidiFXChord::MidiFXChord()
	{
        basicParams_.addPage(1); // MFXCHRDPAGE_NOTES
        basicParams_.addPage(4); // MFXCHRDPAGE_GBL1
        basicParams_.addPage(4); // MFXCHRDPAGE_1
        basicParams_.addPage(4); // MFXCHRDPAGE_2
        basicParams_.addPage(6); // MFXCHRDPAGE_3 - Custom chord notes, toggled on and off

        intervalParams_.addPage(1); // MFXCHRDPAGE_NOTES
        intervalParams_.addPage(4); // MFXCHRDPAGE_GBL1
        intervalParams_.addPage(4); // MFXCHRDPAGE_1
        intervalParams_.addPage(4); // MFXCHRDPAGE_2
        intervalParams_.addPage(4); // MFXCHRDPAGE_3
        intervalParams_.addPage(4); // MFXCHRDPAGE_4

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
            if(note.prevNoteNumber == lastNote_)
            {
                chordNotes_.active = false;
            }
            
			processNoteOff(note);
			return;
		}

		if (chancePerc_ != 100 && (chancePerc_ == 0 || random(100) > chancePerc_))
		{
			sendNoteOut(note);
			return;
		}

        lastNote_ = note.prevNoteNumber;

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

    void MidiFXChord::calculateRemap()
    {
    }

    void MidiFXChord::onEncoderChangedSelectParam(Encoder::Update enc)
	{
        auto params = getParams();

        params->changeParam(enc.dir());
		omxDisp.setDirty();
	}

    void MidiFXChord::onEncoderChangedEditParam(Encoder::Update enc)
    {
        auto params = getParams();

        int8_t selPage = params->getSelPage();
        int8_t selParam = params->getSelParam() + 1; // Add one for readability

        auto chordPtr = &chord_;

        auto amtSlow = enc.accel(1);
        // auto amtFast = enc.accel(5);

        // Global 1 - UI Mode, Root, Scale, Octave
        if (selPage == MFXCHRDPAGE_GBL1)
        {
            if (selParam == 1)
			{
				useGlobalScale_ = constrain(useGlobalScale_ + amtSlow, 0, 1);
				if (amtSlow != 0)
				{
                    omxDisp.displayMessage((useGlobalScale_ ? "Global: ON" : "Global: OFF"));
					calculateRemap();
				}
			}
			else if (selParam == 2)
			{
				if (useGlobalScale_)
				{
					int prevRoot = scaleConfig.scaleRoot;
					scaleConfig.scaleRoot = constrain(scaleConfig.scaleRoot + amtSlow, 0, 12 - 1);
					if (prevRoot != scaleConfig.scaleRoot)
					{
						calculateRemap();
					}
				}
				else
				{
					int prevRoot = rootNote_;
					rootNote_ = constrain(rootNote_ + amtSlow, 0, 12 - 1);
					if (prevRoot != rootNote_)
					{
						calculateRemap();
					}
				}
			}
			else if (selParam == 3)
			{
				if (useGlobalScale_)
				{
					int prevPat = scaleConfig.scalePattern;
					scaleConfig.scalePattern = constrain(scaleConfig.scalePattern + amtSlow, -1, MusicScales::getNumScales() - 1);
					if (prevPat != scaleConfig.scalePattern)
					{
						omxDisp.displayMessage(MusicScales::getScaleName(scaleConfig.scalePattern));
						calculateRemap();
					}
				}
				else
				{
					int prevPat = scaleIndex_;
					scaleIndex_ = constrain(scaleIndex_ + amtSlow, -1, MusicScales::getNumScales() - 1);
					if (prevPat != scaleIndex_)
					{
						omxDisp.displayMessage(MusicScales::getScaleName(scaleIndex_));
						calculateRemap();
					}
				}
			}
			else if (selParam == 4)
			{
				chancePerc_ = constrain(chancePerc_ + amtSlow, 0, 100);
			}
            // onEncoderChangedEditParam(&enc, selParam, 1, CPARAM_UIMODE);
            // onEncoderChangedEditParam(&enc, selParam, 2, CPARAM_SCALE_ROOT);
            // onEncoderChangedEditParam(&enc, selParam, 3, CPARAM_SCALE_PAT);
            // onEncoderChangedEditParam(&enc, selParam, 4, CPARAM_GBL_OCT);
        }
        // else if (selPage == MFXCHRDPAGE_SCALES)
        // {
        //     auto musicScale = chordUtil.getMusicScale();
        //     omxUtil.onEncoderChangedEditParam(&enc, musicScale, selParam, 1, GPARAM_SCALE_ROOT);
        //     omxUtil.onEncoderChangedEditParam(&enc, musicScale, selParam, 2, GPARAM_SCALE_PAT);
        //     // omxUtil.onEncoderChangedEditParam(&enc, musicScale, selParam, 3, GPARAM_SCALE_LOCK);
        //     // omxUtil.onEncoderChangedEditParam(&enc, musicScale, selParam, 4, GPARAM_SCALE_GRP16);
        // }
        else if (selPage == MFXCHRDPAGE_1)
        {
            chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 1, CPARAM_CHORD_TYPE);
            // chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 2, CPARAM_CHORD_MFX);
            // chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 3, CPARAM_CHORD_VEL);
            // chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 4, CPARAM_CHORD_MCHAN);
        }
        // PAGE TWO - Basic: Note, Octave, Chord    Interval: numNotes, degree, octave, transpose
        else if (selPage == MFXCHRDPAGE_2)
        {
            if (chord_.type == CTYPE_INTERVAL)
            {
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 1, CPARAM_INT_NUMNOTES);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 2, CPARAM_INT_DEGREE);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 3, CPARAM_INT_OCTAVE);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 4, CPARAM_INT_TRANSPOSE);
            }
            else if (chord_.type == CTYPE_BASIC)
            {
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 1, CPARAM_BAS_NOTE);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 2, CPARAM_BAS_OCT);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 3, CPARAM_BAS_BALANCE);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 4, CPARAM_BAS_CHORD);
            }
        }
        // PAGE THREE - spread, rotate, voicing
        else if (selPage == MFXCHRDPAGE_3)
        {
            if (chord_.type == CTYPE_INTERVAL)
            {
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 1, CPARAM_INT_SPREAD);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 2, CPARAM_INT_ROTATE);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 3, CPARAM_INT_VOICING);
            }
            else if (chord_.type == CTYPE_BASIC)
            {
                auto amtSlow = enc.accel(1);
                int8_t sel = params->getSelParam();
                chord_.customNotes[sel].note = constrain(chord_.customNotes[sel].note + amtSlow, -48, 48);

                if (amtSlow != 0) // To see notes change on keyboard leds
                {
                    if (useGlobalScale_)
                    {
                        rootNote_ = scaleConfig.scaleRoot;
                        scaleIndex_ = scaleConfig.scalePattern;
                    }

                    chordUtil.constructChord(chordPtr, &chordNotes_, rootNote_, scaleIndex_);
                }
            }
        }
        // PAGE FOUR - spreadUpDown, quartalVoicing
        else if (selPage == MFXCHRDPAGE_4)
        {
            if (chord_.type == CTYPE_INTERVAL)
            {
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 1, CPARAM_INT_SPRDUPDOWN);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 2, CPARAM_INT_QUARTVOICE);
            }
        }

        omxDisp.setDirty();
        // omxLeds.setDirty();

        // int8_t page = params_.getSelPage();
        // int8_t param = params_.getSelParam();

        // auto amt = enc.accel(5);

        // if (page == MFXCHORDPAGE_1)
        // {
        // 	if (param == 0)
        // 	{
        // 		chancePerc_ = constrain(chancePerc_ + amt, 0, 255);
        // 	}
        // }
        // omxDisp.setDirty();
    }

    ParamManager *MidiFXChord::getParams()
    {
        if (chord_.type == CTYPE_BASIC)
        {
            basicParams_.setPageEnabled(MFXCHRDPAGE_3, chord_.chord == kCustomChordPattern);
            intervalParams_.setSelPageAndParam(basicParams_.getSelPage(), basicParams_.getSelParam());

            return &basicParams_;
        }
        else
        {
            basicParams_.setSelPageAndParam(intervalParams_.getSelPage(), intervalParams_.getSelParam());
            return &intervalParams_;
        }
    }

    void MidiFXChord::onDisplayUpdate(uint8_t funcKeyMode)
    {
        omxDisp.clearLegends();

        auto params = getParams();

        // if (chordEditMode_ == false && (mode_ == CHRDMODE_EDIT) && funcKeyMode_ == FUNCKEYMODE_F1) // Edit mode enter edit mode
        // {
        //     omxDisp.dispGenericModeLabel("Edit chord", params->getNumPages(), params->getSelPage());
        // }
        // else if (chordEditMode_ == false && (mode_ == CHRDMODE_EDIT) && funcKeyMode_ == FUNCKEYMODE_F2) // Edit mode copy
        // {
        //     omxDisp.dispGenericModeLabel("Copy to", params->getNumPages(), params->getSelPage());
        // }
        // if (chordEditMode_ == false && (mode_ == CHRDMODE_PLAY || mode_ == CHRDMODE_EDIT || mode_ == CHRDMODE_MANSTRUM) && funcKeyMode_ == FUNCKEYMODE_F2) // Play mode copy
        // {
        //     omxDisp.dispGenericModeLabel("Copy to", params->getNumPages(), params->getSelPage());
        // }
        // else if (chordEditMode_ == false && (mode_ == CHRDMODE_PRESET) && funcKeyMode_ == FUNCKEYMODE_F1) // Preset move load
        // {
        // 	omxDisp.dispGenericModeLabel("Load from", params->getNumPages(), params->getSelPage());
        // }
        // else if (chordEditMode_ == false && (mode_ == CHRDMODE_PRESET) && funcKeyMode_ == FUNCKEYMODE_F2) // Preset move save
        // {
        // 	omxDisp.dispGenericModeLabel("Save to", params->getNumPages(), params->getSelPage());
        // }
        // else if (chordEditMode_ == false && mode_ == CHRDMODE_MANSTRUM)
        // {
        //     omxDisp.dispGenericModeLabel("Enc Strum", params->getNumPages(), 0);
        // }
        // else
        if (params->getSelPage() == MFXCHRDPAGE_NOTES)
        {
            // if (chordNotes_[selectedChord_].active || chordEditNotes_.active)
            // {
            // notesString = "";
            tempString = "";

            for (uint8_t i = 0; i < 6; i++)
            {
                int8_t note = chordNotes_.notes[i];

                if (chordEditNotes_.active)
                {
                    note = chordEditNotes_.notes[i];
                }

                if (note >= 0 && note <= 127)
                {
                    if (i > 0)
                    {
                        tempString.append(" ");
                    }
                    tempString.append(MusicScales::getFullNoteName(note));
                }
            }

            const char *labels[1];
            labels[0] = tempString.c_str();
            // if (chordEditNotes_.active)
            // {
            //     // int rootNote = chords_[selectedChord_].note;
            //     omxDisp.dispKeyboard(chordEditNotes_.rootNote, chordEditNotes_.notes, true, labels, 1);
            // }
            // else
            // {
            //     omxDisp.dispKeyboard(chordNotes_[selectedChord_].rootNote, chordNotes_[selectedChord_].notes, true, labels, 1);
            // }

            omxDisp.dispKeyboard(chordNotes_.rootNote, chordNotes_.notes, true, labels, 1);
            // }
            // else
            // {
            //     omxDisp.dispKeyboard(-1, noNotes, false, nullptr, 0);

            //     // omxDisp.dispGenericModeLabel("-", params->getNumPages(), params->getSelPage());
            // }
        }
        // Chord page
        else if (params->getSelPage() == MFXCHRDPAGE_2 && chord_.type == CTYPE_BASIC)
        {
            auto noteName = MusicScales::getNoteName(chord_.note, true);
            int octave = chord_.basicOct + 4;
            tempString = String(octave);
            auto chordType = kChordMsg[chord_.chord];

            activeChordBalance_ = chordUtil.getChordBalance(chord_.balance);

            omxDisp.dispChordBasicPage(params->getSelParam(), getEncoderSelect(), noteName, tempString.c_str(), chordType, activeChordBalance_.type, activeChordBalance_.velMult);
        }
        // Custom Chord Notes
        else if (params->getSelPage() == MFXCHRDPAGE_3 && chord_.type == CTYPE_BASIC && chord_.chord == kCustomChordPattern)
        {
            const char *labels[6];
            const char *headers[1];
            headers[0] = "Custom Chord";

            for (uint8_t i = 0; i < 6; i++)
            {
                int note = chord_.customNotes[i].note;

                if (note == 0)
                {
                    if (i == 0)
                    {
                        tempStrings[i] = "RT";
                        // customNotesStrings[i] = "RT";
                    }
                    else
                    {
                        tempStrings[i] = "-";
                        // customNotesStrings[i] = "-";
                    }
                }
                else
                {
                    if (note > 0)
                    {
                        tempStrings[i] = "+" + String(note);
                        // customNotesStrings[i] = "+" + String(note);
                    }
                    else
                    {
                        tempStrings[i] = "" + String(note);
                        // customNotesStrings[i] = "" + String(note);
                    }
                }

                labels[i] = tempStrings[i].c_str();
                // labels[i] = customNotesStrings[i].c_str();
            }

            omxDisp.dispCenteredSlots(labels, 6, params->getSelParam(), getEncoderSelect(), true, true, headers, 1);
        }
        else // Boring generic view
        {
            setupPageLegends();
            omxDisp.dispGenericMode2(params->getNumPages(), params->getSelPage(), params->getSelParam(), getEncoderSelect());
        }
    }
        
    void MidiFXChord::setupPageLegend(ChordSettings *chord, uint8_t index, uint8_t paramType)
    {
    }

    void MidiFXChord::setupPageLegends()
    {
        omxDisp.clearLegends();

        int8_t page = getParams()->getSelPage();

        auto chordPtr = &chord_;

        switch (page)
        {
        case MFXCHRDPAGE_GBL1:
        {
            omxDisp.legends[0] = "GLBL";
			omxDisp.legendText[0] = useGlobalScale_ ? "ON" : "OFF";

			omxDisp.legends[1] = "ROOT";
			omxDisp.legendText[1] = MusicScales::getNoteName(rootNote_);

			omxDisp.legends[2] = "SCALE";
			if (scaleIndex_ < 0)
			{
				omxDisp.legendText[2] = "Off";
			}
			else
			{
				omxDisp.legendVals[2] = scaleIndex_;
			}

			omxDisp.legends[3] = "CHC%";
            tempString = String(chancePerc_) + "%";
			omxDisp.legendText[3] = tempString.c_str();
        }
        break;
        // case MFXCHRDPAGE_OUTMIDI:
        // {
        //     omxUtil.setupPageLegend(0, GPARAM_MOUT_OCT);
        //     omxUtil.setupPageLegend(1, GPARAM_MOUT_CHAN);
        //     omxUtil.setupPageLegend(2, GPARAM_MOUT_VEL);
        // }
        // break;
        // case MFXCHRDPAGE_POTSANDMACROS:
        // {
        //     omxUtil.setupPageLegend(0, GPARAM_POTS_PBANK);
        //     omxUtil.setupPageLegend(1, GPARAM_MIDI_THRU);
        //     omxUtil.setupPageLegend(2, GPARAM_MACRO_MODE);
        //     omxUtil.setupPageLegend(3, GPARAM_MACRO_CHAN);
        // }
        // break;
        // case MFXCHRDPAGE_SCALES:
        // {
        //     auto musicScale = chordUtil.getMusicScale();
        //     omxUtil.setupPageLegend(musicScale, 0, GPARAM_SCALE_ROOT);
        //     omxUtil.setupPageLegend(musicScale, 1, GPARAM_SCALE_PAT);
        //     omxUtil.setupPageLegend(musicScale, 2, GPARAM_SCALE_LOCK);
        //     omxUtil.setupPageLegend(musicScale, 3, GPARAM_SCALE_GRP16);
        // }
        // break;
        case MFXCHRDPAGE_1:
        {
            chordUtil.setupPageLegend(chordPtr, 0, CPARAM_CHORD_TYPE);
            // chordUtil.setupPageLegend(chordPtr, 1, CPARAM_CHORD_MFX);
            // chordUtil.setupPageLegend(chordPtr, 2, CPARAM_CHORD_VEL);
            // chordUtil.setupPageLegend(chordPtr, 3, CPARAM_CHORD_MCHAN);
        }
        break;
        case MFXCHRDPAGE_2:
        {
            if (chord_.type == CTYPE_INTERVAL)
            {
                chordUtil.setupPageLegend(chordPtr, 0, CPARAM_INT_NUMNOTES);
                chordUtil.setupPageLegend(chordPtr, 1, CPARAM_INT_DEGREE);
                chordUtil.setupPageLegend(chordPtr, 2, CPARAM_INT_OCTAVE);
                chordUtil.setupPageLegend(chordPtr, 3, CPARAM_INT_TRANSPOSE);
            }
            else if (chord_.type == CTYPE_BASIC)
            {
                chordUtil.setupPageLegend(chordPtr, 0, CPARAM_BAS_NOTE);
                chordUtil.setupPageLegend(chordPtr, 1, CPARAM_BAS_OCT);
                chordUtil.setupPageLegend(chordPtr, 2, CPARAM_BAS_CHORD);
                chordUtil.setupPageLegend(chordPtr, 3, CPARAM_BAS_BALANCE);
            }
        }
        break;
        case MFXCHRDPAGE_3:
        {
            if (chord_.type == CTYPE_INTERVAL)
            {
                chordUtil.setupPageLegend(chordPtr, 0, CPARAM_INT_SPREAD);
                chordUtil.setupPageLegend(chordPtr, 1, CPARAM_INT_ROTATE);
                chordUtil.setupPageLegend(chordPtr, 2, CPARAM_INT_VOICING);
            }
        }
        break;
        case MFXCHRDPAGE_4:
        {
            if (chord_.type == CTYPE_INTERVAL)
            {
                chordUtil.setupPageLegend(chordPtr, 0, CPARAM_INT_SPRDUPDOWN);
                chordUtil.setupPageLegend(chordPtr, 1, CPARAM_INT_QUARTVOICE);
            }
        }
        break;
        default:
            break;
        }
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
