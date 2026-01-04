#include "omx_screensaver.h"
#include "../consts/consts.h"
#include "../config.h"
#include "../utils/omx_util.h"
#include "../hardware/omx_disp.h"
#include "../hardware/omx_leds.h"

void OmxScreensaver::setScreenSaverColor()
{
	colorConfig.screensaverColor = map(potSettings.analog[4]->getValue(), potMinVal, potMaxVal, 0, ssMaxColorDepth);
}

void OmxScreensaver::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
	// set screensaver color with pot 4
	if (potSettings.analog[4]->hasChanged())
	{
		setScreenSaverColor();
	}
	// reset screensaver
	if (potSettings.analog[0]->hasChanged() || potSettings.analog[1]->hasChanged() || potSettings.analog[2]->hasChanged() || potSettings.analog[3]->hasChanged())
	{
		screenSaverCounter = 0;
	}
}

void OmxScreensaver::updateScreenSaverState()
{
	if (screenSaverCounter > screensaverInterval)
	{
		if (!screenSaverActive)
		{
			screenSaverActive = true;
			setScreenSaverColor();
		}
	}
	else if (screenSaverCounter < 10)
	{
		ssstep = 0;
		ssloop = 0;
		//		setAllLEDS(0,0,0);
		screenSaverActive = false;
		nextStepTimeSS = millis();
	}
	else
	{
		screenSaverActive = false;
		nextStepTimeSS = millis();
	}
}

bool OmxScreensaver::shouldShowScreenSaver()
{
	return screenSaverActive;
}

void OmxScreensaver::onEncoderChanged(Encoder::Update enc)
{
}

void OmxScreensaver::onKeyUpdate(OMXKeypadEvent e)
{
}

void OmxScreensaver::onDisplayUpdate()
{
	updateLEDs();
	omxDisp.clearDisplay();
}

void OmxScreensaver::resetCounter()
{
	screenSaverCounter = 0;
}

void OmxScreensaver::updateLEDs()
{
	unsigned long playstepmillis = millis();
	if (playstepmillis > nextStepTimeSS)
	{
		ssstep = ssstep % 16;
		ssloop = ssloop % 16;

		int j = 26 - ssloop;
		int i = ssstep + 11;

		// Always turn off "black" keypad LEDs (1-10)
		for (int z = 1; z < 11; z++)
		{
			strip.setPixelColor(z, 0);
		}

		// Check for LED off mode (pot at minimum)
		if (colorConfig.screensaverColor < ssMinThreshold)
		{
			for (int w = 11; w < 27; w++)
			{
				strip.setPixelColor(w, 0, 0, 0);
			}
		}
		else
		{
			if (!ssreverse)
			{
				// turn off all leds
				for (int x = 0; x < 16; x++)
				{
					if (i < j)
					{
						strip.setPixelColor(x + 11, 0);
					}
					if (x + 11 > j)
					{
						strip.setPixelColor(x + 11, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor)));
					}
				}
				strip.setPixelColor(i + 1, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor)));
			}
			else
			{
				for (int y = 0; y < 16; y++)
				{
					if (i >= j)
					{
						strip.setPixelColor(y + 11, 0);
					}
					if (y + 11 < j)
					{
						strip.setPixelColor(y + 11, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor)));
					}
				}
				strip.setPixelColor(i + 1, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor)));
			}
		}

		ssstep++;
		if (ssstep == 16)
		{
			ssloop++;
		}
		if (ssloop == 16)
		{
			ssreverse = !ssreverse;
		}
		nextStepTimeSS = nextStepTimeSS + sleepTick;

		omxLeds.setDirty();
	}
}
