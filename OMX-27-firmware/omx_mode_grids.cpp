#include "omx_mode_grids.h"
#include "config.h"
#include "colors.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "sequencer.h"

using namespace grids;

enum GridModePage {
    GRIDS_XY,
    GRIDS_NOTES
};

OmxModeGrids::OmxModeGrids()
{
    for (int i = 0; i < 4; i++)
    {
        gridsXY[i][0] = grids_.getX(i);
        gridsXY[i][1] = grids_.getY(i);
    }
}

void OmxModeGrids::InitSetup()
{
    initSetup = true;
}

void OmxModeGrids::onModeActivated()
{
    if(!initSetup){
        InitSetup();
    }
}

void OmxModeGrids::onClockTick() {
    grids_.gridsTick();
}

void OmxModeGrids::onPotChanged(int potIndex, int potValue)
{
    if (potIndex < 4)
    {
        grids_.setDensity(potIndex, potSettings.analogValues[potIndex] * 2);
    }
    else if (potIndex == 4)
    {
        int newres = (float(potSettings.analogValues[potIndex]) / 128.f) * 3;
        grids_.setResolution(newres);
    }
}

void OmxModeGrids::loopUpdate()
{
}

void OmxModeGrids::onEncoderChanged(Encoder::Update enc)
{
    auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)

    int paramStep = param % 5;

    if (paramStep != 0) // Page select mode if 0
    {
        switch (page)
        {
        case GRIDS_XY:
            if (paramStep == 1) // Accent
            {
                int newAccent = constrain(grids_.accent + amt, 0, 255);
                grids_.accent = newAccent;
            }
            else if (paramStep == 2) // GridX
            {
                int numGrids = sizeof(gridsSelected);
                for (int g = 0; g < numGrids; g++)
                {
                    if (gridsSelected[g])
                    {
                        int newX = constrain(grids_.getX(g) + amt, 0, 255);
                        gridsXY[g][0] = newX;
                        grids_.setX(g, newX);
                    }
                }
            }
            else if (paramStep == 3) // GridY
            {
                int numGrids = sizeof(gridsSelected);
                for (int g = 0; g < numGrids; g++)
                {
                    if (gridsSelected[g])
                    {
                        int newY = constrain(grids_.getY(g) + amt, 0, 255);
                        gridsXY[g][1] = newY;
                        grids_.setY(g, newY);
                    }
                }
            }
            else if (paramStep == 4) // Chaos
            {
                int newChaos = constrain(grids_.chaos + amt, 0, 255);
                grids_.chaos = newChaos;
            }
            break;
        case GRIDS_NOTES:
            if (paramStep == 1)
            {
                grids_.grids_notes[0] = constrain(grids_.grids_notes[0] + amt, 0, 127);
            }
            else if (paramStep == 2)
            {
                grids_.grids_notes[1] = constrain(grids_.grids_notes[1] + amt, 0, 127);
            }
            else if (paramStep == 3)
            {
                grids_.grids_notes[2] = constrain(grids_.grids_notes[2] + amt, 0, 127);
            }
            else if (paramStep == 4)
            {
                grids_.grids_notes[3] = constrain(grids_.grids_notes[3] + amt, 0, 127);
            }
            break;
        default:
            break;
        }
    }
    omxDisp.setDirty();
}

void OmxModeGrids::onEncoderButtonDown()
{
    param = (param + 1 ) % kNumParams;
    page = param / NUM_DISP_PARAMS;
}

void OmxModeGrids::onEncoderButtonDownLong()
{
    
}

bool OmxModeGrids::shouldBlockEncEdit()
{
    return false;
}

void OmxModeGrids::onKeyUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();

    auto keyState = midiSettings.keyState;

    if (!e.held())
    {
        if (e.down() && thisKey == 0) // Aux key down
        {
            // Sequencer shouldn't be a dependancy here but current is used to advance clocks. 
            if (sequencer.playing && gridsAUX)
            {
                gridsAUX = false;
                grids_.stop();
                sequencer.playing = false;
            }
            else
            {
                gridsAUX = true;
                grids_.start();
                sequencer.playing = true;
            }
        }
        else if (e.down() && e.clicks() == 0 && (thisKey > 2 && thisKey < 11))
        {
            int patt = thisKey - 3;
            // SAVE
            if (!keyState[1] && keyState[2])
            { 
                // F2 + PATTERN TO SAVE
                for (int k = 0; k < 4; k++)
                {
                    grids_.gridSaves[patt][k].density = grids_.getDensity(k);
                    grids_.gridSaves[patt][k].x = grids_.getX(k);
                    grids_.gridSaves[patt][k].y = grids_.getY(k);
                    // Serial.print("saved:");
                    // Serial.print(grids_wrapper.gridSaves[patt][k].density);
                    // Serial.print(":");
                    // Serial.print(grids_wrapper.gridSaves[patt][k].x);
                    // Serial.print(":");
                    // Serial.println(grids_wrapper.gridSaves[patt][k].y);
                }

                omxDisp.displayMessage((String)"Saved " + (patt + 1));
            }
            else
            {
                // SELECT
                grids_.playingPattern = patt;
                for (int k = 0; k < 4; k++)
                {
                    grids_.setDensity(k, grids_.gridSaves[patt][k].density);
                    grids_.setX(k, grids_.gridSaves[patt][k].x);
                    grids_.setY(k, grids_.gridSaves[patt][k].y);
                    // Serial.print("state:");
                    // Serial.print(grids_wrapper.gridSaves[patt][k].density);
                    // Serial.print(":");
                    // Serial.print(grids_wrapper.gridSaves[patt][k].x);
                    // Serial.print(":");
                    // Serial.println(grids_wrapper.gridSaves[patt][k].y);
                }

                omxDisp.displayMessage((String)"Load " + (patt + 1));

            }
        }
    }

    if (e.down() && (thisKey > 10 && thisKey < 15))
    {
        gridsSelected[thisKey - 11] = true;
        omxDisp.setDirty();
        page = GRIDS_XY;
        param = 2;
    }
    else if (!e.down() && (thisKey > 10 && thisKey < 15))
    {
        gridsSelected[thisKey - 11] = false;
        omxDisp.setDirty();
    }

    if (e.down() && (thisKey > 14 && thisKey < 19))
    {
        gridsSelected[thisKey - 15] = true;
        omxDisp.setDirty();
        page = GRIDS_XY;
        param = 3;
    }
    else if (!e.down() && (thisKey > 14 && thisKey < 19))
    {
        gridsSelected[thisKey - 15] = false;
        omxDisp.setDirty();
    }
}

void OmxModeGrids::onKeyHeldUpdate(OMXKeypadEvent e)
{
}

void OmxModeGrids::updateLEDs()
{
    omxLeds.updateBlinkStates();

    bool blinkState = omxLeds.getBlinkState();

    int patternNum = grids_.playingPattern;
    if (gridsAUX)
    {
        // Blink left/right keys for octave select indicators.
        auto color1 = blinkState ? LIME : LEDOFF;
        strip.setPixelColor(0, color1);
    }
    else
    {
        strip.setPixelColor(0, LEDOFF);
    }
    uint32_t colors[8] = {};
    colors[0] = blinkState ? MAGENTA : LEDOFF;
    colors[1] = blinkState ? ORANGE : LEDOFF;
    colors[2] = blinkState ? RED : LEDOFF;
    colors[3] = blinkState ? RBLUE : LEDOFF;
    colors[4] = blinkState ? MAGENTA : LEDOFF;
    colors[5] = blinkState ? ORANGE : LEDOFF;
    colors[6] = blinkState ? RED : LEDOFF;
    colors[7] = blinkState ? RBLUE : LEDOFF;

    // FIX THIS

    auto keyState = midiSettings.keyState;

    for (int k = 0; k < 4; k++)
    {
        // Change color of 4 GridX keys when pushed
        if (keyState[k + 11])
        {
            strip.setPixelColor(k + 11, colors[k]);
        }
        else
        {
            strip.setPixelColor(k + 11, PINK);
        }
    }

    for (int k = 4; k < 8; k++)
    {
        // Change color of 4 GridY keys when pushed
        if (keyState[k + 11])
        {
            strip.setPixelColor(k + 11, colors[k]);
        }
        else
        {
            strip.setPixelColor(k + 11, LTCYAN);
        }
    }

    // LEDS for top row
    for (int j = 1; j < LED_COUNT - 16; j++)
    {
        if (j == 1)
        {
            // F1
            if (keyState[j] && blinkState)
            {
                strip.setPixelColor(j, LEDOFF);
            }
            else
            {
                strip.setPixelColor(j, FUNKONE);
            }
        }
        else if (j == 2)
        {
            // F2
            if (keyState[j] && blinkState)
            {
                strip.setPixelColor(j, LEDOFF);
            }
            else
            {
                strip.setPixelColor(j, FUNKTWO);
            }
        }
        else if (j == patternNum + 3)
        { 
            // PATTERN SELECT
            strip.setPixelColor(j, seqColors[patternNum]);

            // Not needed here
            // if (patternParams && blinkState)
            // {
            //     strip.setPixelColor(j, LEDOFF);
            // }
        }
        else
        {
            strip.setPixelColor(j, LEDOFF);
        }
    }

    omxLeds.setDirty();
}

void OmxModeGrids::setupPageLegends()
{

    // if (midiSettings.keyState[11] || midiSettings.keyState[15])
    //     {
    //         thisGrid = 0;
    //     }
    //     else if (keyState[12] || keyState[16])
    //     {
    //         thisGrid = 1;
    //     }
    //     else if (keyState[13] || keyState[17])
    //     {
    //         thisGrid = 2;
    //     }
    //     else if (keyState[14] || keyState[18])
    //     {
    //         thisGrid = 3;
    //     }

    omxDisp.clearLegends();

    switch (page)
    {
    case GRIDS_XY:
    {
        int numGrids = sizeof(gridsSelected);
        int thisGrid = 0;
        int selGridsCount = 0;

        for (int i = 0; i < numGrids; i++)
        {
            if (gridsSelected[i])
            {
                selGridsCount++;
                thisGrid = i;
            }
        }

        if (selGridsCount == 0)
        {
            omxDisp.legends[1] = "X";
            omxDisp.legends[2] = "Y";
            thisGrid = -1;
        }
        else if (selGridsCount == 1)
        {
            omxDisp.legends[1] = String("X " + (thisGrid + 1)).c_str();
            omxDisp.legends[2] = String("Y " + (thisGrid + 1)).c_str();
        }
        else
        {
            omxDisp.legends[1] = "X *";
            omxDisp.legends[2] = "Y *";
        }

        // char bufx[4];
        // char bufy[4];
        // snprintf(bufx, sizeof(bufx), "X %d", gridXKeyChannel + 1);
        // snprintf(bufy, sizeof(bufy), "Y %d", gridYKeyChannel + 1);
        omxDisp.legends[0] = "ACNT"; // "BPM";
        // omxDisp.legends[1] = bufx;
        // omxDisp.legends[2] = bufy;
        omxDisp.legends[3] = "XAOS";
        omxDisp.legendVals[0] = grids_.accent; // (int)clockbpm;
        if (thisGrid != -1)
        {
            omxDisp.legendVals[1] = gridsXY[thisGrid][0];
            omxDisp.legendVals[2] = gridsXY[thisGrid][1];
        }
        omxDisp.legendVals[3] = grids_.chaos;
        omxDisp.dispPage = 1;
    }
    break;
    case GRIDS_NOTES:
        omxDisp.legends[0] = "1";
        omxDisp.legends[1] = "2";
        omxDisp.legends[2] = "3";
        omxDisp.legends[3] = "4";
        omxDisp.legendVals[0] = grids_.grids_notes[0];
        omxDisp.legendVals[1] = grids_.grids_notes[1];
        omxDisp.legendVals[2] = grids_.grids_notes[2];
        omxDisp.legendVals[3] = grids_.grids_notes[3];
        omxDisp.dispPage = 2;
        break;
    default:
        break;
    }
}

void OmxModeGrids::onDisplayUpdate()
{
    updateLEDs();

    if (omxDisp.isDirty())
    { // DISPLAY
        if (!encoderConfig.enc_edit)
        {
            int pselected = param % NUM_DISP_PARAMS;
            setupPageLegends();
            omxDisp.dispGenericMode(pselected);
        }
    }
}