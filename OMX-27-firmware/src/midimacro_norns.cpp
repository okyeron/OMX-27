#include "midimacro_norns.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "MM.h"
#include "consts.h"
#include "colors.h"

namespace midimacro
{
    enum NornsPage {
        NRNPAGE_ENC1,
        NRNPAGE_ENC2,
        NRNPAGE_ENC3
    };

    MidiMacroNorns::MidiMacroNorns()
    {
        params_.addPage(1); // Enc1
        params_.addPage(1); // Enc2
        params_.addPage(1); // Enc3

        encoderSelect_ = true;
    }

    String MidiMacroNorns::getName()
    {
        return String("NORNS");
    }

    void MidiMacroNorns::onEnabled()
    {
    }

    void MidiMacroNorns::onDisabled()
    {
    }
   
    void MidiMacroNorns::loopUpdate()
    {
    }

    void MidiMacroNorns::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
    {
        omxUtil.sendPots(potIndex, midiMacroConfig.midiMacroChan);
    }

    void MidiMacroNorns::onKeyUpdate(OMXKeypadEvent e)
    {
        int thisKey = e.key();
        // int keyPos = thisKey - 11;

        if (thisKey != 0 && !e.held())
        {
            if ((thisKey >= 6 && thisKey <= 10) || (thisKey >= 19))
            {
                if (e.down())
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
                    if (thisKey == but1_)
                    {
                        MM::sendControlChange(ccBut1_, 127, midiMacroConfig.midiMacroChan);
                    }
                    else if (thisKey == but2_)
                    {
                        MM::sendControlChange(ccBut2_, 127, midiMacroConfig.midiMacroChan);
                    }
                    else if (thisKey == but3_)
                    {
                        MM::sendControlChange(ccBut3_, 127, midiMacroConfig.midiMacroChan);
                    }
                    else if (thisKey == enc1_)
                    {
                        params_.setSelPageAndParam(0,0);
                        encoderSelect_ = false;
                        omxDisp.setDirty();
                    }
                    else if (thisKey == enc2_)
                    {
                        params_.setSelPageAndParam(1,0);
                        encoderSelect_ = false;
                        omxDisp.setDirty();
                    }
                    else if (thisKey == enc3_)
                    {
                        params_.setSelPageAndParam(2,0);
                        encoderSelect_ = false;
                        omxDisp.setDirty();
                    }
                    else if (thisKey == keyUp_)
                    {
                        // params_.setSelPageAndParam(1,0);
                        // encoderSelect_ = false;
                        MM::sendControlChange(ccEnc2_, 63, midiMacroConfig.midiMacroChan);
                        delay(20);
                        MM::sendControlChange(ccEnc2_, 63, midiMacroConfig.midiMacroChan);
                    }
                    else if (thisKey == keyDown_)
                    {
                        // params_.setSelPageAndParam(1,0);
                        // encoderSelect_ = false;
                        MM::sendControlChange(ccEnc2_, 65, midiMacroConfig.midiMacroChan);
                        delay(20);
                        MM::sendControlChange(ccEnc2_, 65, midiMacroConfig.midiMacroChan);
                    }
                    else if (thisKey == keyLeft_)
                    {
                        // params_.setSelPageAndParam(2,0);
                        // encoderSelect_ = false;
                        MM::sendControlChange(ccEnc3_, 63, midiMacroConfig.midiMacroChan);
                        delay(20);
                        MM::sendControlChange(ccEnc3_, 63, midiMacroConfig.midiMacroChan);
                    }
                    else if (thisKey == keyRight_)
                    {
                        // params_.setSelPageAndParam(2,0);
                        // encoderSelect_ = false;
                        MM::sendControlChange(ccEnc3_, 65, midiMacroConfig.midiMacroChan);
                        delay(20);
                        MM::sendControlChange(ccEnc3_, 65, midiMacroConfig.midiMacroChan);
                    }
                }
                else
                {
                    if (thisKey == but1_)
                    {
                        MM::sendControlChange(ccBut1_, 0, midiMacroConfig.midiMacroChan);
                    }
                    else if (thisKey == but2_)
                    {
                        MM::sendControlChange(ccBut2_, 0, midiMacroConfig.midiMacroChan);
                    }
                    else if (thisKey == but3_)
                    {
                        MM::sendControlChange(ccBut3_, 0, midiMacroConfig.midiMacroChan);
                    }
                }
            }
        }

        omxLeds.setDirty();
    }

    void MidiMacroNorns::drawLEDs()
    {
        // omxLeds.updateBlinkStates();

        if(omxLeds.isDirty() == false)
        {
            return;
        }

        // auto blinkState = omxLeds.getBlinkState();

        omxLeds.setAllLEDS(0, 0, 0);

        strip.setPixelColor(0, BLUE); // aux

        strip.setPixelColor(but1_, midiSettings.keyState[but1_] ? LTYELLOW : ORANGE);
        strip.setPixelColor(but2_, midiSettings.keyState[but2_] ? LTYELLOW : ORANGE);
        strip.setPixelColor(but3_, midiSettings.keyState[but3_] ? LTYELLOW : ORANGE);

        strip.setPixelColor(enc1_, RED);
        strip.setPixelColor(enc2_, RED);
        strip.setPixelColor(enc3_, RED);

        strip.setPixelColor(keyUp_, midiSettings.keyState[keyUp_] ? LTCYAN : BLUE);
        strip.setPixelColor(keyDown_, midiSettings.keyState[keyDown_] ? LTCYAN : BLUE);
        strip.setPixelColor(keyLeft_, midiSettings.keyState[keyLeft_] ? LTCYAN : BLUE);
        strip.setPixelColor(keyRight_, midiSettings.keyState[keyRight_] ? LTCYAN : BLUE);

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

    void MidiMacroNorns::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        // int8_t param = params_.getSelParam();

        // auto amt = enc.accel(5);

        uint8_t encCC = 0;

        if (page == NRNPAGE_ENC1)
            encCC = ccEnc1_;
        else if (page == NRNPAGE_ENC2)
            encCC = ccEnc2_;
        else if (page == NRNPAGE_ENC3)
            encCC = ccEnc3_;

        if (enc.dir() > 0)
        {
            MM::sendControlChange(encCC, 65, midiMacroConfig.midiMacroChan);
        }
        else if (enc.dir() < 0)
        {
            MM::sendControlChange(encCC, 63, midiMacroConfig.midiMacroChan);
        }
      
        omxDisp.setDirty();
    }

    void MidiMacroNorns::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        int8_t page = params_.getSelPage();

        bool genericDisp = true;

        switch (page)
        {
        case NRNPAGE_ENC1:
        {
            omxDisp.dispGenericModeLabel("Enc 1", params_.getNumPages(), params_.getSelPage());
            genericDisp = false;
        }
        break;
        case NRNPAGE_ENC2:
        {
            omxDisp.dispGenericModeLabel("Enc 2", params_.getNumPages(), params_.getSelPage());
            genericDisp = false;
        }
        break;
        case NRNPAGE_ENC3:
        {
            omxDisp.dispGenericModeLabel("Enc 3", params_.getNumPages(), params_.getSelPage());
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