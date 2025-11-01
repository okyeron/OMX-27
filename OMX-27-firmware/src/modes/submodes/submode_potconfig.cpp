
#include "../../globals.h"
#include "submode_potconfig.h"
#include "../../hardware/omx_disp.h"
#include "../../hardware/omx_leds.h"
#include "../../consts/colors.h"

enum PotConfigPage
{
	POTPAGE_1,
	POTPAGE_2,
	POTPAGE_EXIT
};

SubModePotConfig::SubModePotConfig()
{
	params_.addPage(4);
	params_.addPage(4);
	params_.addPage(1); // Exit submode
}

void SubModePotConfig::onEnabled()
{
	params_.setSelPageAndParam(0, 0);
	encoderSelect_ = true;
	omxLeds.setDirty();
	omxDisp.setDirty();

	auxReleased_ = !midiSettings.keyState[0];
}

void SubModePotConfig::onDisabled()
{
	strip.clear();
	omxLeds.setDirty();
	omxDisp.setDirty();
}

void SubModePotConfig::loopUpdate()
{
}

bool SubModePotConfig::updateLEDs()
{
	strip.clear();

	// bool blinkState = omxLeds.getBlinkState();
	// bool blinkStateSlow = omxLeds.getSlowBlinkState();

	// Serial.println("MidiFX Leds");
	// auto auxColor = midiFXParamView_ ? (blinkStateSlow ? ORANGE : LEDOFF) : RED;
	strip.setPixelColor(0, RED);

	// for(uint8_t i = 1; i < 26; i++)
	// {
	//     strip.setPixelColor(i, LEDOFF);
	// }

	for (uint8_t i = 0; i < 5; i++)
	{
		auto bankColor = i == potSettings.potbank ? LTYELLOW : DKGREEN;
		strip.setPixelColor(11 + i, bankColor);
	}

	// if (midiFXParamView_)
	// {
	//     uint8_t selFXType = 0;

	//     if(getMidiFX(selectedMidiFX_) != nullptr)
	//     {
	//         // Serial.println("Selected MidiFX not null");
	//         selFXType = getMidiFX(selectedMidiFX_)->getFXType();
	//     }

	//     for (uint8_t i = 0; i < 8; i++)
	//     {
	//         auto fxColor = (i == selFXType ? GREEN : DKGREEN);

	//         strip.setPixelColor(19 + i, fxColor);
	//     }
	// }

	return true;
}

void SubModePotConfig::onEncoderChanged(Encoder::Update enc)
{
	SubmodeInterface::onEncoderChanged(enc);

	// if (midiFXParamView_)
	// {
	//     if (getMidiFX(selectedMidiFX_) != nullptr)
	//     {
	//         getMidiFX(selectedMidiFX_)->onEncoderChanged(enc);
	//     }
	// }
	// else
	// {
	//     SubmodeInterface::onEncoderChanged(enc);
	// }
}

void SubModePotConfig::onEncoderChangedEditParam(Encoder::Update enc)
{
	auto amt = enc.accel(2); // where 5 is the acceleration factor if you want it, 0 if you don't)

	int8_t selPage = params_.getSelPage(); // Add one for readability
	int8_t selParam = params_.getSelParam() + 1;

	// PAGE ONE
	if (selPage == POTPAGE_1)
	{
		int ccIndex = params_.getSelParam();

		pots[potSettings.potbank][ccIndex] = constrain(pots[potSettings.potbank][ccIndex] + amt, 0, 127);
	}
	else if (selPage == POTPAGE_2)
	{
		if (selParam == 1)
		{
			pots[potSettings.potbank][4] = constrain(pots[potSettings.potbank][4] + amt, 0, 127);
		}
		else if (selParam == 4)
		{
			potSettings.potbank = constrain(potSettings.potbank + amt, 0, NUM_CC_BANKS - 1);
		}
	}

	omxDisp.setDirty();
	omxLeds.setDirty();
}

void SubModePotConfig::onEncoderButtonDown()
{
	if (params_.getSelPage() == POTPAGE_EXIT && params_.getSelParam() == 0)
	{
		setEnabled(false);
	}
	else
	{
		SubmodeInterface::onEncoderButtonDown();
	}

	omxDisp.setDirty();
	omxLeds.setDirty();
}

bool SubModePotConfig::onKeyUpdate(OMXKeypadEvent e)
{
	int thisKey = e.key();
	// auto keyState = midiSettings.keyState;

	if (e.down())
	{
		if (thisKey == 0)
		{
			if (auxReleased_)
			{
				setEnabled(false);
			}
		}

		// Quick Select FX Slot
		if (thisKey >= 11 && thisKey <= 15)
		{
			potSettings.potbank = thisKey - 11;
		}

		// // Change FX type
		// if (midiFXParamView_)
		// {
		//     if (thisKey >= 19 && thisKey < 19 + 8)
		//     {
		//         changeMidiFXType(selectedMidiFX_, thisKey - 19);
		//         // selectMidiFX(thisKey - 19);
		//     }
		// }
	}

	if (!e.down() && thisKey == 0)
	{
		// Used to prevent quickly exiting if entered through aux shortcut.
		auxReleased_ = true;
	}

	omxDisp.setDirty();
	omxLeds.setDirty();

	return true;
}

void SubModePotConfig::setupPageLegends()
{
	omxDisp.clearLegends();

	// omxDisp.dispPage = page + 1;

	int8_t page = params_.getSelPage();

	switch (page)
	{
	case POTPAGE_1:
	{
		omxDisp.legends[0] = "CC 1";
		omxDisp.legends[1] = "CC 2";
		omxDisp.legends[2] = "CC 3";
		omxDisp.legends[3] = "CC 4";
		omxDisp.legendVals[0] = pots[potSettings.potbank][0];
		omxDisp.legendVals[1] = pots[potSettings.potbank][1];
		omxDisp.legendVals[2] = pots[potSettings.potbank][2];
		omxDisp.legendVals[3] = pots[potSettings.potbank][3];
	}
	break;
	case POTPAGE_2:
	{
		omxDisp.legends[0] = "CC 5";
		omxDisp.legends[1] = "";
		omxDisp.legends[2] = "";
		omxDisp.legends[3] = "PBNK";
		omxDisp.legendVals[0] = pots[potSettings.potbank][4];
		omxDisp.legendVals[1] = -127;
		omxDisp.legendVals[2] = -127;
		omxDisp.legendVals[3] = (potSettings.potbank + 1);
		omxDisp.legendText[1] = "";
		omxDisp.legendText[2] = "";
	}
	break;
	case POTPAGE_EXIT:
	{
		omxDisp.legends[0] = "Exit";
		omxDisp.legends[1] = "";
		omxDisp.legends[2] = "";
		omxDisp.legends[3] = "";
		omxDisp.legendVals[0] = -127;
		omxDisp.legendVals[1] = -127;
		omxDisp.legendVals[2] = -127;
		omxDisp.legendVals[3] = -127;
		omxDisp.legendText[0] = "Exit";
		omxDisp.legendText[1] = "";
		omxDisp.legendText[2] = "";
		omxDisp.legendText[3] = "";
	}
	break;
	default:
		break;
	}
}

void SubModePotConfig::onDisplayUpdate()
{
	// omxLeds.updateBlinkStates();

	// if (omxLeds.isDirty())
	// {
	//     updateLEDs();
	// }

	if (omxDisp.isDirty())
	{
		if (!encoderConfig.enc_edit)
		{
			setupPageLegends();
			omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
		}
	}
}
