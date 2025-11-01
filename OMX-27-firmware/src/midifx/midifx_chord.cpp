#include "midifx_chord.h"
#include "../hardware/omx_disp.h"
#include "../utils/omx_util.h"
#include "../utils/chord_util.h"
#include "../utils/music_scales.h"

namespace midifx
{
    const char* chordTypeLabel = "Chord Type";
    const char* chordTypeOptions[2] = {"Basic", "Interval"};

    enum MfxChordsModePage
    {
        MFXCHRDPAGE_NOTES = 0,
        MFXCHRDPAGE_CHORDTYPE = 1,	    // Select Chord Type, Basic or Interval
        MFXCHRDPAGE_CHANCE = 2,	    // Select Chord Type, Basic or Interval
        MFXCHRDPAGE_BASIC_NOTES = 3,
        MFXCHRDPAGE_CUSTOM_NOTES = 4,
        MFXCHRDPAGE_SCALES = 3,
        MFXCHRDPAGE_INT_NOTES = 4,
        MFXCHRDPAGE_INT_SPREAD = 5,
        MFXCHRDPAGE_INT_QUART = 6,
        // MFXCHRDPAGE_1,	            // Note, Octave, Chord,         | numNotes, degree, octave, transpose
        // MFXCHRDPAGE_3,	            //                              | spread, rotate, voicing
        // MFXCHRDPAGE_4,	            //                              | spreadUpDown, quartalVoicing
    };

	enum MFXChordPage
	{
		MFXCHORDPAGE_1
	};

	MidiFXChord::MidiFXChord()
	{
        basicParams_.addPage(1); // MFXCHRDPAGE_NOTES
        basicParams_.addPage(1); // MFXCHRDPAGE_CHORDTYPE
        basicParams_.addPage(1); // MFXCHRDPAGE_CHANCE
        basicParams_.addPage(4); // MFXCHRDPAGE_BASIC_NOTES
        basicParams_.addPage(6); // MFXCHRDPAGE_CUSTOM_NOTES - Custom chord notes, toggled on and off

        intervalParams_.addPage(1); // MFXCHRDPAGE_NOTES
        intervalParams_.addPage(1); // MFXCHRDPAGE_CHORDTYPE
        intervalParams_.addPage(1); // MFXCHRDPAGE_CHANCE
        intervalParams_.addPage(4); // MFXCHRDPAGE_SCALES
        intervalParams_.addPage(4); // MFXCHRDPAGE_INT_NOTES
        intervalParams_.addPage(4); // MFXCHRDPAGE_INT_SPREAD
        intervalParams_.addPage(4); // MFXCHRDPAGE_INT_QUART

		encoderSelect_ = true;
	}

	int MidiFXChord::getFXType()
	{
		return MIDIFX_CHORD;
	}

	const char *MidiFXChord::getName()
	{
		return "Chord";
	}

	const char *MidiFXChord::getDispName()
	{
		return "CHRD";
	}

	MidiFXInterface *MidiFXChord::getClone()
	{
		auto clone = new MidiFXChord();

        clone->chancePerc_ = chancePerc_;
        clone->useGlobalScale_ = useGlobalScale_;
        clone->rootNote_ = rootNote_;
        clone->scaleIndex_ = scaleIndex_;
        clone->chord_.CopySettingsFrom(&chord_);
        
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

        int8_t autoOctave = 0;

        if (chord_.type == CTYPE_BASIC)
        {
            chord_.note = inNote.noteNumber % 12;
            autoOctave = (inNote.noteNumber / 12) - 5;
        }
        else if (chord_.type == CTYPE_INTERVAL)
        {
            // Get the note forced to the current scale
            // int8_t noteInScale = chordUtil.getMusicScale()->remapNoteToScale(inNote.noteNumber);
            chord_.degree = MusicScales::getDegreeFromNote(inNote.noteNumber, rootNote_, scaleIndex_);
            // chord_.basicOct = (inNote.noteNumber / 12) - 5;
            autoOctave = ((inNote.noteNumber + 12 - rootNote_) / 12) - 6;
        }

        // if (constructChord(chordIndex))
        if (chordUtil.constructChord(&chord_, &chordNotes_, autoOctave, rootNote_, scaleIndex_, true))
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
        auto amtFast = enc.accel(5);

        if (selPage == MFXCHRDPAGE_CHORDTYPE)
        {
            chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 1, CPARAM_CHORD_TYPE);
        }
        else if (selPage == MFXCHRDPAGE_CHANCE)
        {
            chancePerc_ = constrain(chancePerc_ + amtFast, 0, 100);
        }

        if (chord_.type == CTYPE_INTERVAL)
        {
            if (selPage == MFXCHRDPAGE_SCALES)
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
                        rootNote_ = scaleConfig.scaleRoot;
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
			            scaleIndex_ = scaleConfig.scalePattern;
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
            }
            else if (selPage == MFXCHRDPAGE_INT_NOTES)
            {
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 1, CPARAM_INT_NUMNOTES);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 2, CPARAM_INT_DEGREE);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 3, CPARAM_INT_OCTAVE);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 4, CPARAM_INT_TRANSPOSE);
            }
            else if (selPage == MFXCHRDPAGE_INT_SPREAD)
            {
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 1, CPARAM_INT_SPREAD);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 2, CPARAM_INT_ROTATE);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 3, CPARAM_INT_VOICING);
            }
            else if (selPage == MFXCHRDPAGE_INT_QUART)
            {
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 1, CPARAM_INT_SPRDUPDOWN);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 2, CPARAM_INT_QUARTVOICE);
            }
        }
        else if (chord_.type == CTYPE_BASIC)
        {
            if (selPage == MFXCHRDPAGE_BASIC_NOTES)
            {
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 1, CPARAM_BAS_NOTE);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 2, CPARAM_BAS_OCT);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 3, CPARAM_BAS_BALANCE);
                chordUtil.onEncoderChangedEditParam(&enc, chordPtr, selParam, 4, CPARAM_BAS_CHORD);
            }
            else if (selPage == MFXCHRDPAGE_CUSTOM_NOTES)
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

                    chordUtil.constructChord(chordPtr, &chordNotes_, midiSettings.octave, rootNote_, scaleIndex_, true);
                }
            }
        }

        omxDisp.setDirty();
    }

    ParamManager *MidiFXChord::getParams()
    {
        if (chord_.type == CTYPE_BASIC)
        {
            basicParams_.setPageEnabled(MFXCHRDPAGE_CUSTOM_NOTES, chord_.chord == kCustomChordPattern);
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
                        tempString += " ";
                    }
                    tempString += (MusicScales::getFullNoteName(note));
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
        else if (params->getSelPage() == MFXCHRDPAGE_CHORDTYPE)
        {
            omxDisp.dispOptionCombo(chordTypeLabel, chordTypeOptions, 2, chord_.type, getEncoderSelect());
        }
        else if (params->getSelPage() == MFXCHRDPAGE_CHANCE)
        {
            omxDisp.dispParamBar(chancePerc_, chancePerc_, 0, 100, !getEncoderSelect(), false, "Chord Trigger", "Chance");
        }
        // Chord page
        else if (params->getSelPage() == MFXCHRDPAGE_BASIC_NOTES && chord_.type == CTYPE_BASIC)
        {
            auto noteName = MusicScales::getNoteName(chord_.note, true);
            int octave = chord_.basicOct + 4;
            tempString = String(octave);
            auto chordType = kChordMsg[chord_.chord];

            activeChordBalance_ = chordUtil.getChordBalance(chord_.balance);

            omxDisp.dispChordBasicPage(params->getSelParam(), getEncoderSelect(), noteName, tempString.c_str(), chordType, activeChordBalance_.type, activeChordBalance_.velMult);
        }
        // Custom Chord Notes
        else if (params->getSelPage() == MFXCHRDPAGE_CUSTOM_NOTES && chord_.type == CTYPE_BASIC && chord_.chord == kCustomChordPattern)
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

        if (page == MFXCHRDPAGE_CHORDTYPE)
        {
            chordUtil.setupPageLegend(chordPtr, 0, CPARAM_CHORD_TYPE);
        }

        if (chord_.type == CTYPE_INTERVAL)
        {
            switch (page)
            {
            case MFXCHRDPAGE_SCALES:
            {
                omxDisp.setLegend(0, "GLBL", !useGlobalScale_, "ON");
                omxDisp.setLegend(1, "ROOT", MusicScales::getNoteName(rootNote_));
                omxDisp.setLegend(2, "SCALE", scaleIndex_ < 0, scaleIndex_);
            }
            break;
            case MFXCHRDPAGE_INT_NOTES:
            {
                chordUtil.setupPageLegend(chordPtr, 0, CPARAM_INT_NUMNOTES);
                chordUtil.setupPageLegend(chordPtr, 1, CPARAM_INT_DEGREE);
                chordUtil.setupPageLegend(chordPtr, 2, CPARAM_INT_OCTAVE);
                chordUtil.setupPageLegend(chordPtr, 3, CPARAM_INT_TRANSPOSE);
            }
            break;
            case MFXCHRDPAGE_INT_SPREAD:
            {
                chordUtil.setupPageLegend(chordPtr, 0, CPARAM_INT_SPREAD);
                chordUtil.setupPageLegend(chordPtr, 1, CPARAM_INT_ROTATE);
                chordUtil.setupPageLegend(chordPtr, 2, CPARAM_INT_VOICING);
            }
            break;
            case MFXCHRDPAGE_INT_QUART:
            {
                chordUtil.setupPageLegend(chordPtr, 0, CPARAM_INT_SPRDUPDOWN);
                chordUtil.setupPageLegend(chordPtr, 1, CPARAM_INT_QUARTVOICE);
            }
            break;
            default:
                break;
            }
        }
    }

    int MidiFXChord::saveToDisk(int startingAddress, Storage *storage)
    {
        mfxChordSave chordSave;
        chordSave.chancePerc = chancePerc_;
        chordSave.useGlobalScale = useGlobalScale_;
        chordSave.rootNote = rootNote_;
        chordSave.scaleIndex = scaleIndex_;
        chordSave.chord.CopySettingsFrom(&chord_);

        int saveSize = sizeof(mfxChordSave);

        auto saveBytesPtr = (byte *)(&chordSave);
        for (int j = 0; j < saveSize; j++)
        {
            storage->write(startingAddress + j, *saveBytesPtr++);
        }

        startingAddress += saveSize;

        return startingAddress;
    }

    int MidiFXChord::loadFromDisk(int startingAddress, Storage *storage)
    {
        int saveSize = sizeof(mfxChordSave);

        auto chordSave = mfxChordSave{};
        auto current = (byte *)&chordSave;
        for (int j = 0; j < saveSize; j++)
        {
            *current = storage->read(startingAddress + j);
            current++;
        }

        startingAddress += saveSize;

        chancePerc_ = chordSave.chancePerc;
        useGlobalScale_ = chordSave.useGlobalScale;
        rootNote_ = chordSave.rootNote;
        scaleIndex_ = chordSave.scaleIndex;
        chord_.CopySettingsFrom(&chordSave.chord);

        return startingAddress;
    }
}
