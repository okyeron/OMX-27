#include "omx_mode_midi_keyboard.h"
#include "config.h"
#include "colors.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "MM.h"
#include "music_scales.h"

void OmxModeMidiKeyboard::InitSetup()
{
    initSetup = true;
}

void OmxModeMidiKeyboard::onModeActivated()
{
    // auto init when activated
    if(!initSetup){
        InitSetup();
    }
}

void OmxModeMidiKeyboard::changePage(int amt)
{
    midiPageParams.mmpage = constrain(midiPageParams.mmpage + amt, 0, midiPageParams.numPages - 1);
    midiPageParams.miparam = midiPageParams.mmpage * NUM_DISP_PARAMS;
}

void OmxModeMidiKeyboard::setParam(int paramIndex)
{
    if(paramIndex >= 0)
    {
        midiPageParams.miparam  = paramIndex % midiPageParams.numParams;
    }
    else
    {
        midiPageParams.miparam  = (paramIndex + midiPageParams.numParams) % midiPageParams.numParams;
    }

    // midiPageParams.miparam  = (midiPageParams.miparam  + 1) % 15;
    midiPageParams.mmpage = midiPageParams.miparam  / NUM_DISP_PARAMS;
}

void OmxModeMidiKeyboard::onPotChanged(int potIndex, int potValue)
{
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

void OmxModeMidiKeyboard::onEncoderChanged(Encoder::Update enc)
{
    if (organelleMotherMode)
    {
        // CHANGE PAGE
        if (midiPageParams.miparam  == 0)
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
        // change MIDI Background Color
        // midiBg_Hue = constrain(midiBg_Hue + (amt * 32), 0, 65534); // 65535
        return; // break;
    }

    auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)

    // CHANGE PAGE
    // if (midiPageParams.miparam  == 0 || midiPageParams.miparam  == 5 || midiPageParams.miparam  == 10)
    if(midiPageParams.miparam % 5 == 0)
    {
        changePage(amt);
        // midiPageParams.mmpage = constrain(midiPageParams.mmpage + amt, 0, 2);
        // midiPageParams.miparam  = midiPageParams.mmpage * NUM_DISP_PARAMS;
    }
    int pageIndex = 0;

    // PAGE ONE
    if (midiPageParams.miparam  == pageIndex + 1)
    {
        // set octave
        midiSettings.newoctave = constrain(midiSettings.octave + amt, -5, 4);
        if (midiSettings.newoctave != midiSettings.octave)
        {
            midiSettings.octave = midiSettings.newoctave;
        }
    }
    else if (midiPageParams.miparam  == pageIndex + 2)
    {
        int newchan = constrain(sysSettings.midiChannel + amt, 1, 16);
        if (newchan != sysSettings.midiChannel)
        {
            sysSettings.midiChannel = newchan;
        }
    }

    pageIndex = 5;
    
    // PAGE TWO
    if (midiPageParams.miparam  == pageIndex + 1)
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
    else if (midiPageParams.miparam  == pageIndex + 2)
    {
        midiSettings.midiRRChannelOffset = constrain(midiSettings.midiRRChannelOffset + amt, 0, 15);
    }
    else if (midiPageParams.miparam  == pageIndex + 3)
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
    else if (midiPageParams.miparam  == pageIndex + 4)
    {
        midiSettings.currbank = constrain(midiSettings.currbank + amt, 0, 127);
        // Bank Select is 2 mesages
        MM::sendControlChange(0, 0, sysSettings.midiChannel);
        MM::sendControlChange(32, midiSettings.currbank, sysSettings.midiChannel);
        MM::sendProgramChange(midiSettings.currpgm, sysSettings.midiChannel);
    }
    pageIndex = 10;
    // PAGE THREE
    if (midiPageParams.miparam  == pageIndex + 1)
    {
        potSettings.potbank = constrain(potSettings.potbank + amt, 0, NUM_CC_BANKS - 1);
    }
    if (midiPageParams.miparam  == pageIndex + 2)
    {
        midiSettings.midiSoftThru = constrain(midiSettings.midiSoftThru + amt, 0, 1);
    }
    if (midiPageParams.miparam  == pageIndex + 3)
    {
        midiMacroConfig.midiMacro = constrain(midiMacroConfig.midiMacro + amt, 0, nummacromodes);
    }
    if (midiPageParams.miparam  == pageIndex + 4)
    {
        midiMacroConfig.midiMacroChan = constrain(midiMacroConfig.midiMacroChan + amt, 1, 16);
    }
    pageIndex = 15;
    // PAGE FOUR - SCALES
    if (midiPageParams.miparam  == pageIndex + 1)
    {
        int prevRoot = scaleConfig.scaleRoot;
        scaleConfig.scaleRoot = constrain(scaleConfig.scaleRoot  + amt, 0, 12 - 1);
        if(prevRoot != scaleConfig.scaleRoot) setScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
    }
    if (midiPageParams.miparam  == pageIndex + 2)
    {
        int prevPat = scaleConfig.scalePattern;
        scaleConfig.scalePattern = constrain(scaleConfig.scalePattern  + amt, -1, getNumScales() - 1);
        if(prevPat != scaleConfig.scalePattern) 
        {
            omxDisp.displayMessage(scaleNames[scaleConfig.scalePattern]);
            setScale(scaleConfig.scaleRoot, scaleConfig.scalePattern);
        }
    }
    if (midiPageParams.miparam  == pageIndex + 3)
    {
        // midiMacroConfig.midiMacro = constrain(midiMacroConfig.midiMacro + amt, 0, nummacromodes);
    }
    if (midiPageParams.miparam  == pageIndex + 4)
    {
        // midiMacroConfig.midiMacroChan = constrain(midiMacroConfig.midiMacroChan + amt, 1, 16);
    }

    omxDisp.setDirty();
}

void OmxModeMidiKeyboard::onEncoderButtonDown()
{
    if (organelleMotherMode)
    {
        setParam(midiPageParams.miparam + 1);

        // midiPageParams.miparam  = (midiPageParams.miparam  + 1) % NUM_DISP_PARAMS;
        // MM::sendControlChange(CC_OM1,100,sysSettings.midiChannel);
    }
    else
    {
        // switch midi oct/chan selection
        setParam(midiPageParams.miparam + 1);
        // midiPageParams.miparam  = (midiPageParams.miparam  + 1) % 15;
        // midiPageParams.mmpage = midiPageParams.miparam  / NUM_DISP_PARAMS;
    }
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

void OmxModeMidiKeyboard::onKeyUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();
    int keyPos = thisKey - 11;

    // ### KEY PRESS EVENTS
    if (midiMacroConfig.midiMacro)
    {
        if (e.clicks() == 2 && thisKey == 0)
        {
            midiMacroConfig.m8AUX = !midiMacroConfig.m8AUX;
            if (!midiMacroConfig.m8AUX)
            {
                for (int m = 1; m < LED_COUNT; m++)
                {
                    strip.setPixelColor(m, LEDOFF);
                }
            }
        }
        if (midiMacroConfig.m8AUX)
        {
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
    }

    // REGULAR KEY PRESSES
    if (!e.held())
    { // IGNORE LONG PRESS EVENTS
        if (e.down() && thisKey != 0)
        {
            // Serial.println(" pressed");
            if (thisKey == 11 || thisKey == 12 || thisKey == 1 || thisKey == 2)
            {
                if (midiSettings.midiAUX)
                {
                    if (thisKey == 11 || thisKey == 12)
                    {
                        int amt = thisKey == 11 ? -1 : 1;
                        midiSettings.newoctave = constrain(midiSettings.octave + amt, -5, 4);
                        if (midiSettings.newoctave != midiSettings.octave)
                        {
                            midiSettings.octave = midiSettings.newoctave;
                        }
                    }
                    else if (thisKey == 1 || thisKey == 2)
                    {
                        int chng = thisKey == 1 ? -1 : 1;

                        setParam(constrain((midiPageParams.miparam  + chng) % midiPageParams.numParams, 0, midiPageParams.numParams - 1));
                        // midiPageParams.miparam  = constrain((midiPageParams.miparam  + chng) % 15, 0, 14);
                        // midiPageParams.mmpage = midiPageParams.miparam  / NUM_DISP_PARAMS;
                    }
                }
                else
                {
                    omxUtil.midiNoteOn(thisKey, midiSettings.defaultVelocity, sysSettings.midiChannel);
                }
            }
            else
            {
                omxUtil.midiNoteOn(thisKey, midiSettings.defaultVelocity, sysSettings.midiChannel);
            }
        }
        else if (!e.down() && thisKey != 0)
        {
            omxUtil.midiNoteOff(thisKey, sysSettings.midiChannel);
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

void OmxModeMidiKeyboard::updateLEDs()
{
}

void OmxModeMidiKeyboard::onDisplayUpdate()
{
    // playingPattern = 0; 		// DEFAULT MIDI MODE TO THE FIRST PATTERN SLOT
    omxLeds.drawMidiLeds(); // SHOW LEDS

    if (omxDisp.isDirty())
    { // DISPLAY
        if (!encoderConfig.enc_edit)
        {
            int pselected = midiPageParams.miparam  % NUM_DISP_PARAMS;
            if (midiPageParams.mmpage == 0) // SUBMODE_MIDI
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
                omxDisp.dispPage = 1;

                omxDisp.dispGenericMode(pselected);
            }
            else if (midiPageParams.mmpage == 1) // SUBMODE_MIDI2
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
                omxDisp.dispPage = 2;
                omxDisp.dispGenericMode(pselected);
            }
            else if (midiPageParams.mmpage == 2) // SUBMODE_MIDI3
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
                omxDisp.dispPage = 3;
                omxDisp.dispGenericMode(pselected);
            }
            else if (midiPageParams.mmpage == 3) // SCALES
            {
                omxDisp.clearLegends();
                omxDisp.legends[0] = "ROOT"; 
                omxDisp.legends[1] = "SCALE"; 
                omxDisp.legends[2] = ""; 
                omxDisp.legends[3] = "";
                omxDisp.legendVals[0] = -127;
                if(scaleConfig.scalePattern < 0){
                    omxDisp.legendVals[1] = -127;
                    omxDisp.legendText[1] = "Off";
                }
                else 
                {
                    omxDisp.legendVals[1] = scaleConfig.scalePattern;
                }

                omxDisp.legendVals[2] = -127;
                omxDisp.legendVals[3] = -127;

                omxDisp.legendText[0] = noteNames[scaleConfig.scaleRoot];
                omxDisp.legendText[2] = "";
                omxDisp.legendText[3] = "";
                omxDisp.dispPage = 4;
                omxDisp.dispGenericMode(pselected);
            }
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
