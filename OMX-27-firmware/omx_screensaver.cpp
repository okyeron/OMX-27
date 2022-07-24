#include "omx_screensaver.h"
#include "config.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"

void OmxScreensaver::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
    colorConfig.screensaverColor = potSettings.analog[4]->getValue() * 4; // value is 0-32764 for strip.ColorHSV

    // reset screensaver
    if (potSettings.analog[0]->hasChanged() || potSettings.analog[1]->hasChanged() || potSettings.analog[2]->hasChanged() || potSettings.analog[3]->hasChanged())
    {
        screenSaverCounter = 0;
    }
}

void OmxScreensaver::updateScreenSaverState()
{
    if (screenSaverCounter > screensaverInterval ){
		screenSaverActive = true;
	} else if (screenSaverCounter < 10){
		ssstep = 0;
		ssloop = 0;
//		setAllLEDS(0,0,0);
		screenSaverActive = false;
		nextStepTimeSS = millis();
	} else {
		screenSaverActive = false;
		nextStepTimeSS = millis();
	}
}

bool OmxScreensaver::shouldShowScreenSaver()
{
    return screenSaverActive;
}

void OmxScreensaver::onEncoderChanged(Encoder::Update enc) {

}

void OmxScreensaver::onKeyUpdate(OMXKeypadEvent e)
{
}

void OmxScreensaver::updateLEDs()
{
    unsigned long playstepmillis = millis();
	if (playstepmillis > nextStepTimeSS){ 
		ssstep = ssstep % 16;
		ssloop = ssloop % 16 ;
	
		int j = 26 - ssloop;
		int i = ssstep + 11;

		for (int z=1; z<11; z++){
			strip.setPixelColor(z, 0);
		}
		if (!ssreverse) {
			// turn off all leds
			for (int x=0; x<16; x++){
				if (i < j){
					strip.setPixelColor(x+11, 0);
				}
				if (x+11 > j){
					strip.setPixelColor(x+11, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor)));
				}
			}
			strip.setPixelColor(i+1, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor)));
		} else {
			for (int y=0; y<16; y++){
				if (i >= j){
					strip.setPixelColor(y+11, 0);
				}
				if (y+11 < j){
					strip.setPixelColor(y+11, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor)));
				}
			}
			strip.setPixelColor(i+1, strip.gamma32(strip.ColorHSV(colorConfig.screensaverColor)));
		}
		ssstep++;
		if (ssstep == 16){
			ssloop++;
		}
		if (ssloop == 16){
			ssreverse = !ssreverse;
		}
		nextStepTimeSS = nextStepTimeSS + sleepTick;

        omxLeds.setDirty();
	}
}

void OmxScreensaver::resetCounter()
{
    screenSaverCounter = 0;
}

void OmxScreensaver::onDisplayUpdate()
{
    updateLEDs();
	omxDisp.clearDisplay();
}

