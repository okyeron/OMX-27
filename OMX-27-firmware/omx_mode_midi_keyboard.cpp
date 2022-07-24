#include "omx_mode_midi_keyboard.h"
#include "config.h"
#include "colors.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "MM.h"
#include "music_scales.h"

OmxModeMidiKeyboard::OmxModeMidiKeyboard()
{
    // Add 4 pages
    params.addPage(4);
    params.addPage(4);
    params.addPage(4);
    params.addPage(4);

    subModeMidiFx.setNoteOutputFunc(&OmxModeMidiKeyboard::onNotePostFXForwarder, this);
}

void OmxModeMidiKeyboard::InitSetup()
{
    initSetup = true;
}

void OmxModeMidiKeyboard::onModeActivated()
{
    // auto init when activated
    if (!initSetup)
    {
        InitSetup();
    }

    params.setSelPageAndParam(0, 0);
    encoderSelect = true;
}

// void OmxModeMidiKeyboard::changePage(int amt)
// {
//     midiPageParams.mmpage = constrain(midiPageParams.mmpage + amt, 0, midiPageParams.numPages - 1);
//     midiPageParams.miparam = midiPageParams.mmpage * NUM_DISP_PARAMS;
// }

// void OmxModeMidiKeyboard::setParam(int paramIndex)
// {
//     if (paramIndex >= 0)
//     {
//         midiPageParams.miparam = paramIndex % midiPageParams.numParams;
//     }
//     else
//     {
//         midiPageParams.miparam = (paramIndex + midiPageParams.numParams) % midiPageParams.numParams;
//     }

//     // midiPageParams.miparam  = (midiPageParams.miparam  + 1) % 15;
//     midiPageParams.mmpage = midiPageParams.miparam / NUM_DISP_PARAMS;
// }

void OmxModeMidiKeyboard::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
     if (isSubmodeEnabled() && activeSubmode->usesPots())
    {
        activeSubmode->onPotChanged(potIndex, prevValue, newValue, analogDelta);
        return;
    }

    if (midiMacroConfig.midiMacro)
    {
        omxUtil.sendPots(potIndex, midiMacroConfig.midiMacroChan);
    }
    else
    {
        omxUtil.sendPots(potIndex, sysSettings.midiChannel);
    }

    omxDisp.setDirty();
}

void OmxModeMidiKeyboard::loopUpdate()
{
    if (isSubmodeEnabled())
    {
        activeSubmode->loopUpdate();
    }
}


// Handles selecting params using encoder
void OmxModeMidiKeyboard::onEncoderChangedSelectParam(Encoder::Update enc)
{
    if(enc.dir() == 0) return;

    if (enc.dir() < 0) // if turn CCW
    {
        params.decrementParam();
    }
    else if (enc.dir() > 0) // if turn CW
    {
        params.incrementParam();
    }

    omxDisp.setDirty();
}

void OmxModeMidiKeyboard::onEncoderChanged(Encoder::Update enc)
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onEncoderChanged(enc);
        return;
    }

    if (encoderSelect)
    {
        onEncoderChangedSelectParam(enc);
        return;
    }

    if (organelleMotherMode)
    {
        // CHANGE PAGE
        if (params.getSelParam() == 0)
        {
            if (enc.dir() < 0)
            { // if turn ccw
                MM::sendControlChange(CC_OM2, 0, sysSettings.midiChannel);
            }
            else if (enc.dir() > 0)
            { // if turn cw
                MM::sendControlChange(CC_OM2, 127, sysSettings.midiChannel);
            }
        }

        omxDisp.setDirty();
    }

    if (midiSettings.midiAUX)
    {
        // if (enc.dir() < 0)
        // { // if turn ccw
        //     setParam(midiPageParams.miparam - 1);
        //     omxDisp.setDirty();
        // }
        // else if (enc.dir() > 0)
        // { // if turn cw
        //     setParam(midiPageParams.miparam + 1);
        //     omxDisp.setDirty();
        // }

        // change MIDI Background Color
        // midiBg_Hue = constrain(midiBg_Hue + (amt * 32), 0, 65534); // 65535
        return; // break;
    }

    auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)

    int8_t selPage = params.getSelPage() + 1; // Add one for readability
    int8_t selParam = params.getSelParam() + 1;

    // PAGE ONE
    if (selPage == 1)
    {
        if (selParam == 1)
        {
            // set octave
            midiSettings.newoctave = constrain(midiSettings.octave + amt, -5, 4);
            if (midiSettings.newoctave != midiSettings.octave)
            {
                midiSettings.octave = midiSettings.newoctave;
            }
        }
        else if (selParam == 2)
        {
            int newchan = constrain(sysSettings.midiChannel + amt, 1, 16);
            if (newchan != sysSettings.midiChannel)
            {
                sysSettings.midiChannel = newchan;
            }
        }
    }
    // PAGE TWO
    else if (selPage == 2)
    {
        if (selParam == 1)
        {
            int newrrchan = constrain(midiSettings.midiRRChannelCount + amt, 1, 16);
            if (newrrchan != midiSettings.midiRRChannelCount)
            {
                midiSettings.midiRRChannelCount = newrrchan;
                if (midiSettings.midiRRChannelCount == 1)
                {
                    midiSettings.midiRoundRobin = false;
                }
                else
                {
                    midiSettings.midiRoundRobin = true;
                }
            }
        }
        else if (selParam == 2)
        {
            midiSettings.midiRRChannelOffset = constrain(midiSettings.midiRRChannelOffset + amt, 0, 15);
        }
        else if (selParam == 3)
        {
            midiSettings.currpgm = constrain(midiSettings.currpgm + amt, 0, 127);

            if (midiSettings.midiRoundRobin)
            {
                for (int q = midiSettings.midiRRChannelOffset + 1; q < midiSettings.midiRRChannelOffset + midiSettings.midiRRChannelCount + 1; q++)
                {
                    MM::sendProgramChange(midiSettings.currpgm, q);
                }
            }
            else
            {
                MM::sendProgramChange(midiSettings.currpgm, sysSettings.midiChannel);
            }
        }
        else if (selParam == 4)
        {
            midiSettings.currbank = constrain(midiSettings.currbank + amt, 0, 127);
            // Bank Select is 2 mesages
            MM::sendControlChange(0, 0, sysSettings.midiChannel);
            MM::sendControlChange(32, midiSettings.currbank, sysSettings.midiChannel);
            MM::sendProgramChange(midiSettings.currpgm, sysSettings.midiChannel);
        }
    }
    // PAGE THREE
    else if (selPage == 3)
    {
        if (selParam == 1)
        {
            potSettings.potbank = constrain(potSettings.potbank + amt, 0, NUM_CC_BANKS - 1);
        }
        if (selParam == 2)
        {
            midiSettings.midiSoftThru = constrain(midiSettings.midiSoftThru + amt, 0, 1);
        }
        if (selParam == 3)
        {
            midiMacroConfig.midiMacro = constrain(midiMacroConfig.midiMacro + amt, 0, nummacromodes);
        }
        if (selParam == 4)
        {
            midiMacroConfig.midiMacroChan = constrain(midiMacroConfig.midiMacroChan + amt, 1, 16);
        }
    }
    // PAGE FOUR - SCALES
    else if (selPage == 4)
    {
        if (selParam == 1)
        {
            int prevRoot = scaleConfig.scaleRoot;
            scaleConfig.scaleRoot = constrain(scaleConfig.scaleRoot + amt, 0, 12 - 1);
            if (prevRoot != scaleConfig.scaleRoot)
            {
                musicScale->calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
            }
        }
        if (selParam == 2)
        {
            int prevPat = scaleConfig.scalePattern;
            scaleConfig.scalePattern = constrain(scaleConfig.scalePattern + amt, -1, musicScale->getNumScales() - 1);
            if (prevPat != scaleConfig.scalePattern)
            {
                omxDisp.displayMessage(musicScale->getScaleName(scaleConfig.scalePattern));
                musicScale->calculateScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
            }
        }
        if (selParam == 3)
        {
            scaleConfig.lockScale = constrain(scaleConfig.lockScale + amt, 0, 1);
        }
        if (selParam == 4)
        {
            scaleConfig.group16 = constrain(scaleConfig.group16 + amt, 0, 1);
        }
    }

    omxDisp.setDirty();
}

void OmxModeMidiKeyboard::onEncoderButtonDown()
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onEncoderButtonDown();
        return;
    }

    encoderSelect = !encoderSelect;
    omxDisp.isDirty();
}

void OmxModeMidiKeyboard::onEncoderButtonUp()
{
    if (organelleMotherMode)
    {
        //				MM::sendControlChange(CC_OM1,0,sysSettings.midiChannel);
    }
}

void OmxModeMidiKeyboard::onEncoderButtonDownLong()
{
}

bool OmxModeMidiKeyboard::shouldBlockEncEdit()
{
    if (isSubmodeEnabled())
    {
        return activeSubmode->shouldBlockEncEdit();
    }

    return false;
}

void OmxModeMidiKeyboard::onKeyUpdate(OMXKeypadEvent e)
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onKeyUpdate(e);
        return;
    }

    int thisKey = e.key();

    // // Aux key debugging
    // if(thisKey == 0){
    //     const char* dwn = e.down() ? " Down: True" : " Down: False";
    //     Serial.println(String("Clicks: ") + String(e.clicks()) + dwn);
    // }

    // Aux double click toggle macro
    if (midiMacroConfig.midiMacro > 0)
    {
        if (!midiMacroConfig.m8AUX)
        {
            // Enter M8 Mode
            if (!e.down() && thisKey == 0 && e.clicks() == 2)
            {
                midiMacroConfig.m8AUX = true;
                midiSettings.midiAUX = false;
                return;
            }
        }
        else // M8 Macro mode
        {
            // Exit M8 mode
            if (!e.down() && thisKey == 0 && e.clicks() == 2)
            {
                midiMacroConfig.m8AUX = false;
                midiSettings.midiAUX = false;

                // Clear LEDs
                for (int m = 1; m < LED_COUNT; m++)
                {
                    strip.setPixelColor(m, LEDOFF);
                }
                return;
            }

            onKeyUpdateM8Macro(e);
            return;
        }
    }

    // REGULAR KEY PRESSES
    if (!e.held())
    { // IGNORE LONG PRESS EVENTS
        if (e.down() && thisKey != 0)
        {
            bool keyConsumed = false; // If used for aux, key will be consumed and not send notes.

            if (midiSettings.midiAUX) // Aux mode
            {
                if (thisKey == 11 || thisKey == 12) // Change Octave
                {
                    int amt = thisKey == 11 ? -1 : 1;
                    midiSettings.newoctave = constrain(midiSettings.octave + amt, -5, 4);
                    if (midiSettings.newoctave != midiSettings.octave)
                    {
                        midiSettings.octave = midiSettings.newoctave;
                    }
                    keyConsumed = true;
                }
                else if (thisKey == 1 || thisKey == 2) // Change Param selection
                {
                    if(thisKey == 1){
                        params.decrementParam();
                    }
                    else if(thisKey == 2){
                        params.incrementParam();
                    }
                    // int chng = thisKey == 1 ? -1 : 1;

                    // setParam(constrain((midiPageParams.miparam + chng) % midiPageParams.numParams, 0, midiPageParams.numParams - 1));
                    keyConsumed = true;
                }
                else if (e.down() && thisKey == 10)
                {
                    enableSubmode(&subModeMidiFx);
                    keyConsumed = true;
                }
                else if (thisKey == 26)
				{
					keyConsumed = true;
                }
            }

            if(!keyConsumed)
            {
                doNoteOn(thisKey);
                // omxUtil.midiNoteOn(musicScale, thisKey, midiSettings.defaultVelocity, sysSettings.midiChannel);
            }
        }
        else if (!e.down() && thisKey != 0)
        {
            doNoteOff(thisKey);
            // omxUtil.midiNoteOff(thisKey, sysSettings.midiChannel);
        }
    }
    //				Serial.println(e.clicks());

    // AUX KEY
    if (e.down() && thisKey == 0)
    {
        // Hard coded Organelle stuff
        //					MM::sendControlChange(CC_AUX, 100, sysSettings.midiChannel);

        if (!midiMacroConfig.m8AUX)
        {
            midiSettings.midiAUX = true;
        }

        //					if (midiAUX) {
        //						// STOP CLOCK
        //						Serial.println("stop clock");
        //					} else {
        //						// START CLOCK
        //						Serial.println("start clock");
        //					}
        //					midiAUX = !midiAUX;
    }
    else if (!e.down() && thisKey == 0)
    {
        // Hard coded Organelle stuff
        //					MM::sendControlChange(CC_AUX, 0, sysSettings.midiChannel);
        if (midiSettings.midiAUX)
        {
            midiSettings.midiAUX = false;
        }
        // turn off leds
        strip.setPixelColor(0, LEDOFF);
        strip.setPixelColor(1, LEDOFF);
        strip.setPixelColor(2, LEDOFF);
        strip.setPixelColor(11, LEDOFF);
        strip.setPixelColor(12, LEDOFF);
    }
}

void OmxModeMidiKeyboard::onKeyUpdateM8Macro(OMXKeypadEvent e)
{
    if (!midiMacroConfig.m8AUX)
        return;

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

void OmxModeMidiKeyboard::updateLEDs()
{
}

void OmxModeMidiKeyboard::onDisplayUpdate()
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onDisplayUpdate();
        return;
    }

    omxLeds.drawMidiLeds(musicScale); // SHOW LEDS

    if (omxDisp.isDirty())
    { // DISPLAY
        if (!encoderConfig.enc_edit)
        {
            if (params.getSelPage() == 0) // SUBMODE_MIDI
            {
                omxDisp.clearLegends();

                //			if (midiRoundRobin) {
                //				displaychan = rrChannel;
                //			}
                omxDisp.legends[0] = "OCT";
                omxDisp.legends[1] = "CH";
                omxDisp.legends[2] = "CC";
                omxDisp.legends[3] = "NOTE";
                omxDisp.legendVals[0] = (int)midiSettings.octave + 4;
                omxDisp.legendVals[1] = sysSettings.midiChannel;
                omxDisp.legendVals[2] = potSettings.potVal;
                omxDisp.legendVals[3] = midiSettings.midiLastNote;
            }
            else if (params.getSelPage() == 1) // SUBMODE_MIDI2
            {
                omxDisp.clearLegends();

                omxDisp.legends[0] = "RR";
                omxDisp.legends[1] = "RROF";
                omxDisp.legends[2] = "PGM";
                omxDisp.legends[3] = "BNK";
                omxDisp.legendVals[0] = midiSettings.midiRRChannelCount;
                omxDisp.legendVals[1] = midiSettings.midiRRChannelOffset;
                omxDisp.legendVals[2] = midiSettings.currpgm + 1;
                omxDisp.legendVals[3] = midiSettings.currbank;
            }
            else if (params.getSelPage() == 2) // SUBMODE_MIDI3
            {
                omxDisp.clearLegends();

                omxDisp.legends[0] = "PBNK"; // Potentiometer Banks
                omxDisp.legends[1] = "THRU"; // MIDI thru (usb to hardware)
                omxDisp.legends[2] = "MCRO"; // Macro mode
                omxDisp.legends[3] = "M-CH";
                omxDisp.legendVals[0] = potSettings.potbank + 1;
                omxDisp.legendVals[1] = -127;
                if (midiSettings.midiSoftThru)
                {
                    omxDisp.legendText[1] = "On";
                }
                else
                {
                    omxDisp.legendText[1] = "Off";
                }
                omxDisp.legendVals[2] = -127;
                omxDisp.legendText[2] = macromodes[midiMacroConfig.midiMacro];
                omxDisp.legendVals[3] = midiMacroConfig.midiMacroChan;
            }
            else if (params.getSelPage() == 3) // SCALES
            {
                omxDisp.clearLegends();
                omxDisp.legends[0] = "ROOT";
                omxDisp.legends[1] = "SCALE";
                omxDisp.legends[2] = "LOCK";
                omxDisp.legends[3] = "GROUP";
                omxDisp.legendVals[0] = -127;
                if (scaleConfig.scalePattern < 0)
                {
                    omxDisp.legendVals[1] = -127;
                    omxDisp.legendText[1] = "Off";
                }
                else
                {
                    omxDisp.legendVals[1] = scaleConfig.scalePattern;
                }

                omxDisp.legendVals[2] = -127;
                omxDisp.legendVals[3] = -127;

                omxDisp.legendText[0] = musicScale->getNoteName(scaleConfig.scaleRoot);
                omxDisp.legendText[2] = scaleConfig.lockScale ? "On" : "Off";
                omxDisp.legendText[3] = scaleConfig.group16 ? "On" : "Off";
            }

            omxDisp.dispGenericMode2(4, params.getSelPage(), params.getSelParam(), encoderSelect);
        }
    }
}

// incoming midi note on
void OmxModeMidiKeyboard::inMidiNoteOn(byte channel, byte note, byte velocity)
{
    if (organelleMotherMode)
        return;

    midiSettings.midiLastNote = note;
    int whatoct = (note / 12);
    int thisKey;
    uint32_t keyColor = MIDINOTEON;

    if ((whatoct % 2) == 0)
    {
        thisKey = note - (12 * whatoct);
    }
    else
    {
        thisKey = note - (12 * whatoct) + 12;
    }
    if (whatoct == 0)
    { // ORANGE,YELLOW,GREEN,MAGENTA,CYAN,BLUE,LIME,LTPURPLE
    }
    else if (whatoct == 1)
    {
        keyColor = ORANGE;
    }
    else if (whatoct == 2)
    {
        keyColor = YELLOW;
    }
    else if (whatoct == 3)
    {
        keyColor = GREEN;
    }
    else if (whatoct == 4)
    {
        keyColor = MAGENTA;
    }
    else if (whatoct == 5)
    {
        keyColor = CYAN;
    }
    else if (whatoct == 6)
    {
        keyColor = LIME;
    }
    else if (whatoct == 7)
    {
        keyColor = CYAN;
    }
    strip.setPixelColor(midiKeyMap[thisKey], keyColor); //  Set pixel's color (in RAM)
                                                        //	dirtyPixels = true;
    strip.show();
    omxDisp.setDirty();
}

void OmxModeMidiKeyboard::inMidiNoteOff(byte channel, byte note, byte velocity)
{
    if (organelleMotherMode)
        return;

    int whatoct = (note / 12);
    int thisKey;
    if ((whatoct % 2) == 0)
    {
        thisKey = note - (12 * whatoct);
    }
    else
    {
        thisKey = note - (12 * whatoct) + 12;
    }
    strip.setPixelColor(midiKeyMap[thisKey], LEDOFF); //  Set pixel's color (in RAM)
                                                      //	dirtyPixels = true;
    strip.show();
    omxDisp.setDirty();
}

void OmxModeMidiKeyboard::SetScale(MusicScales *scale)
{
    this->musicScale = scale;
}

void OmxModeMidiKeyboard::enableSubmode(SubmodeInterface *subMode)
{
    activeSubmode = subMode;
    activeSubmode->setEnabled(true);
    omxDisp.setDirty();
}

void OmxModeMidiKeyboard::disableSubmode()
{
    activeSubmode = nullptr;
    omxDisp.setDirty();
}

bool OmxModeMidiKeyboard::isSubmodeEnabled()
{
    if(activeSubmode == nullptr) return false;

    if(activeSubmode->isEnabled() == false){
        disableSubmode();
        return false;
    }

    return true;
}

void OmxModeMidiKeyboard::doNoteOn(uint8_t keyIndex)
{
    MidiNoteGroup noteGroup = omxUtil.midiNoteOn2(musicScale, keyIndex, midiSettings.defaultVelocity, sysSettings.midiChannel);

    if(noteGroup.noteNumber == 255) return;

    // Serial.println("doNoteOn: " + String(noteGroup.noteNumber));

    noteGroup.prevNoteNumber = noteGroup.noteNumber;

    subModeMidiFx.noteInput(noteGroup);
}
void OmxModeMidiKeyboard::doNoteOff(uint8_t keyIndex)
{
    MidiNoteGroup noteGroup = omxUtil.midiNoteOff2(keyIndex, sysSettings.midiChannel);

    if(noteGroup.noteNumber == 255) return;

    // Serial.println("doNoteOff: " + String(noteGroup.noteNumber));

    noteGroup.prevNoteNumber = noteGroup.noteNumber;
    subModeMidiFx.noteInput(noteGroup);
}

// // Called by a euclid sequencer when it triggers a note
// void OmxModeMidiKeyboard::onNoteTriggered(uint8_t euclidIndex, MidiNoteGroup note)
// {
//     // Serial.println("OmxModeEuclidean::onNoteTriggered " + String(euclidIndex) + " note: " + String(note.noteNumber));
    
//     subModeMidiFx.noteInput(note);

//     omxDisp.setDirty();
// }

// Called by the midiFX group when a note exits it's FX Pedalboard
void OmxModeMidiKeyboard::onNotePostFX(MidiNoteGroup note)
{
    if(note.noteOff)
    {
        // Serial.println("OmxModeMidiKeyboard::onNotePostFX noteOff: " + String(note.noteNumber));

        if (note.sendMidi)
        {
            MM::sendNoteOff(note.noteNumber, note.velocity, note.channel);
        }
        if (note.sendCV)
        {
            omxUtil.cvNoteOff();
        }
    }
    else
    {
        // Serial.println("OmxModeMidiKeyboard::onNotePostFX noteOn: " + String(note.noteNumber));

        if (note.sendMidi)
        {
            MM::sendNoteOn(note.noteNumber, note.velocity, note.channel);
        }
        if (note.sendCV)
        {
            omxUtil.cvNoteOn(note.noteNumber);
        }
    }

    // uint32_t noteOnMicros = note.noteonMicros; // TODO Might need to be set to current micros
    // pendingNoteOns.insert(note.noteNumber, note.velocity, note.channel, noteOnMicros, note.sendCV);

    // uint32_t noteOffMicros = noteOnMicros + (note.stepLength * clockConfig.step_micros);
    // pendingNoteOffs.insert(note.noteNumber, note.channel, noteOffMicros, note.sendCV);
}