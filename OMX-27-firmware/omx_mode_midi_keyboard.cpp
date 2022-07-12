#include "omx_mode_midi_keyboard.h"
#include "config.h"
#include "colors.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"

void OmxModeMidiKeyboard::OnPotChanged(int potIndex, int potValue)
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
        if (miparam == 0)
        {
            if (u.dir() < 0)
            { // if turn ccw
                MM::sendControlChange(CC_OM2, 0, sysSettings.midiChannel);
            }
            else if (u.dir() > 0)
            { // if turn cw
                MM::sendControlChange(CC_OM2, 127, sysSettings.midiChannel);
            }
        }
        dirtyDisplay = true;
    }

    if (midiAUX)
    {
        // change MIDI Background Color
        // midiBg_Hue = constrain(midiBg_Hue + (amt * 32), 0, 65534); // 65535
        break;
    }
    // CHANGE PAGE
    if (miparam == 0 || miparam == 5 || miparam == 10)
    {
        mmpage = constrain(mmpage + amt, 0, 2);
        miparam = mmpage * NUM_DISP_PARAMS;
    }
    // PAGE ONE
    if (miparam == 2)
    {
        int newchan = constrain(sysSettings.midiChannel + amt, 1, 16);
        if (newchan != sysSettings.midiChannel)
        {
            sysSettings.midiChannel = newchan;
        }
    }
    else if (miparam == 1)
    {
        // set octave
        newoctave = constrain(octave + amt, -5, 4);
        if (newoctave != octave)
        {
            octave = newoctave;
        }
    }
    // PAGE TWO
    if (miparam == 6)
    {
        int newrrchan = constrain(midiRRChannelCount + amt, 1, 16);
        if (newrrchan != midiRRChannelCount)
        {
            midiRRChannelCount = newrrchan;
            if (midiRRChannelCount == 1)
            {
                midiRoundRobin = false;
            }
            else
            {
                midiRoundRobin = true;
            }
        }
    }
    else if (miparam == 7)
    {
        midiRRChannelOffset = constrain(midiRRChannelOffset + amt, 0, 15);
    }
    else if (miparam == 8)
    {
        currpgm = constrain(currpgm + amt, 0, 127);

        if (midiRoundRobin)
        {
            for (int q = midiRRChannelOffset + 1; q < midiRRChannelOffset + midiRRChannelCount + 1; q++)
            {
                MM::sendProgramChange(currpgm, q);
            }
        }
        else
        {
            MM::sendProgramChange(currpgm, sysSettings.midiChannel);
        }
    }
    else if (miparam == 9)
    {
        currbank = constrain(currbank + amt, 0, 127);
        // Bank Select is 2 mesages
        MM::sendControlChange(0, 0, sysSettings.midiChannel);
        MM::sendControlChange(32, currbank, sysSettings.midiChannel);
        MM::sendProgramChange(currpgm, sysSettings.midiChannel);
    }
    // PAGE THREE
    if (miparam == 11)
    {
        potbank = constrain(potbank + amt, 0, NUM_CC_BANKS - 1);
    }
    if (miparam == 12)
    {
        midiSoftThru = constrain(midiSoftThru + amt, 0, 1);
    }
    if (miparam == 13)
    {
        midiMacro = constrain(midiMacro + amt, 0, nummacromodes);
    }
    if (miparam == 14)
    {
        midiMacroChan = constrain(midiMacroChan + amt, 1, 16);
    }

    dirtyDisplay = true;
}

void OmxModeMidiKeyboard::onEncoderButtonDown()
{
    if (organelleMotherMode)
    {
        miparam = (miparam + 1) % NUM_DISP_PARAMS;
        // MM::sendControlChange(CC_OM1,100,sysSettings.midiChannel);
    }
    else
    {
        // switch midi oct/chan selection
        miparam = (miparam + 1) % 15;
        mmpage = miparam / NUM_DISP_PARAMS;
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
    if (midiSettings.midiMacro)
    {
        if (e.clicks() == 2 && thisKey == 0)
        {
            m8AUX = !m8AUX;
            if (!m8AUX)
            {
                for (int m = 1; m < LED_COUNT; m++)
                {
                    strip.setPixelColor(m, LEDOFF);
                }
            }
        }
        if (m8AUX)
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
                        MM::sendNoteOn(mutePos, 1, midiMacroChan);
                    }
                    else
                    {
                        MM::sendNoteOff(mutePos, 0, midiMacroChan);
                    }
                    break;
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
                            MM::sendNoteOff(mutePos, 0, midiMacroChan);
                        }
                    }
                    break;
                }
                else if (e.down() && (thisKey == 2))
                {
                    // ?
                    break;
                }
                else if (e.down() && (thisKey == 3))
                {
                    // return to mixer
                    // hold shift 4 left 1 down, release shift
                    MM::sendNoteOn(1, 1, midiMacroChan); // Shift
                    delay(40);
                    MM::sendNoteOn(6, 1, midiMacroChan); // Up
                    delay(20);
                    MM::sendNoteOff(6, 0, midiMacroChan);
                    delay(40);
                    MM::sendNoteOn(4, 1, midiMacroChan); // Left
                    delay(20);
                    MM::sendNoteOff(4, 0, midiMacroChan);
                    delay(40);
                    MM::sendNoteOn(4, 1, midiMacroChan); // Left
                    delay(20);
                    MM::sendNoteOff(4, 0, midiMacroChan);
                    delay(40);
                    MM::sendNoteOn(4, 1, midiMacroChan); // Left
                    delay(20);
                    MM::sendNoteOff(4, 0, midiMacroChan);
                    delay(40);
                    MM::sendNoteOn(4, 1, midiMacroChan); // Left
                    delay(20);
                    MM::sendNoteOff(4, 0, midiMacroChan);
                    delay(40);
                    MM::sendNoteOn(7, 1, midiMacroChan); // Down
                    delay(20);
                    MM::sendNoteOff(7, 0, midiMacroChan);
                    MM::sendNoteOff(1, 0, midiMacroChan);

                    break;
                }
                else if (e.down() && (thisKey == 4))
                {
                    // snap save
                    MM::sendNoteOn(1, 1, midiMacroChan); // Shift
                    delay(40);
                    MM::sendNoteOn(3, 1, midiMacroChan); // Option
                    delay(40);
                    MM::sendNoteOff(3, 0, midiMacroChan);
                    MM::sendNoteOff(1, 0, midiMacroChan);

                    break;
                }
                else if (e.down() && (thisKey == 5))
                {
                    // snap load
                    MM::sendNoteOn(1, 1, midiMacroChan); // Shift
                    delay(40);
                    MM::sendNoteOn(2, 1, midiMacroChan); // Edit
                    delay(40);
                    MM::sendNoteOff(2, 0, midiMacroChan);
                    MM::sendNoteOff(1, 0, midiMacroChan);

                    // then reset mutes/solos
                    for (int z = 0; z < 16; z++)
                    {
                        if (m8mutesolo[z])
                        {
                            m8mutesolo[z] = false;
                        }
                    }

                    break;
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
                            MM::sendNoteOff(mutePos, 0, midiMacroChan);
                        }
                    }
                    break;
                }
                else if (e.down() && (thisKey == 7))
                {
                    // ??
                    break;
                }
                else if (e.down() && (thisKey == 8))
                {
                    // ??
                    break;
                }
                else if (e.down() && (thisKey == 9))
                {
                    // waveform
                    MM::sendNoteOn(6, 1, midiMacroChan); // Up
                    MM::sendNoteOn(7, 1, midiMacroChan); // Down
                    MM::sendNoteOn(5, 1, midiMacroChan); // Right
                    MM::sendNoteOn(4, 1, midiMacroChan); // Left
                    delay(40);

                    MM::sendNoteOff(6, 0, midiMacroChan); // Up
                    MM::sendNoteOff(7, 0, midiMacroChan); // Down
                    MM::sendNoteOff(5, 0, midiMacroChan); // Right
                    MM::sendNoteOff(4, 0, midiMacroChan); // Left

                    break;
                }
                else if (e.down() && (thisKey == 10))
                {
                    // play
                    MM::sendNoteOn(0, 1, midiMacroChan); // Play
                    delay(40);
                    MM::sendNoteOff(0, 0, midiMacroChan); // Play

                    //								MM::sendNoteOn(1, 1, midiMacroChan); // Shift
                    //								MM::sendNoteOn(3, 1, midiMacroChan); // Option
                    //								MM::sendNoteOn(2, 1, midiMacroChan); // Edit
                    //								MM::sendNoteOn(6, 1, midiMacroChan); // Up
                    //								MM::sendNoteOn(7, 1, midiMacroChan); // Down
                    //								MM::sendNoteOn(4, 1, midiMacroChan); // Left
                    //								MM::sendNoteOn(5, 1, midiMacroChan); // Right
                    break;
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
                if (midiAUX)
                {
                    if (thisKey == 11 || thisKey == 12)
                    {
                        int amt = thisKey == 11 ? -1 : 1;
                        newoctave = constrain(octave + amt, -5, 4);
                        if (newoctave != octave)
                        {
                            octave = newoctave;
                        }
                    }
                    else if (thisKey == 1 || thisKey == 2)
                    {
                        int chng = thisKey == 1 ? -1 : 1;
                        miparam = constrain((miparam + chng) % 15, 0, 14);
                        mmpage = miparam / NUM_DISP_PARAMS;
                    }
                }
                else
                {
                    midiNoteOn(thisKey, defaultVelocity, sysSettings.midiChannel);
                }
            }
            else
            {
                midiNoteOn(thisKey, defaultVelocity, sysSettings.midiChannel);
            }
        }
        else if (!e.down() && thisKey != 0)
        {
            midiNoteOff(thisKey, sysSettings.midiChannel);
        }
    }
    //				Serial.println(e.clicks());

    // AUX KEY
    if (e.down() && thisKey == 0)
    {
        // Hard coded Organelle stuff
        //					MM::sendControlChange(CC_AUX, 100, sysSettings.midiChannel);

        if (!m8AUX)
        {
            midiAUX = true;
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
        if (midiAUX)
        {
            midiAUX = false;
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
    midi_leds(); // SHOW LEDS
    if (dirtyDisplay)
    { // DISPLAY
        if (!encoderConfig.enc_edit)
        {
            int pselected = miparam % NUM_DISP_PARAMS;
            if (mmpage == 0)
            {
                dispGenericMode(SUBMODE_MIDI, pselected);
            }
            else if (mmpage == 1)
            {
                dispGenericMode(SUBMODE_MIDI2, pselected);
            }
            else if (mmpage == 2)
            {
                dispGenericMode(SUBMODE_MIDI3, pselected);
            }
        }
    }
}

// incoming midi note on
void OmxModeMidiKeyboard::inMidiNoteOn(byte channel, byte note, byte velocity)
{
    if(organelleMotherMode) return;

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

void OmxModeMidiKeyboard::inMidiNoteOff(byte channel, byte note, byte velocity) {
    if(organelleMotherMode) return;

    int whatoct = (note / 12);
		int thisKey;
		if ( (whatoct % 2) == 0) {
			thisKey = note - (12 * whatoct);
		} else {
			thisKey = note - (12 * whatoct) + 12;
		}
		strip.setPixelColor(midiKeyMap[thisKey], LEDOFF);         //  Set pixel's color (in RAM)
	//	dirtyPixels = true;
		strip.show();
		omxDisp.setDirty();
    
}
