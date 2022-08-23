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
        M8PAGE_MUTESOLO,
        M8PAGE_CONTROL
    };

    MidiMacroM8::MidiMacroM8()
    {
        params_.addPage(1); // Mute / Solo
        params_.addPage(1); // Control
        encoderSelect_ = true;

        for(uint8_t i = 0; i < 16; i++)
        {
            m8mutesolo_[i] = false;
        }
    }

    String MidiMacroM8::getName()
    {
        return String("M8");
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

    void MidiMacroM8::onEncoderButtonDown()
    {
        encoderSelect_ = true;
        // encoderSelect_ = !encoderSelect_;
        omxDisp.setDirty();
    }

    void MidiMacroM8::onKeyUpdate(OMXKeypadEvent e)
    {
        int thisKey = e.key();
        int keyPos = thisKey - 11;

        int8_t page = params_.getSelPage();

        if (page == M8PAGE_MUTESOLO)
        {
            if (!e.held())
            {
                if (e.down() && (thisKey > 10 && thisKey < 27))
                {
                    // Mutes / Solos
                    m8mutesolo_[keyPos] = !m8mutesolo_[keyPos];
                    int mutePos = keyPos + 12;
                    if (m8mutesolo_[keyPos])
                    {
                        if (keyPos < 8)
                        {
                            omxDisp.displayMessage("Mute");
                        }
                        else
                        {
                            omxDisp.displayMessage("Solo");
                        }
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
                    omxDisp.displayMessage("Unmute all");
                    // release all mutes
                    for (int z = 0; z < 8; z++)
                    {
                        int mutePos = z + 12;
                        if (m8mutesolo_[z])
                        {
                            m8mutesolo_[z] = false;
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
                    omxDisp.displayMessage("Goto Mixer");
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
                    omxDisp.displayMessage("Goto Mixer");

                    return; // break;
                }
                else if (e.down() && (thisKey == 4))
                {
                    omxDisp.displayMessage("Save snapshot");
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
                    omxDisp.displayMessage("Load snapshot");
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
                        if (m8mutesolo_[z])
                        {
                            m8mutesolo_[z] = false;
                        }
                    }

                    return; // break;
                }
                else if (e.down() && (thisKey == 6))
                {
                    omxDisp.displayMessage("Unsolo all");
                    // release all solos
                    for (int z = 8; z < 16; z++)
                    {
                        int mutePos = z + 12;
                        if (m8mutesolo_[z])
                        {
                            m8mutesolo_[z] = false;
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
                    omxDisp.displayMessage("Waveform");
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
                    omxDisp.displayMessage("Play");
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
        else if(page == M8PAGE_CONTROL)
        {
            if (thisKey != 0 && !e.held())
            {
                if ((thisKey >= 6 && thisKey <= 10) || (thisKey >= 19))
                {
                    if(e.down())
                    {
                        DoNoteOn(thisKey);
                    }
                    else
                    {
                        DoNoteOff(thisKey);
                    }
                }
                else
                {

                    if (e.down())
                    {
                        if (thisKey == keyUp_)
                            MM::sendNoteOn(6, 1, midiMacroConfig.midiMacroChan);
                        if (thisKey == keyDown_)
                            MM::sendNoteOn(7, 1, midiMacroConfig.midiMacroChan);
                        if (thisKey == keyLeft_)
                            MM::sendNoteOn(4, 1, midiMacroConfig.midiMacroChan);
                        if (thisKey == keyRight_)
                            MM::sendNoteOn(5, 1, midiMacroConfig.midiMacroChan);

                        if (thisKey == keyOption_)
                            MM::sendNoteOn(3, 1, midiMacroConfig.midiMacroChan);
                        if (thisKey == keyEdit_)
                            MM::sendNoteOn(2, 1, midiMacroConfig.midiMacroChan);

                        if (thisKey == keyShift_)
                            MM::sendNoteOn(1, 1, midiMacroConfig.midiMacroChan);
                        if (thisKey == keyPlay_)
                            MM::sendNoteOn(0, 1, midiMacroConfig.midiMacroChan);
                    }
                    else
                    {
                        if (thisKey == keyUp_)
                            MM::sendNoteOff(6, 0, midiMacroConfig.midiMacroChan);
                        if (thisKey == keyDown_)
                            MM::sendNoteOff(7, 0, midiMacroConfig.midiMacroChan);
                        if (thisKey == keyLeft_)
                            MM::sendNoteOff(4, 0, midiMacroConfig.midiMacroChan);
                        if (thisKey == keyRight_)
                            MM::sendNoteOff(5, 0, midiMacroConfig.midiMacroChan);

                        if (thisKey == keyOption_)
                            MM::sendNoteOff(3, 0, midiMacroConfig.midiMacroChan);
                        if (thisKey == keyEdit_)
                            MM::sendNoteOff(2, 0, midiMacroConfig.midiMacroChan);

                        if (thisKey == keyShift_)
                            MM::sendNoteOff(1, 0, midiMacroConfig.midiMacroChan);
                        if (thisKey == keyPlay_)
                            MM::sendNoteOff(0, 0, midiMacroConfig.midiMacroChan);
                    }
                }
            }
        }

        omxLeds.setDirty();
    }

    void MidiMacroM8::drawLEDs()
    {
        // omxLeds.updateBlinkStates();

        if(omxLeds.isDirty() == false)
        {
            return;
        }

        auto blinkState = omxLeds.getBlinkState();

        omxLeds.setAllLEDS(0, 0, 0);

        int8_t page = params_.getSelPage();

        if (page == M8PAGE_MUTESOLO)
        {
            auto color5 = blinkState ? ORANGE : LEDOFF;
            auto color6 = blinkState ? RED : LEDOFF;

            strip.setPixelColor(0, BLUE);
            strip.setPixelColor(1, ORANGE);  // all mute
            strip.setPixelColor(3, LIME);    // MIXER
            strip.setPixelColor(4, CYAN);    // snap load
            strip.setPixelColor(5, MAGENTA); // snap save

            for (int m = 11; m < LED_COUNT - 8; m++)
            {
                if (m8mutesolo_[m - 11])
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
                if (m8mutesolo_[m - 11])
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
        else if (page == M8PAGE_CONTROL)
        {
            strip.setPixelColor(0, BLUE); // aux

            strip.setPixelColor(keyUp_, ORANGE); // up
            strip.setPixelColor(keyDown_, ORANGE); // down
            strip.setPixelColor(keyLeft_, RED); // left
            strip.setPixelColor(keyRight_, RED); // right

            strip.setPixelColor(keyOption_, BLUE); // option
            strip.setPixelColor(keyEdit_, BLUE); // edit
            strip.setPixelColor(keyShift_, GREEN); // shift
            strip.setPixelColor(keyPlay_, GREEN); // play

            for (int q = 1; q < LED_COUNT; q++)
            {
                if ((q >= 6 && q <= 10) || (q >= 19))
                {
                    if (midiSettings.midiKeyState[q] == -1)
                    {
                        if (colorConfig.midiBg_Hue == 0)
                        {
                            strip.setPixelColor(q, omxLeds.getKeyColor(scale_, q)); // set off or in scale
                        }
                        else if (colorConfig.midiBg_Hue == 32)
                        {
                            strip.setPixelColor(q, LOWWHITE);
                        }
                        else
                        {
                            strip.setPixelColor(q, strip.ColorHSV(colorConfig.midiBg_Hue, colorConfig.midiBg_Sat, colorConfig.midiBg_Brightness));
                        }
                    }
                    else
                    {
                        strip.setPixelColor(q, MIDINOTEON);
                    }
                }
            }
        }
    }

    void MidiMacroM8::onEncoderChangedEditParam(Encoder::Update enc)
    {
        // int8_t page = params_.getSelPage();
        // int8_t param = params_.getSelParam();

        // auto amt = enc.accel(5); 
        
        omxDisp.setDirty();
    }

    void MidiMacroM8::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        int8_t page = params_.getSelPage();

        bool genericDisp = true;

        switch (page)
        {
        case M8PAGE_MUTESOLO:
        {
            omxDisp.dispGenericModeLabel("Mute Solo", params_.getNumPages(), params_.getSelPage());
            genericDisp = false;
        }
        break;
        case M8PAGE_CONTROL:
        {
            omxDisp.dispGenericModeLabel("Control", params_.getNumPages(), params_.getSelPage());
            genericDisp = false;
        }
        break;
        default:
            break;
        }

        if (genericDisp)
        {
            omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
        }
    }
}