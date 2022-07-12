#include "omx_leds.h"
#include "consts.h"

//  OmxLeds::OmxLeds(){
//     Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//  }

void OmxLeds::updateLeds()
{
    blinkInterval = clockConfig.step_delay * 2;
    unsigned long slowBlinkInterval = blinkInterval * 2;

    if (blink_msec >= blinkInterval)
    {
        blinkState = !blinkState;
        blink_msec = 0;
    }
    if (slow_blink_msec >= slowBlinkInterval)
    {
        slowBlinkState = !slowBlinkState;
        slow_blink_msec = 0;
    }
}

bool OmxLeds::getBlinkState()
{
    return blinkState;
}
bool OmxLeds::getSlowBlinkState()
{
    return slowBlinkState;
}

void OmxLeds::setAllLEDS(int R, int G, int B) {
	for(int i=0; i<LED_COUNT; i++) { // For each pixel...
		strip.setPixelColor(i, strip.Color(R, G, B));
	}
	setDirty();
}

void OmxLeds::setDirty()
{
    dirtyPixels = true;
}

void OmxLeds::showLeds()
{
	// are pixels dirty
    if (dirtyPixels)
    {
        strip.show();
        dirtyPixels = false;
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t OmxLeds::Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void OmxLeds::colorWipe(byte red, byte green, byte blue, int SpeedDelay)
{
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, strip.Color(red, green, blue));
        strip.show();
        delay(SpeedDelay);
    }
}

// Theatre-style crawling lights with rainbow effect
void OmxLeds::theaterChaseRainbow(uint8_t wait)
{ 
    for (int j = 0; j < 256; j++)
    { // cycle all 256 colors in the wheel
        for (int q = 0; q < 3; q++)
        {
            for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
            {
                strip.setPixelColor(i + q, Wheel((i + j) % 255)); // turn every third pixel on
            }
            strip.show();
            delay(wait);
            for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
            {
                strip.setPixelColor(i + q, 0); // turn every third pixel off
            }
        }
    }
}

void OmxLeds::CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay, int start, int end)
{
    for (int i = start; i < end - EyeSize - 2; i++)
    {
        setAllLEDS(0, 0, 0);
        strip.setPixelColor(i, strip.Color(red / 10, green / 10, blue / 10));
        for (int j = 1; j <= EyeSize; j++)
        {
            strip.setPixelColor(i + j, strip.Color(red, green, blue));
        }
        strip.setPixelColor(i + EyeSize + 1, strip.Color(red / 10, green / 10, blue / 10));
        strip.show();
        delay(SpeedDelay);
    }
    delay(ReturnDelay);
    for (int i = end - EyeSize - 2; i > start; i--)
    {
        setAllLEDS(0, 0, 0);
        strip.setPixelColor(i, strip.Color(red / 10, green / 10, blue / 10));
        for (int j = 1; j <= EyeSize; j++)
        {
            strip.setPixelColor(i + j, strip.Color(red, green, blue));
        }
        strip.setPixelColor(i + EyeSize + 1, strip.Color(red / 10, green / 10, blue / 10));
        strip.show();
        delay(SpeedDelay);
    }
    delay(ReturnDelay);
}

void OmxLeds::RolandFill(byte red, byte green, byte blue, int start, int end, int SpeedDelay)
{
    for (uint16_t j = end; j > start; j--)
    {
        for (uint16_t i = start; i < end; i++)
        {
            strip.setPixelColor(i, strip.Color(red, green, blue));
            strip.show();
            if (i < j)
            {
                strip.setPixelColor(i, 0);
            }
            if (!sysSettings.screenSaverMode)
            {
                return;
            }
        }
        strip.setPixelColor(j, strip.Color(red, green, blue));
        strip.show();
    }
    for (uint16_t j = end; j > start - 1; j--)
    {
        for (uint16_t i = start; i < end + 1; i++)
        {
            strip.setPixelColor(i, strip.Color(red, green, blue));
            strip.show();
            if (i > j)
            {
                strip.setPixelColor(i, 0);
            }
            if (!sysSettings.screenSaverMode)
            {
                return;
            }
        }
        if (j != start)
        {
            strip.setPixelColor(j, strip.Color(red, green, blue));
        }
        strip.show();
    }
}

OmxLeds omxLeds;
