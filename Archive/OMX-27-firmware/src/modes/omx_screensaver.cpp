#include "omx_screensaver.h"
#include "../consts/consts.h"
#include "../config.h"
#include "../utils/omx_util.h"
#include "../hardware/omx_disp.h"
#include "../hardware/omx_leds.h"

// Maps pot value to full HSV hue range (0-65535)
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
	// - Animation state advances on time gate (every 80ms)
	// - LED rendering happens EVERY call for instant color response
	// - Smooth gradient wave animation with optimized RGB scaling
	unsigned long currentMillis = millis();

	// Advance animation state on time gate only
	if (currentMillis >= nextStepTimeSS)
	{
		ssstep++;
		if (ssstep >= 16)
		{
			ssstep = 0;
			ssloop++;
			if (ssloop >= 16)
			{
				ssloop = 0;
			}
		}
		nextStepTimeSS = currentMillis + sleepTick;
	}

	// Always turn off keypad LEDs (1-10)
	for (int z = 1; z < 11; z++)
	{
		strip.setPixelColor(z, 0, 0, 0);
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
		// Calculate base color once, then scale RGB per LED
		const uint32_t baseColor = strip.ColorHSV(colorConfig.screensaverColor, 255, 255);
		const uint8_t baseR = (baseColor >> 16) & 0xFF;
		const uint8_t baseG = (baseColor >> 8) & 0xFF;
		const uint8_t baseB = baseColor & 0xFF;

		// Wave position: 0-255 continuous range, multiply by 4 for visible speed
		const int wavePos = ((ssloop * 16 + ssstep) * 4) & 0xFF;

		// Render smooth traveling gradient wave
		for (int ledNum = 0; ledNum < 16; ledNum++)
		{
			const int ledIndex = ledNum + 11;

			// Calculate phase: creates one wavelength across the 16 LEDs
			int phase = (wavePos + (ledNum * 16)) & 0xFF;

			// Triangle wave for smooth brightness: 0 -> 255 -> 0
			uint8_t brightness;
			if (phase < 128)
			{
				brightness = phase * 2;
			}
			else
			{
				brightness = (255 - phase) * 2;
			}

			// Scale RGB by brightness (fast integer math)
			const uint8_t r = (baseR * brightness) >> 8;
			const uint8_t g = (baseG * brightness) >> 8;
			const uint8_t b = (baseB * brightness) >> 8;

			strip.setPixelColor(ledIndex, strip.gamma32(strip.Color(r, g, b)));
		}
	}

	omxLeds.setDirty();
}