#include "midimacro_m8.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "MM.h"
#include "consts.h"
#include "colors.h"

namespace midimacro
{
    enum M8Page {
        CHPAGE_1
    };

    MidiMacroM8::MidiMacroM8()
    {
        params_.addPage(4);
        encoderSelect_ = true;
    }

    String MidiMacroM8::getName()
    {
        return String("Chance");
    }

    void MidiMacroM8::onEnabled()
    {
    }

    void MidiMacroM8::onDisabled()
    {
    }
   
    void MidiMacroM8::loopUpdate()
    {
    }

    void MidiMacroM8::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
    {
        omxUtil.sendPots(potIndex, midiMacroConfig.midiMacroChan);
    }

    void MidiMacroM8::onKeyUpdate(OMXKeypadEvent e)
    {
        int thisKey = e.key();
        int keyPos = thisKey - 11;

        if (!e.held())
        {
            if (e.down() && (thisKey > 10 && thisKey < 27))
            {
                // Mutes / Solos
                m8mutesolo[keyPos] = !m8mutesolo[keyPos];
                int mutePos = keyPos + 12;
                if (m8mutesolo[keyPos])
                {
                    MM::sendNoteOn(mutePos, 1, midiMacroConfig.midiMacroChan);
                }
                else
                {
                    MM::sendNoteOff(mutePos, 0, midiMacroConfig.midiMacroChan);
                }
                return; // break;
            }
            else if (e.down() && (thisKey == 1))
            {
                // release all mutes
                for (int z = 0; z < 8; z++)
                {
                    int mutePos = z + 12;
                    if (m8mutesolo[z])
                    {
                        m8mutesolo[z] = false;
                        MM::sendNoteOff(mutePos, 0, midiMacroConfig.midiMacroChan);
                    }
                }
                return; // break;
            }
            else if (e.down() && (thisKey == 2))
            {
                // ?
                return; // break;
            }
            else if (e.down() && (thisKey == 3))
            {
                // return to mixer
                // hold shift 4 left 1 down, release shift
                MM::sendNoteOn(1, 1, midiMacroConfig.midiMacroChan); // Shift
                delay(40);
                MM::sendNoteOn(6, 1, midiMacroConfig.midiMacroChan); // Up
                delay(20);
                MM::sendNoteOff(6, 0, midiMacroConfig.midiMacroChan);
                delay(40);
                MM::sendNoteOn(4, 1, midiMacroConfig.midiMacroChan); // Left
                delay(20);
                MM::sendNoteOff(4, 0, midiMacroConfig.midiMacroChan);
                delay(40);
                MM::sendNoteOn(4, 1, midiMacroConfig.midiMacroChan); // Left
                delay(20);
                MM::sendNoteOff(4, 0, midiMacroConfig.midiMacroChan);
                delay(40);
                MM::sendNoteOn(4, 1, midiMacroConfig.midiMacroChan); // Left
                delay(20);
                MM::sendNoteOff(4, 0, midiMacroConfig.midiMacroChan);
                delay(40);
                MM::sendNoteOn(4, 1, midiMacroConfig.midiMacroChan); // Left
                delay(20);
                MM::sendNoteOff(4, 0, midiMacroConfig.midiMacroChan);
                delay(40);
                MM::sendNoteOn(7, 1, midiMacroConfig.midiMacroChan); // Down
                delay(20);
                MM::sendNoteOff(7, 0, midiMacroConfig.midiMacroChan);
                MM::sendNoteOff(1, 0, midiMacroConfig.midiMacroChan);

                return; // break;
            }
            else if (e.down() && (thisKey == 4))
            {
                // snap save
                MM::sendNoteOn(1, 1, midiMacroConfig.midiMacroChan); // Shift
                delay(40);
                MM::sendNoteOn(3, 1, midiMacroConfig.midiMacroChan); // Option
                delay(40);
                MM::sendNoteOff(3, 0, midiMacroConfig.midiMacroChan);
                MM::sendNoteOff(1, 0, midiMacroConfig.midiMacroChan);

                return; // break;
            }
            else if (e.down() && (thisKey == 5))
            {
                // snap load
                MM::sendNoteOn(1, 1, midiMacroConfig.midiMacroChan); // Shift
                delay(40);
                MM::sendNoteOn(2, 1, midiMacroConfig.midiMacroChan); // Edit
                delay(40);
                MM::sendNoteOff(2, 0, midiMacroConfig.midiMacroChan);
                MM::sendNoteOff(1, 0, midiMacroConfig.midiMacroChan);

                // then reset mutes/solos
                for (int z = 0; z < 16; z++)
                {
                    if (m8mutesolo[z])
                    {
                        m8mutesolo[z] = false;
                    }
                }

                return; // break;
            }
            else if (e.down() && (thisKey == 6))
            {
                // release all solos
                for (int z = 8; z < 16; z++)
                {
                    int mutePos = z + 12;
                    if (m8mutesolo[z])
                    {
                        m8mutesolo[z] = false;
                        MM::sendNoteOff(mutePos, 0, midiMacroConfig.midiMacroChan);
                    }
                }
                return; // break;
            }
            else if (e.down() && (thisKey == 7))
            {
                // ??
                return; // break;
            }
            else if (e.down() && (thisKey == 8))
            {
                // ??
                return; // break;
            }
            else if (e.down() && (thisKey == 9))
            {
                // waveform
                MM::sendNoteOn(6, 1, midiMacroConfig.midiMacroChan); // Up
                MM::sendNoteOn(7, 1, midiMacroConfig.midiMacroChan); // Down
                MM::sendNoteOn(5, 1, midiMacroConfig.midiMacroChan); // Right
                MM::sendNoteOn(4, 1, midiMacroConfig.midiMacroChan); // Left
                delay(40);

                MM::sendNoteOff(6, 0, midiMacroConfig.midiMacroChan); // Up
                MM::sendNoteOff(7, 0, midiMacroConfig.midiMacroChan); // Down
                MM::sendNoteOff(5, 0, midiMacroConfig.midiMacroChan); // Right
                MM::sendNoteOff(4, 0, midiMacroConfig.midiMacroChan); // Left

                return; // break;
            }
            else if (e.down() && (thisKey == 10))
            {
                // play
                MM::sendNoteOn(0, 1, midiMacroConfig.midiMacroChan); // Play
                delay(40);
                MM::sendNoteOff(0, 0, midiMacroConfig.midiMacroChan); // Play

                //								MM::sendNoteOn(1, 1, midiMacroChan); // Shift
                //								MM::sendNoteOn(3, 1, midiMacroChan); // Option
                //								MM::sendNoteOn(2, 1, midiMacroChan); // Edit
                //								MM::sendNoteOn(6, 1, midiMacroChan); // Up
                //								MM::sendNoteOn(7, 1, midiMacroChan); // Down
                //								MM::sendNoteOn(4, 1, midiMacroChan); // Left
                //								MM::sendNoteOn(5, 1, midiMacroChan); // Right
                return; // break;
            }
        }
    }

    void MidiMacroM8::drawLEDs()
    {
        omxLeds.updateBlinkStates();
        auto blinkState = omxLeds.getBlinkState();

        auto color5 = blinkState ? ORANGE : LEDOFF;
        auto color6 = blinkState ? RED : LEDOFF;

        strip.setPixelColor(0, BLUE);
        strip.setPixelColor(1, ORANGE);  // all mute
        strip.setPixelColor(3, LIME);    // MIXER
        strip.setPixelColor(4, CYAN);    // snap load
        strip.setPixelColor(5, MAGENTA); // snap save

        for (int m = 11; m < LED_COUNT - 8; m++)
        {
            if (m8mutesolo[m - 11])
            {
                strip.setPixelColor(m, color5);
            }
            else
            {
                strip.setPixelColor(m, ORANGE);
            }
        }

        strip.setPixelColor(6, RED); // all solo
        for (int m = 19; m < LED_COUNT; m++)
        {
            if (m8mutesolo[m - 11])
            {
                strip.setPixelColor(m, color6);
            }
            else
            {
                strip.setPixelColor(m, RED);
            }
        }
        strip.setPixelColor(2, LEDOFF);
        strip.setPixelColor(7, LEDOFF);
        strip.setPixelColor(8, LEDOFF);

        strip.setPixelColor(9, YELLOW); // WAVES
        strip.setPixelColor(10, BLUE);  // PLAY
    }

    void MidiMacroM8::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        auto amt = enc.accel(5); 

        if(page == CHPAGE_1)
        {
            if (param == 0)
            {
                // chancePerc_ = constrain(chancePerc_ + amt, 0, 255);
            }
        }
        omxDisp.setDirty();
    }

    void MidiMacroM8::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        int8_t page = params_.getSelPage();

        switch (page)
        {
        case CHPAGE_1:
        {
            // omxDisp.legends[0] = "PERC";
            // omxDisp.legends[1] = "";
            // omxDisp.legends[2] = "";
            // omxDisp.legends[3] = "";
            // omxDisp.legendVals[0] = -127;
            // omxDisp.legendVals[1] = -127;
            // omxDisp.legendVals[2] = -127;
            // omxDisp.legendVals[3] = -127;

            // uint8_t perc = ((chancePerc_ / 255.0f) * 100);
            // String msg = String(perc) + "%";
            // omxDisp.legendText[0] = msg.c_str();
        }
        break;
        default:
            break;
        }

        omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
    }
}