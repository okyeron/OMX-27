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

void OmxDisp::displayMessage(String msg)
{
    displayMessage(msg.c_str());
}

void OmxDisp::displayMessage(const char *msg)
{
    specialMsgType_ = 0;
    currentMsg = msg;

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
    specialMsgType_ = 0;
    va_list args;
    va_start(args, fmt);
    char buf[24];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    displayMessage(buf);
}

// Something is keeping weird cache of display names or serial logs in memory
void OmxDisp::displayMessageTimed(String msg, uint8_t secs)
{
    currentMsg = msg;
    specialMsgType_ = 0;

    renderMessage();

    messageTextTimer = secs * 100000;
    dirtyDisplay = true;
}

void OmxDisp::displaySpecialMessage(uint8_t msgType, String msg, uint8_t secs)
{
    currentMsg = msg;
    specialMsgType_ = msgType;

    renderMessage();

    messageTextTimer = secs * 100000;
    dirtyDisplay = true;
}



void OmxDisp::chordBalanceMsg(int8_t balArray[], float velArray[], uint8_t secs)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        chordBalArray_[i] = balArray[i];
        chordVelArray_[i] = velArray[i];
    }

    currentMsg = "Balance";
    specialMsgType_ = 1;

    renderMessage();

    messageTextTimer = secs * 100000;
    dirtyDisplay = true;
}

void OmxDisp::renderMessage()
{
    if (specialMsgType_ == 0)
    {
        display.fillRect(0, 0, 128, 32, BLACK);
        u8g2_display.setFontMode(1);
        u8g2_display.setFont(FONT_TENFAT);
        u8g2_display.setForegroundColor(WHITE);
        u8g2_display.setBackgroundColor(BLACK);
        u8g2centerText(currentMsg.c_str(), 0, 10, 128, 32);
        // dirtyDisplay = true;
    }
    else if (specialMsgType_ == 1)
    {
        dispChordBalance();
    }
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

void OmxDisp::u8g2leftText(const char *s, int16_t x, int16_t y, uint16_t w, uint16_t h)
{
    uint16_t bh = u8g2_display.getFontAscent();
    u8g2_display.setCursor(
        x,
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
    legendVals[0] = -127;
    legendVals[1] = -127;
    legendVals[2] = -127;
    legendVals[3] = -127;
    dispPage = 0;
    legendText[0] = "";
    legendText[1] = "";
    legendText[2] = "";
    legendText[3] = "";
    useLegendString[0] = false;
    useLegendString[1] = false;
    useLegendString[2] = false;
    useLegendString[3] = false;
}

void OmxDisp::dispGenericMode(int selected)
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

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

void OmxDisp::dispGenericMode2(uint8_t numPages, int8_t selectedPage, int8_t selectedParam, bool encSelActive)
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

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

    if (selectedParam >= 0 && selectedParam < 4)
    {
        if (encSelActive)
        {
            const int8_t bWidth = 1;
            display.fillRect(selectedParam * 32 + 2, 9, 29, 21, WHITE);
            display.fillRect(selectedParam * 32 + 2 + bWidth, 9 + bWidth, 29 - (bWidth * 2), 21 - (bWidth * 2), BLACK);
        }
        else
        {
            display.fillRect(selectedParam * 32 + 2, 9, 29, 21, WHITE);
        }

            // display.fillRect(selectedParam * 32 + 2, 9, 29, 21, WHITE);

    }

    // ValueBoxes
    bool highlight = false;
    for (int j = 0; j < 4; j++)
    { 
        highlight = (j == selectedParam && !encSelActive);

        if(useLegendString[j])
        {
            dispSymbBox(legendString[j].c_str(), j, highlight);
        }
        else if (legendVals[j] == -127)
        {
            dispSymbBox(legendText[j], j, highlight);
        }
        else
        {
            dispValBox(legendVals[j], j, highlight);
        }
    }

    dispPageIndicators2(numPages, selectedPage);
}

void OmxDisp::dispGenericModeLabel(const char *label, uint8_t numPages, int8_t selectedPage)
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

    display.fillRect(0, 0, 128, 32, BLACK);
    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_TENFAT);
    u8g2_display.setForegroundColor(WHITE);
    u8g2_display.setBackgroundColor(BLACK);
    u8g2centerText(label, 0, 10, 128, 32);

    if (numPages > 1)
    {
        dispPageIndicators2(numPages, selectedPage);
    }
}

void OmxDisp::dispGenericModeLabelDoubleLine(const char* label1, const char* label2, uint8_t numPages, int8_t selectedPage)
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

    display.fillRect(0, 0, 128, 32, BLACK);
    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_VALUES);
    u8g2_display.setForegroundColor(WHITE);
    u8g2_display.setBackgroundColor(BLACK);
    u8g2centerText(label1, 0, 12, 128, 8);
    u8g2centerText(label2, 0, 26, 128, 8);

    if (numPages > 1)
    {
        dispPageIndicators2(numPages, selectedPage);
    }

}

void OmxDisp::dispGenericModeLabelSmallText(const char *label, uint8_t numPages, int8_t selectedPage)
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

    display.fillRect(0, 0, 128, 32, BLACK);
    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_LABELS);
    u8g2_display.setForegroundColor(WHITE);
    u8g2_display.setBackgroundColor(BLACK);
    u8g2centerText(label, 0, 10, 128, 8);

    if (numPages > 1)
    {
        dispPageIndicators2(numPages, selectedPage);
    }
}

void OmxDisp::dispChar16(const char* charArray[], uint8_t charCount, uint8_t selected, uint8_t numPages, int8_t selectedPage, bool encSelActive, bool showLabels, const char* labels[], uint8_t labelCount)
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

    display.fillRect(0, 0, 128, 32, BLACK);

    if(showLabels)
    {
        int8_t selIndex = constrain(selected - 16, -1, 127);
        dispLabelParams(selIndex, encSelActive, labels, labelCount, false);
    }

    uint8_t charWidth = 128 / 16; // 8

    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_CHAR16);

    uint8_t yPos = hline * 2 + 3; // 19
    
    for(uint8_t i = 0; i < 16; i++)
    {
        bool showChar = i < charCount;
        if(i == selected)
        {
            display.drawFastHLine(i * charWidth + 1, 26, charWidth - 2, WHITE);

            if(encSelActive == false)
            {
                display.fillRect(i * charWidth, 14, charWidth, 10, WHITE);
                invertColor(true);
                showChar = true;
            }
            else
            {
                invertColor(false);
            }
            // display.drawLine(i * charWidth - charWidth + 1, yPos + 2, i * charWidth - 1, yPos + 2, WHITE);
        }
        else
        {
            invertColor(false);
        }

        if(showChar)
        {
            u8g2centerText(charArray[i], i * charWidth, yPos, charWidth - 1, 16);
        }
    }

    // if (numPages > 1)
    // {
    //     dispPageIndicators2(numPages, selectedPage);
    // }
}


void OmxDisp::dispValues16(int8_t valueArray[], uint8_t valueCount, int8_t minValue, int8_t maxValue, bool centered, uint8_t selected, uint8_t numPages, int8_t selectedPage, bool encSelActive, bool showLabels, const char* labels[], uint8_t labelCount)
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

    display.fillRect(0, 0, 128, 32, BLACK);

    if(showLabels)
    {
        int8_t selIndex = constrain(selected - 16, -1, 127);
        dispLabelParams(selIndex, encSelActive, labels, labelCount, false);
    }

    uint8_t boxWidth = 128 / 16; // 8
    uint8_t boxStartY = 10;
    uint8_t heightMax = 32;
    uint8_t boxHeight = heightMax - boxStartY;
    uint8_t halfBoxHeight = boxHeight / 2;

    int8_t middleValue = ((maxValue - minValue) / 2) + minValue;

    for(uint8_t i = 0; i < 16; i++)
    {
        if(i < valueCount && valueArray[i] == -127) continue;

        uint16_t fgColor = WHITE;

        uint8_t xPos = i * boxWidth + 2;
        uint8_t width = boxWidth - 4;

        if(i == selected && encSelActive)
        {
            display.fillRect(i * boxWidth, boxStartY, boxWidth, boxHeight, WHITE);
            display.fillRect(i * boxWidth + 1, boxStartY + 1, boxWidth - 2, boxHeight - 2, BLACK);
        }

        if(i >= valueCount)
        {
            // display.fillRect(i * boxWidth + 3, boxStartY + (halfBoxHeight + 1), 1, 1, fgColor);

            continue;
        }

        if (centered)
        {
            if (valueArray[i] >= middleValue)
            {
                float valuePerc = constrain(map((float)valueArray[i], (float)middleValue, (float)maxValue, 0.0f, 1.0f), 0.0f, 1.0f);
                uint8_t valueHeight = max(halfBoxHeight * valuePerc, 0);
                display.fillRect(xPos, boxStartY + (halfBoxHeight + 1) - valueHeight, width, valueHeight + 1, fgColor);

                // if(i == selected)
                // {
                //     Serial.println("valuePerc: " + String(valuePerc) + " valueHeight: " + String(valueHeight) + " startY: " + String(boxStartY + halfBoxHeight - valueHeight));
                // }
            }
            else
            {
                float valuePerc = 1.0f - constrain(map((float)valueArray[i], (float)minValue, (float)middleValue, 0.0f, 1.0f), 0.0f, 1.0f);
                uint8_t valueHeight = constrain((boxHeight - halfBoxHeight) * valuePerc, 0, halfBoxHeight - 3);
                display.fillRect(xPos, boxStartY + halfBoxHeight + 1, width, valueHeight + 1, fgColor);
                // display.fillRect(i + 3, boxStartY + halfBoxHeight + 1, boxWidth - 4, valueHeight - 2, bgColor);
            }
        }
        else
        {
            float valuePerc = constrain(map((float)valueArray[i], (float)minValue, (float)maxValue, 0.0f, 1.0f), 0.0f, 1.0f);
            uint8_t valueHeight = constrain(boxHeight * valuePerc, 0, boxHeight - 1);
            display.fillRect(xPos, boxStartY + boxHeight - valueHeight, width, valueHeight + 1, fgColor);
        }
    }

    // if (numPages > 1)
    // {
    //     dispPageIndicators2(numPages, selectedPage);
    // }
}

void OmxDisp::dispSlots(const char* slotNames[], uint8_t slotCount, uint8_t selected, uint8_t animPos, bool encSelActive, bool showLabels, const char* labels[], uint8_t labelCount)
{
    // if (isMessageActive())
    // {
    //     renderMessage();
    //     return;
    // }

    display.fillRect(0, 0, 128, 32, BLACK);

    // if(showLabels)
    // {
    //     int8_t selIndex = constrain(selected - 16, -1, 127);
    //     dispLabelParams(selIndex, encSelActive, labels, labelCount);
    // }

    // uint8_t rowCount = slotCount - 1;// Selected slot will be raised

    uint8_t rowCount = 4;// Selected slot will be raised

    int8_t selYOffset = 0; // 14 to 0
    int8_t horzOffset = 18; // 18 to 1, can reduce after selYOffset <= 1

    if(animPos < 14)
    {
        selYOffset = 14 - animPos;
    }

    if(selYOffset <= 0)
    {
        horzOffset = map(constrain(animPos, 13, 26), 13, 26, 18, 2);
    }

    uint8_t slotWidth = 128 / rowCount; 
    uint8_t slotHeight = 12;
    uint8_t slotPad = 1;

    // uint8_t charWidth = 128 / 16; // 8

    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_LABELS);

    // uint8_t yPos = hline * 2 + 3; // 19

    uint8_t yPos = 15; // 19

    int8_t slotIndex = selected - 2;
    uint8_t slotOffset = 0;

    if(selected == 0)
    {
        slotOffset = 2;
    }
    else if(selected == 1)
    {
        slotOffset = 1;
    }

    for(int8_t i = slotIndex; i < slotCount; i++)
    {
        if(i != selected)
        {
            if (slotIndex >= 0 && slotIndex < slotCount)
            {
                int8_t hOff = slotOffset < 2 ? -horzOffset + 1 : horzOffset - 2;

                display.fillRect(slotOffset * slotWidth + slotPad + 1 + hOff, yPos, slotWidth - (slotPad * 2) - 2, slotHeight, WHITE);
                display.fillRect(slotOffset * slotWidth + slotPad + 2 + hOff, yPos + 1, slotWidth - 4 - (slotPad * 2), slotHeight - 2, BLACK);
                invertColor(false);
                u8g2centerText(slotNames[i], slotOffset * slotWidth + slotPad + 2 + hOff, yPos + (slotHeight / 2) + 2, slotWidth - 4 - (slotPad * 2), 8);
                slotOffset++;
            }
            slotIndex++;

            if(slotOffset >= 4)
            {
                break;
            }
        }
    }

    // Display selected slot
    slotWidth = 36; 
    slotHeight = 13;
    yPos = 0 + selYOffset; // 19
    uint8_t selectedStart = 64 - (slotWidth / 2);

    display.fillRect(selectedStart + slotPad, yPos, slotWidth - (slotPad * 2), slotHeight, WHITE);
    display.fillRect(selectedStart + slotPad + 1, yPos + 1, slotWidth - 2 - (slotPad * 2), slotHeight - 2, BLACK);
    invertColor(false);
    u8g2_display.setFont(FONT_CHAR16);
    u8g2centerText(slotNames[selected], selectedStart + slotPad + 1, yPos + (slotHeight / 2) + 3, slotWidth - 2 - (slotPad * 2), 8);

    if(yPos + slotHeight < 25)
    {
        display.drawLine(63, yPos + slotHeight, 63, 25, WHITE);
    }
}

void OmxDisp::dispCenteredSlots(const char* slotNames[], uint8_t slotCount, uint8_t selected, bool encoderSelect, bool showLabels, bool centerLabels, const char* labels[], uint8_t labelCount)
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

    display.fillRect(0, 0, 128, 32, BLACK);

    uint8_t slotWidth = 128 / slotCount;

    for(uint8_t i = 0; i < slotCount; i++)
    {
        dispParamLabel(i * slotWidth, 10, slotWidth, 18, selected == i, 1, encoderSelect, true, slotNames[i], FONT_VALUES, 1, true);
    }

    // dispParamLabel(32, 10, 32, 18, selected == 1, 1, encoderSelect, true, octaveName, FONT_VALUES, 1, true);
    // dispParamLabel(0, 0, 128, 10, selected == 3, 0, encoderSelect, true, chordType, FONT_LABELS, 1, true);


    if(showLabels)
    {
        int8_t selIndex = constrain(selected - slotCount, -1, 127);
        dispLabelParams(selIndex, encoderSelect, labels, labelCount, centerLabels);
    }
}

void OmxDisp::dispKeyboard(int rootNote, int noteNumbers[], bool showLabels, const char* labels[], uint8_t labelCount)
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

    const uint8_t wkWidth = 7;
    const uint8_t wkInc = 6;

    const uint8_t wkHeight = 22;
    const uint8_t wkStartX = 16;
    const uint8_t wkStartY = 10;

    const uint8_t bkWidth = 7;
    const uint8_t bkInc = 6;

    const uint8_t bkHeight = 16;
    const uint8_t bkStartX = 13;
    const uint8_t bkStartY = 9;

    display.fillRect(0, 0, 128, 32, BLACK);

    

    // Find and split up black and white notes
    bool blackNotes[10];
    bool whiteNotes[16];

    for(uint8_t i = 0; i < 16; i++)
    {
        if(i < 10)
        {
            blackNotes[i] = false;
        }
        whiteNotes[i] = false;
    }

    // int rootNote = -1;
    
    // // Find the lowest note
    // for(uint8_t i = 0; i < 6; i++)
    // {
    //     if(noteNumbers[i] >= 0 && noteNumbers[i] <= 127)
    //     {
    //         if(rootNote < 0 || noteNumbers[i] < rootNote)
    //         {
    //             rootNote = noteNumbers[i];
    //         }
    //     }
    // }

    bool addOctave = rootNote % 24 >= 12;
    
    for(uint8_t i = 0; i < 6; i++)
    {
        int note = noteNumbers[i];

        // If valid note
        if(note >= 0 && note <= 127)
        {
            // uint8_t threeOctNote = (note + (addOctave ? 12 : 0)) % 36;

            // C edge case if note is 2 octaves above root since there's
            // one extra C
            if(note - rootNote == 24)
            {
                whiteNotes[15] = true;
                continue;
            }

            uint8_t twoOctNote = (note + (addOctave ? 12 : 0)) % 24;

            for(uint8_t j = 1; j < 27; j++)
            {
                uint8_t stepNote = (notes[j] + 12) % 24; // Turn note lookup into 0-24 semitones

                // B edge case
                if(j == 11)
                {
                    // If note is b and less than root note
                    if(note % 12 == 11 && note < rootNote)
                    {
                        whiteNotes[j-11] = true;
                        break;
                    }
                }

                if(twoOctNote == stepNote)
                {
                    if(j >= 11)
                    {
                        whiteNotes[j-11] = true;
                    }
                    else
                    {
                        blackNotes[j-1] = true;
                    }
                    break;
                }
            }
        }
    }

    // draw white keys
    for(uint8_t i = 0; i < 16; i++)
    {
        if (whiteNotes[i] == false)
        {
            // display.fillRect(startX + (wkWidth * i), wkStartY, wkWidth, wkHeight, WHITE);
            display.drawRect(wkStartX + (wkInc * i), wkStartY, wkWidth, wkHeight, WHITE);
        }
    }

    for(uint8_t i = 0; i < 16; i++)
    {
        if (whiteNotes[i])
        {
            display.drawRect(wkStartX + (wkInc * i), wkStartY, wkWidth, wkHeight, BLACK);
            display.fillRect(wkStartX + (wkInc * i) + 1, wkStartY, wkWidth - 2, wkHeight, WHITE);
        }
    }

    uint8_t bOffset = 0;

    // draw black keys
    // Two additional keys for sides
    for(uint8_t i = 0; i < 12; i++)
    {
        bool blackOn = false;

        if(i == 1 || i == 3 || i == 6 || i == 8 || i == 11)
        {
            bOffset += 6;
        }

        uint8_t xStart = bkStartX + bOffset + (bkInc * i);

        if( i >0 && i < 11)
        {
            blackOn = blackNotes[i - 1];
        }
        else
        {
            display.fillRect(xStart, bkStartY, bkWidth, bkHeight, BLACK);
            display.drawRect(xStart + 1, bkStartY + 1, bkWidth - 2, bkHeight - 2, WHITE);
            display.fillRect(xStart + 2, bkStartY, bkWidth - 4, bkHeight - 1, BLACK);
            continue;;
        }

        if (blackOn)
        {
            display.fillRect(xStart, bkStartY, bkWidth, bkHeight, BLACK);
            display.fillRect(xStart + 1, bkStartY + 1, bkWidth - 2, bkHeight - 2, WHITE);
        }
        else
        {
            // display.fillRect(startX + (wkWidth * i), wkStartY, wkWidth, wkHeight, WHITE);
            display.fillRect(xStart, bkStartY, bkWidth, bkHeight, BLACK);
            display.drawRect(xStart + 1, bkStartY + 1, bkWidth - 2, bkHeight - 2, WHITE);
        }
    }

    display.fillRect(0, 10, 16, 32, BLACK); // trim left side
    display.fillRect(113, 10, 15, 32, BLACK); // trim right side
    display.drawLine(18, 10, 110, 10, WHITE); // Cap the top

    if(!whiteNotes[0])
    {
        display.drawLine(16, 24, 16, 31, WHITE); // Left wall
    }

    if(!whiteNotes[15])
    {
        display.drawLine(112, 24, 112, 31, WHITE); // Right wall
    }

    if(showLabels)
    {
        // int8_t selIndex = constrain(selected - 16, -1, 127);
        dispLabelParams(-1, true, labels, labelCount, true);
    }
}

void OmxDisp::dispChordBasicPage(uint8_t selected, bool encoderSelect, const char* noteName, const char* octaveName, const char* chordType, int8_t balArray[], float velArray[])
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

    display.fillRect(0, 0, 128, 32, BLACK);

    dispParamLabel(0, 10, 32, 18, selected == 0, 1, encoderSelect, true, noteName, FONT_VALUES, 1, true);
    dispParamLabel(32, 10, 32, 18, selected == 1, 1, encoderSelect, true, octaveName, FONT_VALUES, 1, true);
    dispParamLabel(0, 0, 128, 10, selected == 3, 0, encoderSelect, true, chordType, FONT_LABELS, 1, true);

    const uint8_t width = 10;
    const uint8_t height = 16;
    const uint8_t highHeight = 10;
    const uint8_t space = 3;
    const uint8_t totalWidth = width + space * 2;
    const uint8_t startY = 11;
    const uint8_t endY = startY + height; // 27

    // const uint8_t startX = 64 - (((totalWidth) * 4) / 2); // 64 is width of duders

    const uint8_t startX = 64; // 64 is width of duders
    
    for(uint8_t i = 0; i < 4; i++)
    {
        uint8_t yPos = map(velArray[i], 0.0f, 1.0f, (float)endY, (float)startY);

        int bal = balArray[i];
        if(bal <= -10) continue;

        if(bal == 0)
        {
            display.fillRect(startX + (totalWidth * i) + space, yPos, width, height, WHITE);

            // Eyes
            // xx xx xx xx xx
            // xx oo xx oo xx
            // xx oo xx oo xx
            // xx xx xx xx xx
            display.fillRect(startX + (totalWidth * i) + space + 2, yPos + 2, 2, 4, BLACK);
            display.fillRect(startX + (totalWidth * i) + space + 6, yPos + 2, 2, 4, BLACK);
        }
        else if(bal < 0)
        {
            yPos += 2;
            display.fillRect(startX + (totalWidth * i) + space - 2, yPos - 2, width + 4, height + 4, WHITE);
            display.fillRect(startX + (totalWidth * i) + space, yPos, width, height, BLACK);
            
            display.fillRect(startX + (totalWidth * i) + space + 2, yPos + 2, 2, 2, WHITE);
            display.fillRect(startX + (totalWidth * i) + space + 6, yPos + 2, 2, 2, WHITE);
        }
        else if(bal > 0)
        {
            display.fillRect(startX + (totalWidth * i) + space, yPos, width, highHeight, WHITE);

            display.fillRect(startX + (totalWidth * i) + space + 2, yPos + 2, 2, 4, BLACK);
            display.fillRect(startX + (totalWidth * i) + space + 6, yPos + 2, 2, 4, BLACK);
        }
    }

    display.fillRect(startX, 28, 64, 10, BLACK);

    if (selected == 2 && encoderSelect)
    {
        display.fillRect(startX + 32 - 1, 28, 2, 4, WHITE);
        display.fillRect(startX + 32 - 3, 28 + 2, 6, 2, WHITE);
    }
    else if (selected == 2 && !encoderSelect)
    {
        display.fillRect(startX + 2, 28, 64 - 4, 2, WHITE);
    }
}

void OmxDisp::dispChordBalance()
{
    const uint8_t width = 10;
    const uint8_t height = 16;
    const uint8_t highHeight = 10;
    const uint8_t space = 3;
    const uint8_t totalWidth = width + space * 2;
    const uint8_t startY = 5;
    const uint8_t endY = startY + height;

    const uint8_t startX = 64 - (((totalWidth) * 4) / 2);

    display.fillRect(0, 0, 128, 32, BLACK);
    
    for(uint8_t i = 0; i < 4; i++)
    {
        uint8_t yPos = map(chordVelArray_[i], 0.0f, 1.0f, (float)endY, (float)startY);

        // Serial.println("ypos: " + String(yPos));

        int bal = chordBalArray_[i];

        // Serial.println("bal: " + String(bal));

        if(bal <= -10) continue;

        if(bal == 0)
        {
            display.fillRect(startX + (totalWidth * i) + space, yPos, width, height, WHITE);

            // Eyes
            // xx xx xx xx xx
            // xx oo xx oo xx
            // xx oo xx oo xx
            // xx xx xx xx xx
            display.fillRect(startX + (totalWidth * i) + space + 2, yPos + 2, 2, 4, BLACK);
            display.fillRect(startX + (totalWidth * i) + space + 6, yPos + 2, 2, 4, BLACK);
        }
        else if(bal < 0)
        {
            yPos += 2;
            display.fillRect(startX + (totalWidth * i) + space - 2, yPos - 2, width + 4, height + 4, WHITE);
            display.fillRect(startX + (totalWidth * i) + space, yPos, width, height, BLACK);
            
            display.fillRect(startX + (totalWidth * i) + space + 2, yPos + 2, 2, 2, WHITE);
            display.fillRect(startX + (totalWidth * i) + space + 6, yPos + 2, 2, 2, WHITE);
        }
        else if(bal > 0)
        {
            display.fillRect(startX + (totalWidth * i) + space, yPos, width, highHeight, WHITE);

            display.fillRect(startX + (totalWidth * i) + space + 2, yPos + 2, 2, 4, BLACK);
            display.fillRect(startX + (totalWidth * i) + space + 6, yPos + 2, 2, 4, BLACK);
        }
    }

    //  Serial.println("");

    display.fillRect(0, endY, 128, 32, BLACK);
}


void OmxDisp::dispLabelParams(int8_t selected, bool encSelActive, const char *labels[], uint8_t labelCount, bool centered)
{
    u8g2_display.setFontMode(1);
    u8g2_display.setFont(FONT_LABELS);
    u8g2_display.setCursor(0, 0);

    uint8_t labelWidth = 128 / labelCount; // 8

    for (uint8_t i = 0; i < labelCount; i++)
    {
        bool invert = false;
        // Label Selected
        if (i == selected)
        {
            if (encSelActive == false)
            {
                display.fillRect(i * labelWidth, 0, labelWidth, 10, WHITE);
                invert = true;
            }
            else
            {
                display.fillRect(i * labelWidth, 0, labelWidth, 10, WHITE);
                display.fillRect(i * labelWidth + 1, 0 + 1, labelWidth - 2, 10 - 2, BLACK);
            }
        }

        invertColor(invert);
        if (centered)
        {
            u8g2centerText(labels[i], i * labelWidth + 2, hline - 2, labelWidth - 4, 10);
        }
        else
        {
            u8g2leftText(labels[i], i * labelWidth + 2, hline - 2, labelWidth - 4, 10);
        }
    }
}

void OmxDisp::dispParamLabel(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool selected, uint8_t selectionType, bool encSelActive, bool showLabel, const char* label, const uint8_t* font, int8_t labelYOffset, bool centered)
{
    bool invert = false;
    // Label Selected
    if (selected && encSelActive)
    {
        if (selectionType == 0)
        {
            display.drawRect(x, y, width, height, WHITE);
            // display.fillRect(x + 1, 0 + 1, width - 2, 10 - 2, BLACK);
        }
        else if (selectionType == 1)
        {
            display.fillRect(x + width / 2 - 1, y + height, 2, 4, WHITE);
            display.fillRect(x + width / 2 - 3, y + height + 2, 6, 2, WHITE);
        }
    }
    else if (selected && !encSelActive)
    {
        if (selectionType == 0)
        {
            display.fillRect(x, y, width, height, WHITE);
            invert = true;
        }
        else if (selectionType == 1)
        {
            display.fillRect(x + 2, y + height, width - 4, 2, WHITE);
        }
    }

    if (showLabel)
    {
        u8g2_display.setFontMode(1);
        u8g2_display.setFont(font);
        u8g2_display.setCursor(0, 0);

        invertColor(invert);
        if (centered)
        {
            u8g2centerText(label, x, y + height / 2 + labelYOffset, width, height);
        }
        else
        {
            u8g2leftText(label, x + 2, y + height / 2 + labelYOffset, width - 4, height);
        }
    }
}

void OmxDisp::dispPageIndicators2(uint8_t numPages, int8_t selected)
{
    int16_t indicatorWidth = 6;
    int16_t indicatorYPos = 32;
    int16_t segment = (6 + 6);

    int16_t start = (128 - (segment * numPages)) / 2.0;

    // Serial.println((String)"start: " + start + " indicatorYPos: " + indicatorYPos + " segment: " + segment);

    for(uint8_t i = 0; i < numPages; i++)
    {
        int16_t h = ((i == selected) ? 2 : 1);

        display.fillRect(start + (i * segment), indicatorYPos - h, indicatorWidth, h, WHITE);
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

void OmxDisp::drawEuclidPattern(bool singleView, bool *pattern, uint8_t steps, uint8_t yPos, bool selected, bool isPlaying, uint8_t seqPos)
{
    if (isMessageActive())
    {
        renderMessage();
        return;
    }

    const bool selectAsLine = false;

    int16_t startSpacing = singleView ? 0 : 6;
    int16_t patWidth = gridw - startSpacing;
    
    if (selected)
    {
        if (selectAsLine)
        {
            display.drawLine(0, yPos, gridw, yPos, WHITE);
            patWidth = gridw;
            startSpacing = 0;
        }
        else
        {
            display.fillRect(0, yPos - 3, 3, 3, WHITE);
            display.drawPixel(1, yPos - 2, BLACK);
        }
    }

    if (steps == 0)
    {
        return;
    }

    int16_t steponHeight = singleView ? 8 : 5;
    // int16_t steponWidth = 2;
    int16_t stepoffHeight = 2;
    // int16_t stepoffWidth = 2;
    // int16_t halfh = gridh / 2;
    // int16_t halfw = gridw / 2;

    float stepint = (float)patWidth / (float)steps;

    for (int i = 0; i < steps; i++)
    {
        int16_t xPos = startSpacing + (stepint * i);
        // int16_t yPos = halfh;

        uint8_t w = 2;
        if(isPlaying && i == seqPos){
            w = 4;
            xPos -= 1;
        }

        if (pattern[i])
        {
            display.fillRect(xPos, yPos - steponHeight, w, steponHeight, WHITE);
        }
        else
        {
            display.fillRect(xPos, yPos - stepoffHeight, w, stepoffHeight, WHITE);
        }

        // if(i == seqPos)
        // {
        //     display.fillRect(xPos, yPos, stepoffWidth, stepoffWidth, WHITE);

        //     // display.drawPixel(xPos, yPos, WHITE);
        // }
    }

    // if (isPlaying)
    // {
    //     uint8_t seqPos
    //     int16_t xPos = (gridw - startSpacing) * playheadPerc + startSpacing;

    //     display.drawPixel(xPos, yPos, WHITE);
    // }

    // omxDisp.setDirty();
}

OmxDisp omxDisp;
