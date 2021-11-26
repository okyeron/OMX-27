#ifndef _INCLUDE_CLEARUI_DISPLAY_H_
#define _INCLUDE_CLEARUI_DISPLAY_H_

#include <Adafruit_SSD1306.h>

#define WHITE SSD1306_WHITE
#define BLACK SSD1306_BLACK

extern Adafruit_SSD1306 display;

void initializeDisplay();

void setRotationSideways();
void setRotationNormal();

void defaultText(int size);
void serifText(int size);
void mono9Text(int size);
void silkText(int size);
void liquidText(int size);
void sans9bText(int size);

void tomText(int size);
void picoText(int size);
void tinyText(int size);
void f5Text(int size);


void centerText(const char* s, int16_t x, int16_t y, uint16_t w, uint16_t h);

void centerNumber(unsigned int n,
	uint16_t x, uint16_t y, uint16_t w, uint16_t h);

template< typename N >
void centerNumber(const N& n, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
	{ centerNumber(static_cast<unsigned int>(n), x, y, w, h); }


bool updateSaver(bool);


void dumpDisplayPBM(Print& stream);


#endif // _INCLUDE_CLEARUI_DISPLAY_H_
