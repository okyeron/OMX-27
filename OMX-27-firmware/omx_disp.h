#pragma once
#include "config.h"

// MESSAGE DISPLAY
const int MESSAGE_TIMEOUT_US = 500000;

class OmxDisp
{
public:
    // Should make into function
    const char* legends[4] = {"","","",""};
	int legendVals[4] = {0,0,0,0};
	int dispPage = 0;
	const char* legendText[4] = {"","","",""};

    OmxDisp();
    void setup();
    void clearDisplay();
    void drawStartupScreen();
    void displayMessage(const char *msg);
    void displayMessagef(const char *fmt, ...);

    void dispGridBoxes();
    void invertColor(bool flip);
    void dispValBox(int v, int16_t n, bool inv);
    void dispSymbBox(const char *v, int16_t n, bool inv);
    void dispGenericMode(int submode, int selected);
    void dispPageIndicators(int page, bool selected);
    void dispMode();

    void testdrawrect();
    void drawLoading();

    void setDirty();

    void showDisplay();

    void bumpDisplayTimer();

    void clearLegends();
    void setSubmode(int submode);

    void UpdateMessageTextTimer();
private:
    int hline = 8;
    int messageTextTimer = 0;
    bool dirtyDisplay = false;

    elapsedMillis dirtyDisplayTimer = 0;
    unsigned long displayRefreshRate = 60;

    void u8g2centerText(const char *s, int16_t x, int16_t y, uint16_t w, uint16_t h);
    void u8g2centerNumber(int n, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
};

extern OmxDisp omxDisp;