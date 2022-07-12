#include "omx_leds.h"
#include "consts.h"

//  OmxLeds::OmxLeds(){
//     Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//  }

void OmxLeds::initSetup()
{
    strip.begin();                       // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();                        // Turn OFF all pixels ASAP
    strip.setBrightness(LED_BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)
    for (int i = 0; i < LED_COUNT; i++)
    { // For each pixel...
        strip.setPixelColor(i, HALFWHITE);
        strip.show(); // Send the updated pixel colors to the hardware.
        delay(5);     // Pause before next pass through loop
    }
    rainbow(5); // rainbow startup pattern
    delay(500);

    // clear LEDs
    strip.fill(0, 0, LED_COUNT);
    strip.show();

    delay(100);
}

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

void OmxLeds::drawMidiLeds() {
	blinkInterval = step_delay*2;

	if (blink_msec >= blinkInterval){
		blinkState = !blinkState;
		blink_msec = 0;
	}

	if (midiSettings.midiAUX){
		// Blink left/right keys for octave select indicators.
		auto color1 = blinkState ? LIME : LEDOFF;
		auto color2 = blinkState ? MAGENTA : LEDOFF;
		auto color3 = blinkState ? ORANGE : LEDOFF;
		auto color4 = blinkState ? RBLUE : LEDOFF;

		for (int q = 1; q < LED_COUNT; q++){				
			if (midiKeyState[q] == -1){
				if (midiBg_Hue == 0){
					strip.setPixelColor(q, LEDOFF);
				} else if (midiBg_Hue == 32){
					strip.setPixelColor(q, LOWWHITE);
				} else {
					strip.setPixelColor(q, strip.ColorHSV(midiBg_Hue, midiBg_Sat, midiBg_Brightness));
				}
			}
		}
		strip.setPixelColor(0, RED);
		strip.setPixelColor(1, color1);
		strip.setPixelColor(2, color2);
		strip.setPixelColor(11, color3);
		strip.setPixelColor(12, color4);
	// Macros
	} else if (midiMacroConfig.m8AUX){
		auto color5 = blinkState ? ORANGE : LEDOFF;
		auto color6 = blinkState ? RED : LEDOFF;

		strip.setPixelColor(0, BLUE);
		strip.setPixelColor(1, ORANGE); // all mute
		strip.setPixelColor(3, LIME); // MIXER
		strip.setPixelColor(4, CYAN); // snap load
		strip.setPixelColor(5, MAGENTA); // snap save

		for(int m = 11; m < LED_COUNT-8; m++){
			if (m8mutesolo[m-11]){
				strip.setPixelColor(m, color5);
			}else{
				strip.setPixelColor(m, ORANGE);
			}
		}
		
		strip.setPixelColor(6, RED); // all solo
		for(int m = 19; m < LED_COUNT; m++){
			if (m8mutesolo[m-11]){
				strip.setPixelColor(m, color6);
			}else{
				strip.setPixelColor(m, RED);
			}
		}
		strip.setPixelColor(2, LEDOFF);
		strip.setPixelColor(7, LEDOFF);
		strip.setPixelColor(8, LEDOFF);

		strip.setPixelColor(9, YELLOW); // WAVES
		strip.setPixelColor(10, BLUE); // PLAY

	} else {
		// AUX key
		strip.setPixelColor(0, LEDOFF);
		
		// Other keys
		if (!sysSettings.screenSaverMode){
			// clear not held leds
			for (int q = 1; q < LED_COUNT; q++){				
				if (midiKeyState[q] == -1){
					if (midiBg_Hue == 0){
						strip.setPixelColor(q, LEDOFF);
					} else if (midiBg_Hue == 32){
						strip.setPixelColor(q, LOWWHITE);
					} else {
						strip.setPixelColor(q, strip.ColorHSV(midiBg_Hue, midiBg_Sat, midiBg_Brightness));
					}
				}
			}
		}
	}
	dirtyPixels = true;
}

bool OmxLeds::getBlinkState()
{
    return blinkState;
}
bool OmxLeds::getSlowBlinkState()
{
    return slowBlinkState;
}

void OmxLeds::setAllLEDS(int R, int G, int B)
{
    for (int i = 0; i < LED_COUNT; i++)
    { // For each pixel...
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

void OmxLeds::rainbow(int wait)
{
    // Hue of first pixel runs 5 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
    // means we'll make 5*65536/256 = 1280 passes through this outer loop:
    for (long firstPixelHue = 0; firstPixelHue < 1 * 65536; firstPixelHue += 256)
    {
        for (int i = 0; i < strip.numPixels(); i++)
        { // For each pixel in strip...
            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (strip.numPixels() steps):
            int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());

            // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the single-argument hue variant. The result
            // is passed through strip.gamma32() to provide 'truer' colors
            // before assigning to each pixel:
            strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
        }
        strip.show(); // Update strip with new contents
        delay(wait);  // Pause for a moment
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
