#include <U8g2_for_Adafruit_GFX.h>

#include "omx_disp.h"
#include "consts.h"
#include "ClearUI.h"

U8G2_FOR_ADAFRUIT_GFX u8g2_display;

// Constructor
OmxDisp::OmxDisp()
{
}

void OmxDisp::setup()
{
    initializeDisplay();
    u8g2_display.begin(display);
}

void OmxDisp::clearDisplay()
{
    // Clear display
    display.display();
    setDirty();
}

void OmxDisp::drawStartupScreen()
{
    display.clearDisplay();
    testdrawrect();
    delay(200);
    display.clearDisplay();
    u8g2_display.setForegroundColor(WHITE);
    u8g2_display.setBackgroundColor(BLACK);
    drawLoading();
}

void OmxDisp::displayMessage(const char *msg)
{
    display.fillRect(0, 0, 128, 32, BLACK);
    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_TENFAT);
    u8g2_display.setForegroundColor(WHITE);
    u8g2_display.setBackgroundColor(BLACK);
    u8g2centerText(msg, 0, 10, 128, 32);

    messageTextTimer = MESSAGE_TIMEOUT_US;
    dirtyDisplay = true;
}

void OmxDisp::displayMessagef(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[24];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    displayMessage(buf);
}

bool OmxDisp::isMessageActive(){
    return messageTextTimer > 0;
}

void OmxDisp::u8g2centerText(const char *s, int16_t x, int16_t y, uint16_t w, uint16_t h)
{
    //  int16_t bx, by;
    uint16_t bw, bh;
    bw = u8g2_display.getUTF8Width(s);
    bh = u8g2_display.getFontAscent();
    u8g2_display.setCursor(
        x + (w - bw) / 2,
        y + (h - bh) / 2);
    u8g2_display.print(s);
}

void OmxDisp::u8g2centerNumber(int n, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    char buf[8];
    itoa(n, buf, 10);
    u8g2centerText(buf, x, y, w, h);
}

void OmxDisp::testdrawrect()
{
    display.clearDisplay();

    for (int16_t i = 0; i < display.height() / 2; i += 2)
    {
        display.drawRect(i, i, display.width() - 2 * i, display.height() - 2 * i, SSD1306_WHITE);
        display.display(); // Update screen with each newly-drawn rectangle
        delay(1);
    }

    delay(500);
}

void OmxDisp::drawLoading()
{
    const char *loader[] = {"\u25f0", "\u25f1", "\u25f2", "\u25f3"};
    display.clearDisplay();
    u8g2_display.setFontMode(0);
    for (int16_t i = 0; i < 16; i += 1)
    {
        display.clearDisplay();
        u8g2_display.setCursor(18, 18);
        u8g2_display.setFont(FONT_TENFAT);
        u8g2_display.print("OMX-27");
        u8g2_display.setFont(FONT_SYMB_BIG);
        u8g2centerText(loader[i % 4], 80, 10, 32, 32); // "\u00BB\u00AB" // // dice: "\u2685"
        display.display();
        delay(100);
    }

    delay(100);
}

void OmxDisp::dispGridBoxes()
{
    display.fillRect(0, 0, gridw, 10, WHITE);
    display.drawFastVLine(gridw / 4, 0, gridh, INVERSE);
    display.drawFastVLine(gridw / 2, 0, gridh, INVERSE);
    display.drawFastVLine(gridw * 0.75, 0, gridh, INVERSE);
}
void OmxDisp::invertColor(bool flip)
{
    if (flip)
    {
        u8g2_display.setForegroundColor(BLACK);
        u8g2_display.setBackgroundColor(WHITE);
    }
    else
    {
        u8g2_display.setForegroundColor(WHITE);
        u8g2_display.setBackgroundColor(BLACK);
    }
}
void OmxDisp::dispValBox(int v, int16_t n, bool inv)
{ // n is box 0-3
    invertColor(inv);
    u8g2centerNumber(v, n * 32, hline * 2 + 3, 32, 22);
}

void OmxDisp::dispSymbBox(const char *v, int16_t n, bool inv)
{ // n is box 0-3
    invertColor(inv);
    u8g2centerText(v, n * 32, hline * 2 + 3, 32, 22);
}



void OmxDisp::clearLegends()
{
    legends[0] = "";
    legends[1] = "";
    legends[2] = "";
    legends[3] = "";
    legendVals[0] = 0;
    legendVals[1] = 0;
    legendVals[2] = 0;
    legendVals[3] = 0;
    dispPage = 0;
    legendText[0] = "";
    legendText[1] = "";
    legendText[2] = "";
    legendText[3] = "";
}

void OmxDisp::dispGenericMode(int selected)
{
    if(isMessageActive()) return;
    
    // const char* legends[4] = {"","","",""};
    // int legendVals[4] = {0,0,0,0};
    // int dispPage = 0;
    // const char* legendText[4] = {"","","",""};
    //	int displaychan = sysSettings.midiChannel;

    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_LABELS);
    u8g2_display.setCursor(0, 0);
    dispGridBoxes();

    // labels
    u8g2_display.setForegroundColor(BLACK);
    u8g2_display.setBackgroundColor(WHITE);

    for (int j = 0; j < 4; j++)
    {
        u8g2centerText(legends[j], (j * 32) + 1, hline - 2, 32, 10);
    }

    // value text formatting
    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_VALUES);
    u8g2_display.setForegroundColor(WHITE);
    u8g2_display.setBackgroundColor(BLACK);

    switch (selected)
    {
    case 1:
        display.fillRect(0 * 32 + 2, 9, 29, 21, WHITE);
        break;
    case 2: //
        display.fillRect(1 * 32 + 2, 9, 29, 21, WHITE);
        break;
    case 3: //
        display.fillRect(2 * 32 + 2, 9, 29, 21, WHITE);
        break;
    case 4: //
        display.fillRect(3 * 32 + 2, 9, 29, 21, WHITE);
        break;
    case 0:
    default:
        break;
    }
    // ValueBoxes
    int highlight = false;
    for (int j = 1; j < NUM_DISP_PARAMS; j++)
    { // start at 1 to only highlight values 1-4

        if (j == selected)
        {
            highlight = true;
        }
        else
        {
            highlight = false;
        }
        if (legendVals[j - 1] == -127)
        {
            dispSymbBox(legendText[j - 1], j - 1, highlight);
        }
        else
        {
            dispValBox(legendVals[j - 1], j - 1, highlight);
        }
    }
    if (dispPage != 0)
    {
        for (int k = 0; k < 4; k++)
        {
            if (dispPage == k + 1)
            {
                dispPageIndicators(k, true);
            }
            else
            {
                dispPageIndicators(k, false);
            }
        }
    }
}

void OmxDisp::dispPageIndicators(int page, bool selected)
{
    if (selected)
    {
        display.fillRect(43 + (page * 12), 30, 6, 2, WHITE);
    }
    else
    {
        display.fillRect(43 + (page * 12), 31, 6, 1, WHITE);
    }
}

void OmxDisp::dispMode()
{
    // labels formatting
    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_BIG);
    u8g2_display.setCursor(0, 0);

    u8g2_display.setForegroundColor(WHITE);
    u8g2_display.setBackgroundColor(BLACK);

    const char *displaymode = "";
    if (sysSettings.newmode != sysSettings.omxMode && encoderConfig.enc_edit)
    {
        displaymode = modes[sysSettings.newmode]; // display.print(modes[sysSettings.newmode]);
    }
    else if (encoderConfig.enc_edit)
    {
        displaymode = modes[sysSettings.omxMode]; // display.print(modes[mode]);
    }
    u8g2centerText(displaymode, 86, 20, 44, 32);
}

void OmxDisp::setDirty()
{
    dirtyDisplay = true;
}

void OmxDisp::UpdateMessageTextTimer()
{
    if (messageTextTimer > 0)
    {
        messageTextTimer -= sysSettings.timeElasped;
        if (messageTextTimer <= 0)
        {
            setDirty();
            messageTextTimer = 0;
        }
    }
}

void OmxDisp::showDisplay()
{
    if (dirtyDisplay)
    {
        if (dirtyDisplayTimer > displayRefreshRate)
        {
            display.display();
            dirtyDisplay = false;
            dirtyDisplayTimer = 0;
        }
    }
}

void OmxDisp::bumpDisplayTimer()
{
    dirtyDisplayTimer = displayRefreshRate + 1;
}

OmxDisp omxDisp;
