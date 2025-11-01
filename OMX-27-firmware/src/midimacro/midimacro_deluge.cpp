#include "midimacro_deluge.h"
#include "../globals.h"
#include "../utils/omx_util.h"
#include "../hardware/omx_disp.h"
#include "../hardware/omx_leds.h"
#include "../midi/midi.h"
#include "../consts/consts.h"
#include "../consts/colors.h"

namespace midimacro
{
	// uint8_t delCCBanks[4][5] = {
	// 	{74, 71, 70, 0, 0},
	// 	{0, 0, 0, 0, 0},
	// 	{0, 0, 0, 0, 0},
	// 	{0, 0, 0, 0, 0},
	// };

	// const char *delParams[4][5] = {
	// 	{"FREQ", "RES", "MORP", "", ""},
	// 	{"", "", "", "", ""},
	// 	{"", "", "", "", ""},
	// 	{"", "", "", "", ""},
	// };

	// const char *pageNames[4] = {"Filt 1", "Filt 2", "Env 1", "Env 2"};

	enum DelugePage
	{
		DELPAGE_FILT1,
		DELPAGE_FILT2,
		DELPAGE_ENV1,
		DELPAGE_ENV2
	};

	MidiMacroDeluge::MidiMacroDeluge()
	{
		// Top Row Banks
		paramBanks[0].bankName = "Env 1";
		paramBanks[0].keyMap = 1;
		paramBanks[0].keyColor = BLUE;
		paramBanks[0].SetCCs("Attack", 73, "Decay", 75, "Sustain", 76, "Release", 72);

		paramBanks[1].bankName = "Env 2";
		paramBanks[1].keyMap = 2;
		paramBanks[1].keyColor = BLUE;
		paramBanks[1].SetCCs("Attack", 77, "Decay", 78, "Sustain", 79, "Release", 80);


		paramBanks[2].bankName = "LPF";
		paramBanks[2].keyMap = 3;
		paramBanks[2].keyColor = RED;
		paramBanks[2].SetCCs("Freq", 74, "Res", 71, "Morph", 70);

		paramBanks[3].bankName = "HPF";
		paramBanks[3].keyMap = 4;
		paramBanks[3].keyColor = RED;
		paramBanks[3].SetCCs("Freq", 81, "Res", 82, "Morph", 83);

		paramBanks[4].bankName = "EQ";
		paramBanks[4].keyMap = 5;
		paramBanks[4].keyColor = RED;
		paramBanks[4].SetCCs("Bas Freq", 84, "Bass LVL", 86, "Treb Freq", 85, "Treb LVL", 87);

		// Bot Row Banks
		paramBanks[5].bankName = "Master";
		paramBanks[5].keyMap = 11;
		paramBanks[5].keyColor = GREEN;
		paramBanks[5].SetCCs("Pan", 10, "Transp", 3, "Porta", 5, "", 255, "Level", 7);

		// OSC1 and FM1 Mapped to same key with toggle
		paramBanks[6].bankName = "OSC 1";
		paramBanks[6].keyMap = 12;
		paramBanks[6].altBank = false;
		paramBanks[6].keyColor = ROSE;
		paramBanks[6].SetCCs("Level", 21, "Transp", 12, "PW", 23, "FM Fdbk", 24, "WT Morph", 25);

		paramBanks[7].bankName = "FM 1";
		paramBanks[7].keyMap = 12;
		paramBanks[7].altBank = true;
		paramBanks[7].keyColor = ROSE;
		paramBanks[7].SetCCs("Level", 54, "Transp", 14, "Feedback", 55);

		// OSC2 and FM2 Mapped to same key with toggle
		paramBanks[8].bankName = "OSC 2";
		paramBanks[8].keyMap = 13;
		paramBanks[8].altBank = false;
		paramBanks[8].keyColor = ROSE;
		paramBanks[8].SetCCs("Level", 26, "Transp", 13, "PW", 28, "FM Fdbk", 29, "WT Morph", 30);

		paramBanks[9].bankName = "FM 2";
		paramBanks[9].keyMap = 13;
		paramBanks[9].altBank = true;
		paramBanks[9].keyColor = ROSE;
		paramBanks[9].SetCCs("Level", 56, "Transp", 15, "Feedback", 57);

		paramBanks[10].bankName = "LFO Delay Reverb";
		paramBanks[10].keyMap = 14;
		paramBanks[10].keyColor = ORANGE;
		paramBanks[10].SetCCs("LFO1 Rate", 58, "LFO2 Rate", 59, "DEL Rate", 53, "Delay", 52, "Reverb", 91);

		paramBanks[11].bankName = "ModFX";
		paramBanks[11].keyMap = 14;
		paramBanks[11].altBank = true;
		paramBanks[11].keyColor = MAGENTA;
		paramBanks[11].SetCCs("Rate", 16, "Depth", 93, "Feedback", 17, "Offset", 18);

		paramBanks[12].bankName = "Distortion Noise";
		paramBanks[12].keyMap = 15;
		paramBanks[12].keyColor = RED;
		paramBanks[12].SetCCs("Bitcrush", 62, "Decimate", 63, "Wavefold", 19, "Noise", 41);

		paramBanks[13].bankName = "Arp Sidechain";
		paramBanks[13].keyMap = 16;
		paramBanks[13].keyColor = LIME;
		paramBanks[13].SetCCs("Arp Rate", 51, "Arp Gate", 50, "Vol Duck", 61, "SC Shape", 60);

		paramBanks[14].bankName = "Custom 1";
		paramBanks[14].keyMap = 17;
		paramBanks[14].keyColor = ORANGE;
		paramBanks[14].SetCCs("Pot 1", 100, "Pot 2", 101, "Pot 3", 102, "Pot 4", 103, "Pot 5", 104);

		paramBanks[15].bankName = "Custom 2";
		paramBanks[15].keyMap = 17;
		paramBanks[15].altBank = true;
		paramBanks[15].keyColor = ORANGE;
		paramBanks[15].SetCCs("Pot 1", 105, "Pot 2", 106, "Pot 3", 107, "Pot 4", 108, "Pot 5", 109);


		params_.addPage(5); //
		// params_.addPage(1); //
		// params_.addPage(1); //
		// params_.addPage(1); //

		for(uint8_t i = 0; i < 127; i++)
		{
			delVals[i] = 0;
		}

		encoderSelect_ = true;
	}

	String MidiMacroDeluge::getName()
	{
		return String("DELUGE");
	}

	MidiParamBank *MidiMacroDeluge::getActiveBank()
	{
		// int8_t selPage = params_.getSelPage();

		// if (selPage < 0 || selPage >= 4)
		// {
		// 	return nullptr;
		// }

		return &paramBanks[selBank];
	}

	void MidiMacroDeluge::keyDownBankShortcut(uint8_t keyIndex)
	{
		auto activeBank = getActiveBank();

		bool selAltBank = false;

		if(activeBank->keyMap == keyIndex)
		{
			// If the active bank's keyMap matches this key then we have opprotunity to select an alt bank if one exists
			// If the active bank is an altbank, then the main level bank will be selected
			// TLDR: Pressing a key multiple times can toggle between different banks
			selAltBank = activeBank->altBank == false;
		}

		for(uint8_t i = 0; i < kNumBanks; i++)
		{
			if(paramBanks[i].keyMap == keyIndex && paramBanks[i].altBank == selAltBank)
			{
				setActiveBank(i);
				return;
			}
		}
	}

	void MidiMacroDeluge::setActiveBank(uint8_t bankIndex)
	{
		if (bankIndex >= kNumBanks)
		{
// 			Serial.println((String)"ERROR:MidiMacroDeluge: Cannot set active bank to " + bankIndex);
			return;
		}

		if (bankIndex != selBank)
		{
			// save/load activeParam to bank
			paramBanks[selBank].activeParam = activeParam;
			selBank = bankIndex;
			activeParam = paramBanks[selBank].activeParam;

			if(params_.getSelPage() == 0)
			{
				params_.setSelParam(activeParam);
			}

			updatePotPickups();
			omxDisp.setDirty();
			omxLeds.setDirty();
		}
	}

	// Updates the pot pickups to the values saved in the active bank
	// Thus if we switch banks, the value will need to be picked up
	// by the pot before it sends out to avoid jumping values.
	void MidiMacroDeluge::updatePotPickups()
	{
		auto activeBank = getActiveBank();

		// Update the potPickups to the values of the active bank
		if(activeBank != nullptr)
		{
			for(int8_t i = 0; i < 5; i++)
			{
				potPickups[i].SetVal(activeBank->midiValues[i], false);
				potPickups[i].SaveRevertVal();
			}
		}
	}

	// Reverts the values of current bank and sends update via midiCC
	// Revert value is saved when switching banks or when a new value comes in through midi
	void MidiMacroDeluge::revertPotPickups()
	{
		auto activeBank = getActiveBank();

		for(int8_t i = 0; i < 5; i++)
		{
			if(activeBank->midiCCs[i] < 128)
			{
				potPickups[i].RevertVal();
				MM::sendControlChange(activeBank->midiCCs[i], potPickups[i].value, midiMacroConfig.midiMacroChan);
			}
		}
	}

	void MidiMacroDeluge::onEnabled()
	{
		omxDisp.displayMessage("Deluge");

		auxDown_ = false;

		updatePotPickups();
	}

	void MidiMacroDeluge::onDisabled()
	{
	}

	void MidiMacroDeluge::inMidiControlChange(byte channel, byte control, byte value)
	{
		if(channel == midiMacroConfig.midiMacroChan)
		{
			// delVals[control] = value; // Might want to do this for speed

			// auto activeBank = getActiveBank();

			// if(activeBank != nullptr)
			// {
			// 	// activeBank->UpdateCCValue(control, value);

			// 	int8_t paramIndex = activeBank->UpdateCCValue(control, value);

			// 	// CC was found in bank and this is the active bank
			// 	if (paramIndex >= 0)
			// 	{
			// 		// Update the pot pickup for this index.
			// 		potPickups[paramIndex].SetVal(value);
			// 		// omxDisp.displayMessageTimed("CC " + String(control) + " Val " + String(value), 5);
			// 	}
			// }

			for (uint8_t i = 0; i < kNumBanks; i++)
			{
				int8_t paramIndex = paramBanks[i].UpdateCCValue(control, value);

				// CC was found in bank and this is the active bank
				if (paramIndex >= 0 && i == selBank)
				{
					// Update the pot pickup for this index.
					potPickups[paramIndex].SetVal(value, true);
					omxDisp.setDirty();
					// omxDisp.displayMessageTimed("CC " + String(control) + " Val " + String(value), 5);
				}
			}

			// int8_t selPage = params_.getSelPage();

			// auto activeBank = getActiveBank();

			// if(activeBank != nullptr)
			// {

			// }

			// Serial.println((String)"IN CC: " + control + " VAL: " + value); // 5968
		}
	}

	void MidiMacroDeluge::loopUpdate()
	{
	}

	void MidiMacroDeluge::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
	{
		// omxDisp.displayMessageTimed("Pot Index " + String(potIndex), 5);

		auto activeBank = getActiveBank();

		if (activeBank != nullptr)
		{
			uint8_t cc = activeBank->midiCCs[potIndex];

			if (cc <= 127 && newValue != prevValue)
			{
				potPickups[potIndex].UpdatePot(prevValue, newValue);
				activeBank->UpdatePotValue(potIndex, potPickups[potIndex].value);

				uint8_t oldValDel = (uint8_t)map(prevValue, 0, 127, 0, 50);
				uint8_t newValDel = (uint8_t)map(newValue, 0, 127, 0, 50);

				if (newValDel != oldValDel)
				{
					activeParam = potIndex;
				}

				if(potPickups[potIndex].pickedUp)
				{
					// omxDisp.displayMessageTimed(String(activeBank->paramNames[potIndex]) + " " + String(cc) + " " + String(potPickups[potIndex].value), 5);

					// uint8_t delugeMapVal = (uint8_t)map(potPickups[potIndex].value, 0, 127, 0, 50);

					// omxDisp.displayMessageTimed(String(activeBank->paramNames[potIndex]) + " " + String(delugeMapVal), 5);
					MM::sendControlChange(cc, potPickups[potIndex].value, midiMacroConfig.midiMacroChan);
				}
				else
				{
					// uint8_t delugeMapNewVal = (uint8_t)map(newValue, 0, 127, 0, 50);
					// uint8_t delugeMapVal = (uint8_t)map(potPickups[potIndex].value, 0, 127, 0, 50);
					// omxDisp.displayMessageTimed(String(delugeMapNewVal) + " -> " + String(delugeMapVal), 5);
				}
			}
		}

		// uint8_t oldV = manStrumSensit_;
		// 	manStrumSensit_ = (uint8_t)map(newValue, 0, 127, 1, 32);
		// 	if (manStrumSensit_ != oldV)
		// 	{
		// 		omxDisp.displayMessageTimed("Sens: " + String(manStrumSensit_), 5);
		// 	}

		// omxUtil.sendPots(potIndex, midiMacroConfig.midiMacroChan);
	}

	void MidiMacroDeluge::onKeyUpdate(OMXKeypadEvent e)
	{
		uint8_t thisKey = e.key();
		// int keyPos = thisKey - 11;

		// AUX KEY
		if (thisKey == 0)
		{
			if (lockAuxView_ && auxDown_ && e.down() && !e.held())
			{
				// unlock aux lock when pressing aux again
				lockAuxView_ = false;
				auxDown_ = true;
			}
			else if (lockAuxView_ && auxDown_ && !e.down())
			{
				auxDown_ = true; // Aux Down stays valid until pushed again
			}
			else
			{
				auxDown_ = e.down();
			}

			omxLeds.setDirty();
			omxDisp.setDirty();
			return;
		}

		// key is on the right side
		bool keyOnRight = (thisKey >= 6 && thisKey <= 10) || (thisKey >= 19);

		if(auxDown_ && !keyOnRight)
		{
			if (!e.held())
			{
				if (e.down())
				{
					if (thisKey >= 1 && thisKey <= 5)
					{
						uint8_t paramIndex = thisKey - 1;
						auto activeBank = getActiveBank();

						if (activeBank->HasParamAtIndex(paramIndex))
						{
							activeParam = paramIndex;

							omxLeds.setDirty();
							omxDisp.setDirty();
						}
					}
					// Change Octave
					else if (thisKey == 11 || thisKey == 12)
					{
						int amt = thisKey == 11 ? -1 : 1;
						midiSettings.octave = constrain(midiSettings.octave + amt, -5, 4);
					}
					// Lock AUX
					else if (thisKey == 14)
					{
						lockAuxView_ = !lockAuxView_;

						if(lockAuxView_)
						{
							omxDisp.displayMessage("Locked AUX");
						}
						else if(midiSettings.keyState[0] == false)
						{
							auxDown_ = false;
						}
					}
					// Revert Values
					else if(thisKey == 18)
					{
						omxDisp.displayMessage("Revert Vals");
						revertPotPickups();
					}
				}
				else
				{
				}
			}

			return;
		}

		if (!e.held())
		{
			// Keyboard on right for playing notes
			if (keyOnRight)
			{
				if (e.down())
				{
					DoNoteOn(thisKey);
				}
				else
				{
					DoNoteOff(thisKey);
				}
			}
			else
			{
				if (e.down())
				{
					// omxDisp.displayMessageTimed("Key Down " + String(thisKey), 5);

					keyDownBankShortcut(thisKey);
					// if (thisKey == keyEnv1_)
					// {
					// 	params_.setSelPage(DELPAGE_ENV1);
					// }
					// else if (thisKey == keyEnv2_)
					// {
					// 	params_.setSelPage(DELPAGE_ENV2);
					// }
					// else if (thisKey == keyFilt1_)
					// {
					// 	params_.setSelPage(DELPAGE_FILT1);
					// }
					// else if (thisKey == keyFilt2_)
					// {
					// 	params_.setSelPage(DELPAGE_FILT2);
					// }
				}
				else
				{
				}
			}
		}

		omxLeds.setDirty();
	}

	void MidiMacroDeluge::drawLEDs()
	{
		// omxLeds.updateBlinkStates();

		if (omxLeds.isDirty() == false)
		{
			return;
		}

		auto blinkState = omxLeds.getBlinkState();

		omxLeds.setAllLEDS(0, 0, 0);

		strip.setPixelColor(0, (auxDown_ && blinkState) ? WHITE : BLUE); // aux

		// strip.setPixelColor(but1_, midiSettings.keyState[but1_] ? LTYELLOW : ORANGE);
		// strip.setPixelColor(but2_, midiSettings.keyState[but2_] ? LTYELLOW : ORANGE);
		// strip.setPixelColor(but3_, midiSettings.keyState[but3_] ? LTYELLOW : ORANGE);

		// strip.setPixelColor(enc1_, RED);
		// strip.setPixelColor(enc2_, RED);
		// strip.setPixelColor(enc3_, RED);

		// strip.setPixelColor(keyUp_, midiSettings.keyState[keyUp_] ? LTCYAN : BLUE);
		// strip.setPixelColor(keyDown_, midiSettings.keyState[keyDown_] ? LTCYAN : BLUE);
		// strip.setPixelColor(keyLeft_, midiSettings.keyState[keyLeft_] ? LTCYAN : BLUE);
		// strip.setPixelColor(keyRight_, midiSettings.keyState[keyRight_] ? LTCYAN : BLUE);

		for (int q = 6; q < LED_COUNT; q++)
		{
			if ((q >= 6 && q <= 10) || (q >= 19))
			{
				if (midiSettings.midiKeyState[q] == -1)
				{
					if (colorConfig.midiBg_Hue == 0)
					{
						strip.setPixelColor(q, omxLeds.getKeyColor(scale_, q)); // set off or in scale
					}
					else if (colorConfig.midiBg_Hue == 32)
					{
						strip.setPixelColor(q, LOWWHITE);
					}
					else
					{
						strip.setPixelColor(q, strip.ColorHSV(colorConfig.midiBg_Hue, colorConfig.midiBg_Sat, colorConfig.midiBg_Brightness));
					}
				}
				else
				{
					strip.setPixelColor(q, MIDINOTEON);
				}
			}
		}

		if (auxDown_)
		{
			for (int8_t i = 1; i <= 5; i++)
			{
				int8_t pIndex = i - 1;
				auto activeBank = getActiveBank();

				if (activeBank->HasParamAtIndex(pIndex))
				{
					strip.setPixelColor(i, pIndex == activeParam ? WHITE : ORANGE);
				}
			}

			omxLeds.drawOctaveKeys(11, 12, midiSettings.octave);

			strip.setPixelColor(14, lockAuxView_ ? PINK : MAGENTA);

			// Revert Values Key
			strip.setPixelColor(18, midiSettings.keyState[18] ? LTCYAN : RED);

		}
		else
		{
			for (int8_t i = 0; i < kNumBanks; i++)
			{
				if (i == selBank)
				{
					strip.setPixelColor(paramBanks[i].keyMap, paramBanks[i].altBank ? LTYELLOW : WHITE);
				}
				else
				{
					if (paramBanks[i].altBank == false)
					{
						strip.setPixelColor(paramBanks[i].keyMap, paramBanks[i].keyColor);
					}
				}
			}
		}
	}

	void MidiMacroDeluge::onEncoderChangedSelectParam(Encoder::Update enc)
	{
		params_.changeParam(enc.dir());

		if(params_.getSelPage() == 0)
		{
			activeParam = params_.getSelParam();
		}

		omxDisp.setDirty();
	}

	void MidiMacroDeluge::onEncoderChangedEditParam(Encoder::Update enc)
	{
		// int8_t page = params_.getSelPage();
		// // int8_t param = params_.getSelParam();

		// // auto amt = enc.accel(5);

		// uint8_t encCC = 0;

		// if (page == NRNPAGE_ENC1)
		// 	encCC = ccEnc1_;
		// else if (page == NRNPAGE_ENC2)
		// 	encCC = ccEnc2_;
		// else if (page == NRNPAGE_ENC3)
		// 	encCC = ccEnc3_;

		// if (enc.dir() > 0)
		// {
		// 	MM::sendControlChange(encCC, 65, midiMacroConfig.midiMacroChan);
		// }
		// else if (enc.dir() < 0)
		// {
		// 	MM::sendControlChange(encCC, 63, midiMacroConfig.midiMacroChan);
		// }

		auto amt = enc.accel(1);

		auto activeBank = getActiveBank();

		if(activeBank->HasParamAtIndex(activeParam))
		{
			uint8_t newValue = constrain(potPickups[activeParam].value + amt, 0, 127);
			potPickups[activeParam].SetVal(newValue, false);
			activeBank->UpdatePotValue(activeParam, potPickups[activeParam].value);
			MM::sendControlChange(activeBank->midiCCs[activeParam], potPickups[activeParam].value, midiMacroConfig.midiMacroChan);
		}

		omxDisp.setDirty();
	}

	void MidiMacroDeluge::onDisplayUpdate()
	{
		omxDisp.clearLegends();

		auto activeBank = getActiveBank();

		if(activeBank != nullptr)
		{
		// 	auto activeBank = getActiveBank();

		// if (activeBank != nullptr)
		// {
		// 	uint8_t cc = activeBank->midiCCs[potIndex];

		// 	if (cc <= 127 && newValue != prevValue)
		// 	{
		// 		potPickups[potIndex].UpdatePot(prevValue, newValue);
		// 		activeBank->UpdatePotValue(potIndex, potPickups[potIndex].value);

		// 		activeParam = potIndex;

		// 		if(potPickups[potIndex].pickedUp)
		// 		{
		// 			// omxDisp.displayMessageTimed(String(activeBank->paramNames[potIndex]) + " " + String(cc) + " " + String(potPickups[potIndex].value), 5);

		// 			uint8_t delugeMapVal = (uint8_t)map(potPickups[potIndex].value, 0, 127, 0, 50);

		// 			// omxDisp.displayMessageTimed(String(activeBank->paramNames[potIndex]) + " " + String(delugeMapVal), 5);
		// 			MM::sendControlChange(cc, potPickups[potIndex].value, midiMacroConfig.midiMacroChan);
		// 		}
		// 		else
		// 		{
		// 			uint8_t delugeMapNewVal = (uint8_t)map(newValue, 0, 127, 0, 50);
		// 			uint8_t delugeMapVal = (uint8_t)map(potPickups[potIndex].value, 0, 127, 0, 50);
		// 			// omxDisp.displayMessageTimed(String(delugeMapNewVal) + " -> " + String(delugeMapVal), 5);
		// 		}
		// 	}
		// }

		uint8_t delugeMapPotVal = (uint8_t)map(potPickups[activeParam].potValue, 0, 127, 0, 50);
		uint8_t delugeMapVal = (uint8_t)map(potPickups[activeParam].value, 0, 127, 0, 50);

		omxDisp.dispParamBar(delugeMapPotVal, delugeMapVal, 0, 50, potPickups[activeParam].pickedUp, false, activeBank->bankName, activeBank->paramNames[activeParam]);

			// omxDisp.dispGenericModeLabel(activeBank->bankName, params_.getNumPages(), params_.getSelPage());
		}
		else
		{
			omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
		}

		// int8_t page = params_.getSelPage();

		// bool genericDisp = true;

		// switch (page)
		// {
		// case DELPAGE_FILT1:
		// case DELPAGE_FILT2:
		// case DELPAGE_ENV1:
		// case DELPAGE_ENV2:
		// {
		// 	omxDisp.dispGenericModeLabel(pageNames[page], params_.getNumPages(), params_.getSelPage());
		// 	genericDisp = false;
		// }
		// break;
		// default:
		// 	break;
		// }

		// if (genericDisp)
		// {
		// 	omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
		// }
	}
}
