#include "midimacro_deluge.h"
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
		paramBanks[0].bankName = "Filt 1";
		paramBanks[0].SetCCs("FREQ", 74, "RES", 71, "MORP", 70);
		paramBanks[1].bankName = "Filt 2";
		paramBanks[2].bankName = "Env 1";
		paramBanks[3].bankName = "Env 2";

		params_.addPage(1); // 
		params_.addPage(1); // 
		params_.addPage(1); // 
		params_.addPage(1); // 

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
		int8_t selPage = params_.getSelPage();

		if (selPage < 0 || selPage >= 4)
		{
			return nullptr;
		}

		return &paramBanks[selPage];
	}

	void MidiMacroDeluge::onEnabled()
	{
		omxDisp.displayMessage("Deluge");

		auto activeBank = getActiveBank();

		// Update the potPickups to the values of the active bank
		if(activeBank != nullptr)
		{
			for(int8_t i = 0; i < 5; i++)
			{
				potPickups[i].SetVal(activeBank->midiValues[i]);
			}
		}
	}

	void MidiMacroDeluge::onDisabled()
	{
	}

	void MidiMacroDeluge::inMidiControlChange(byte channel, byte control, byte value)
	{
		if(channel == midiMacroConfig.midiMacroChan)
		{
			// delVals[control] = value; // Might want to do this for speed

			int8_t selPage = params_.getSelPage();

			auto activeBank = getActiveBank();

			if(activeBank != nullptr)
			{
				for(int8_t i = 0; i < kNumBanks; i++)
				{
					int8_t paramIndex = paramBanks[i].UpdateCCValue(control, value);

					// CC was found in bank and this is the active bank
					if(paramIndex >= 0 && i == selPage)
					{
						// Update the pot pickup for this index. 
						potPickups[i].SetVal(value);
						omxDisp.displayMessageTimed("CC " + String(control) + " Val " + String(value), 5);
					}
				}
			}

			Serial.println((String)"IN CC: " + control + " VAL: " + value); // 5968
		}
	}

	void MidiMacroDeluge::loopUpdate()
	{
	}

	void MidiMacroDeluge::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
	{
		auto activeBank = getActiveBank();

		if (activeBank != nullptr)
		{
			uint8_t cc = activeBank->midiCCs[potIndex];

			if (cc <= 127 && newValue != prevValue)
			{
				potPickups[potIndex].UpdatePot(prevValue, newValue);

				if(potPickups[potIndex].pickedUp)
				{
					omxDisp.displayMessageTimed(String(activeBank->paramNames[potIndex]) + " " + String(cc) + " " + String(potPickups[potIndex].value), 5);
					MM::sendControlChange(cc, potPickups[potIndex].value, midiMacroConfig.midiMacroChan);
				}
				else 
				{
					omxDisp.displayMessageTimed(String(newValue) + " -> " + String(potPickups[potIndex].value), 5);
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
		int thisKey = e.key();
		// int keyPos = thisKey - 11;

		if (thisKey != 0 && !e.held())
		{
			// Keyboard on right for playing notes
			if ((thisKey >= 6 && thisKey <= 10) || (thisKey >= 19))
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
					if (thisKey == keyEnv1_)
					{
						params_.setSelPage(DELPAGE_ENV1);
					}
					else if (thisKey == keyEnv2_)
					{
						params_.setSelPage(DELPAGE_ENV2);
					}
					else if (thisKey == keyFilt1_)
					{
						params_.setSelPage(DELPAGE_FILT1);
					}
					else if (thisKey == keyFilt2_)
					{
						params_.setSelPage(DELPAGE_FILT2);
					}
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

		strip.setPixelColor(0, BLUE); // aux

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

		omxDisp.setDirty();
	}

	void MidiMacroDeluge::onDisplayUpdate()
	{
		omxDisp.clearLegends();

		auto activeBank = getActiveBank();

		if(activeBank != nullptr)
		{
			omxDisp.dispGenericModeLabel(activeBank->bankName, params_.getNumPages(), params_.getSelPage());
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
