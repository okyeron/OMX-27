#include "ClearUI_Display.h"

#include <initializer_list>
#include <Adafruit_GFX.h>
#include <Wire.h>

#include "../consts/consts.h"

#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/TomThumb.h>
#include <Fonts/Picopixel.h>
#include <Fonts/Tiny3x3a2pt7b.h>
#include <Fonts/Org_01.h>
// #include "fonts/slkscr7pt7b.h"
// #include "fonts/liquid_7pt7b.h"

#define FONTSANS FreeSansBold9pt7b
#define FONTMONO9 FreeMono9pt7b
#define FONTSANS12 FreeSans12pt7b
#define FONTSANS9BOLD FreeSansBold9pt7b
#define FONTSANS12BOLD FreeSansBold12pt7b
#define FONT FreeSerifBold9pt7b
#define FONT_TT TomThumb
#define FONT_PICO Picopixel
#define FONT_TINY Tiny3x3a2pt7b
#define FONT5 Org_01
#define FONTSILK slkscr7pt7b
#define FONTLIQUID liquid_7pt7b

#define DIGIT_WIDTH 9
#define DIGIT_HEIGHT 12

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32
#define OLED_RST -1
#define CLKDURING 1000000
#define CLKAFTER 400000

#if BOARDTYPE == OMX2040
Adafruit_SSD1306 display = Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire1, OLED_RST, CLKDURING, CLKAFTER);
#else
Adafruit_SSD1306 display = Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, OLED_RST, CLKDURING, CLKAFTER);
#endif

void initializeDisplay()
{
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

	display.cp437();
	setRotationNormal();
}

// Display is mounted upside down on PCB
void setRotationNormal()
{
	display.setRotation(2);
}

void setRotationSideways()
{
	display.setRotation(1);
}

void defaultText(int size)
{
	display.setTextSize(size);
	display.setFont();
}

void serifText(int size)
{
	display.setTextSize(size);
	display.setFont(&FONT);
	display.setTextColor(WHITE);
	display.setTextWrap(false);
}
void mono9Text(int size)
{
	display.setTextSize(size);
	display.setFont(&FONTMONO9);
	display.setTextColor(WHITE);
	display.setTextWrap(false);
}
// void silkText(int size) {
//   display.setTextSize(size);
//   display.setFont(&FONTSILK);
//   display.setTextColor(WHITE);
//   display.setTextWrap(false);
// }
// void liquidText(int size) {
//   display.setTextSize(size);
//   display.setFont(&FONTSILK);
//   display.setTextColor(WHITE);
//   display.setTextWrap(false);
// }
void sans9bText(int size)
{
	display.setTextSize(size);
	display.setFont(&FONTSANS9BOLD);
	display.setTextColor(WHITE);
	display.setTextWrap(false);
}
// void sans12bText(int size) {
//   display.setTextSize(size);
//   display.setFont(&FONTSANS12BOLD);
//   display.setTextColor(WHITE);
//   display.setTextWrap(false);
// }

void tinyText(int size)
{
	display.setTextSize(size);
	display.setFont(&FONT_TINY);
	display.setTextColor(WHITE);
	display.setTextWrap(false);
}
void tomText(int size)
{
	display.setTextSize(size);
	display.setFont(&FONT_TT);
	display.setTextColor(WHITE);
	display.setTextWrap(false);
}
void picoText(int size)
{
	display.setTextSize(size);
	display.setFont(&FONT_PICO);
	display.setTextColor(WHITE);
	display.setTextWrap(false);
}
void f5Text(int size)
{
	display.setTextSize(size);
	display.setFont(&FONT5);
	display.setTextColor(WHITE);
	display.setTextWrap(false);
}

void centerText(const char *s, int16_t x, int16_t y, uint16_t w, uint16_t h)
{
	int16_t bx, by;
	uint16_t bw, bh;

	display.getTextBounds(s, x, y, &bx, &by, &bw, &bh);
	display.setCursor(
		x + (x - bx) + (w - bw) / 2,
		y + (y - by) + (h - bh) / 2);
	display.print(s);
}

void centerNumber(unsigned int n,
				  uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	char buf[8];
	utoa(n, buf, 10);
	centerText(buf, x, y, w, h);
}

namespace
{
	const unsigned long saverStartDelay = 15 * 60 * 1000;

	unsigned long saverStartAt = 0;
	bool saverRunning = false;

	size_t savedSize = 0;
	uint8_t *savedDisplay = NULL;

	unsigned long saverBumpTime;
	int16_t saverPhase;

	const unsigned char wipePattern[] = {  // 24 x 32
		0b00000010, 0b10101011, 0b11111111,
		0b00000010, 0b10101011, 0b11111111,
		0b00000010, 0b10101011, 0b11111111,
		0b00000101, 0b01010111, 0b11111110,
		0b00000101, 0b01010111, 0b11111110,
		0b00000101, 0b01010111, 0b11111110,
		0b00000101, 0b01010111, 0b11111110,
		0b00000101, 0b01010111, 0b11111110,
		0b00000101, 0b01010111, 0b11111110,
		0b00001010, 0b10101111, 0b11111100,
		0b00001010, 0b10101111, 0b11111100,
		0b00001010, 0b10101111, 0b11111100,
		0b00001010, 0b10101111, 0b11111100,
		0b00001010, 0b10101111, 0b11111100,
		0b00010101, 0b01011111, 0b11111000,
		0b00010101, 0b01011111, 0b11111000,
		0b00010101, 0b01011111, 0b11111000,
		0b00010101, 0b01011111, 0b11111000,
		0b00010101, 0b01011111, 0b11111000,
		0b00101010, 0b10111111, 0b11110000,
		0b00101010, 0b10111111, 0b11110000,
		0b00101010, 0b10111111, 0b11110000,
		0b00101010, 0b10111111, 0b11110000,
		0b00101010, 0b10111111, 0b11110000,
		0b00101010, 0b10111111, 0b11110000,
		0b01010101, 0b01111111, 0b11100000,
		0b01010101, 0b01111111, 0b11100000,
		0b01010101, 0b01111111, 0b11100000,
		0b01010101, 0b01111111, 0b11100000,
		0b01010101, 0b01111111, 0b11100000,
		0b10101010, 0b11111111, 0b11000000,
		0b10101010, 0b11111111, 0b11000000,
	};
}

bool updateSaver(bool redrawn)
{
	auto now = millis();

	if (redrawn)
	{
		saverStartAt = now + saverStartDelay;
		saverRunning = false;
		return false;
	}

	if (now < saverStartAt)
		return false;

	if (!saverRunning)
	{
		if (!savedDisplay)
		{
			savedSize = DISPLAY_WIDTH * ((DISPLAY_HEIGHT + 7) / 8);
			savedDisplay = (uint8_t *)malloc(savedSize);
		}
		memcpy(savedDisplay, display.getBuffer(), savedSize);
		saverRunning = true;
		saverPhase = 0;
		saverBumpTime = now - 1;
	}

	if (now > saverBumpTime)
	{
		display.clearDisplay();
		display.drawBitmap(saverPhase - 24, 0, wipePattern, 24, 32, WHITE);

		auto d = display.getBuffer();
		auto s = savedDisplay;
		for (auto n = savedSize; n > 0; --n)
			*d++ &= *s++;

		display.display();

		saverPhase += 2;
		if (saverPhase >= DISPLAY_WIDTH + 24)
		{
			saverPhase = 0;
			saverBumpTime += 2000; // pause between swipes
		}
		saverBumpTime += 50; // speed of swipe
	}
	return true;
}

void dumpDisplayPBM(Print &stream)
{
	stream.println("");
	stream.println("P1");

	auto w = display.width();
	auto h = display.height();

	stream.print(w);
	stream.print(' ');
	stream.println(h);

	for (auto j = 0; j < h; ++j)
	{
		for (auto i = 0; i < w; ++i)
		{
			stream.print(display.getPixel(i, j) == WHITE ? " 0" : " 1");
			// 1 is black in PBM
		}
		stream.println("");
	}
	stream.println("");
}
