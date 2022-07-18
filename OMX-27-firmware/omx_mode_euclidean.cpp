#include "omx_mode_euclidean.h"
#include "config.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "sequencer.h"
#include "euclidean_sequencer.h"
#include "ClearUI.h"
#include "noteoffs.h"
#include "MM.h"
#include "logic_util.h"

using namespace euclidean;

enum EucModePage {
    EUCLID_DENSITY,
    EUCLID_XY,
    EUCLID_NOTES,
    EUCLID_CONFIG
};

OmxModeEuclidean::OmxModeEuclidean()
{
    midiKeyboard.setMidiMode();
}

void OmxModeEuclidean::InitSetup()
{
    initSetup = true;
}

void OmxModeEuclidean::onModeActivated()
{
    if (!initSetup)
    {
        InitSetup();
    }

    sequencer.playing = false;
    stopSequencers();
    aux_ = false;
    f1_ = false;
    f2_ = false;
    f3_ = false;
    fNone_ = true;
    // grids_.stop();
    // grids_.loadSnapShot(grids_.playingPattern);
    // gridsAUX = false;
}

void OmxModeEuclidean::startSequencers()
{
    for (u_int8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].start();
    }
}
void OmxModeEuclidean::stopSequencers()
{
    for (u_int8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].stop();
    }
	MM::stopClock();
    pendingNoteOffs.allOff();
}

void OmxModeEuclidean::onClockTick()
{
    // euclids[0].clockTick();

    // for (u_int8_t i = 0; i < kNumEuclids; i++)
    // {
    //     euclids[i].clockTick();
    // }
}

// void OmxModeEuclidean::drawEuclidPattern(bool *pattern, uint8_t steps)
// {
//     if(steps == 0 || steps == 1) return;

//     int16_t steponHeight = 3;
//     int16_t stepoffHeight = 1;
//     int16_t stepWidth = 2;
//     int16_t halfh = gridh / 2;
//     int16_t halfw = gridw / 2;

//     int16_t stepint = gridw / steps - 1;

//     display.drawLine(0, halfh, gridw, halfh, HALFWHITE);

//     for (int i = 0; i < steps; i++)
//     {
//         int16_t xPos = stepint * i;
//         int16_t yPos = gridw;

//         if (pattern[i])
//         {
//             display.fillRect(xPos, yPos, stepWidth, steponHeight, WHITE);

//         }
//         else
//         {
//             display.fillRect(xPos, yPos, stepWidth, stepoffHeight, WHITE);
//         }
//     }

//     omxDisp.setDirty();
// }

// void OmxModeEuclidean::printEuclidPattern(bool *pattern, uint8_t steps)
// {
//     String sOut = "";
//     for (uint8_t i = 0; i < steps; i++)
//     {
//         sOut += (pattern[i] ? "X" : "-");
//     }
//     Serial.println(sOut.c_str());
// }

void OmxModeEuclidean::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
    if(midiModeception){
        midiKeyboard.onPotChanged(potIndex, prevValue, newValue, analogDelta);
        return;
    }

    bool valuesChanged = false;

    EuclideanSequencer* activeEuclid = &euclids[0];

    if(potIndex == 0){
        // uint8_t prevRotation = activeEuclid->getRotation();
        // uint8_t rotation = map(newValue, 0, 127, 0, 32);
        // valuesChanged = rotation != prevRotation;

        activeEuclid->setRotation(map(newValue, 0, 127, 0, 32));
    }
    else if(potIndex == 1){
        // uint8_t prevEvents = activeEuclid->getEvents();
        // events = map(newValue, 0, 127, 0, 32);
        // valuesChanged = events != prevEvents;

        activeEuclid->setEvents(map(newValue, 0, 127, 0, 32));
    }
    else if(potIndex == 2){
        // uint8_t prevSteps = steps;
        // steps = map(newValue, 0, 127, 0, 32);
        // valuesChanged = steps != prevSteps;

        activeEuclid->setSteps(map(newValue, 0, 127, 0, 32));
    }

    else if (potIndex == 4)
    {
        uint8_t prevRes = euclids[0].getClockDivMult();
        uint8_t newres = map(newValue, 0, 127, 0, 6);
        euclids[0].setClockDivMult(newres);

        // omxDisp.displayMessage(newres);

        if (newres != prevRes)
        {
            omxDisp.displayMessageTimed(String(multValues[newres]), 10);
        }
    }

    // if(activeEuclid->isDirty()){
    //     // Serial.println((String)"rotation: " + rotation + " events: " + events + " steps: " + steps);

    //     // EuclideanMath::generateEuclidPattern(euclidPattern, rotation, events, steps);
    //     // printEuclidPattern(euclidPattern, steps);
    //     // drawEuclidPattern(euclidPattern, steps);

    // }

    omxDisp.setDirty();

    // Serial.println((String)"AnalogDelta: " + analogDelta);

    // Only change page for significant difference
    // bool autoSelectParam = analogDelta >= 10 && page == GRIDS_DENSITY;

    // if (potIndex < 4)
    // {
    //     if (autoSelectParam)
    //     {
    //         // grids_.setDensity(potIndex, newValue * 2);
    //         // setParam(GRIDS_DENSITY, potIndex + 1);
    //     }

    //     omxDisp.setDirty();
    // }
    // else if (potIndex == 4)
    // {
        
    // }
}

void OmxModeEuclidean::loopUpdate()
{
    if (midiModeception)
    {
        midiKeyboard.loopUpdate();
        return;
    }

    auto keyState = midiSettings.keyState;

    f1_ = keyState[1] && !keyState[2];
    f2_ = !keyState[1] && keyState[2];
    f3_ = keyState[1] && keyState[2];
    fNone_ = !keyState[1] && !keyState[2];



    // bool testProb = probResult(sequencer.getCurrentPattern()->steps[sequencer.seqPos[sequencer.playingPattern]].prob);

	if (sequencer.playing)
	{
		uint32_t playstepmicros = micros();

        euclids[0].clockTick(playstepmicros, clockConfig.step_micros);
	}
}

void OmxModeEuclidean::setParam(uint8_t pageIndex, uint8_t paramPosition)
{
    int p = pageIndex * NUM_DISP_PARAMS + paramPosition;
    setParam(p);
    omxDisp.setDirty();
}

void OmxModeEuclidean::setParam(uint8_t paramIndex)
{
    if (paramIndex >= 0)
    {
        param = paramIndex % kNumParams;
    }
    else
    {
        param = (paramIndex + kNumParams) % kNumParams;
    }
    page = param / NUM_DISP_PARAMS;

    // if(instLockView_ && page == GRIDS_DENSITY)
    // {
    //     int pIndex = param % NUM_DISP_PARAMS;
    //     if(pIndex > 0){
    //         lockedInst_ = pIndex - 1;
    //     }
    // }
}

void OmxModeEuclidean::onEncoderChanged(Encoder::Update enc)
{
    if (midiModeception)
    {
        midiKeyboard.onEncoderChanged(enc);
        return;
    }

    // if (f1_)
    // {
    //     // Change selected param while holding F1
    //     if (enc.dir() < 0) // if turn CCW
    //     { 
    //         setParam(param - 1);
    //         omxDisp.setDirty();
    //     }
    //     else if (enc.dir() > 0) // if turn CW
    //     { 
    //         setParam(param + 1);
    //         omxDisp.setDirty();
    //     }

    //     return; // break;
    // }

    // auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)

    // int paramStep = param % 5;

    // if (paramStep != 0) // Page select mode if 0
    // {
    //     switch (page)
    //     {
    //     default:
    //         break;
    //     }
    // }
    // omxDisp.setDirty();
}

void OmxModeEuclidean::onEncoderButtonDown()
{
    if (midiModeception)
    {
        midiKeyboard.onEncoderButtonDown();
        return;
    }

    // param = (param + 1 ) % kNumParams;
    // setParam(param);
}

void OmxModeEuclidean::onEncoderButtonDownLong()
{
    if (midiModeception)
    {
        midiKeyboard.onEncoderButtonDownLong();
        return;
    }
}

bool OmxModeEuclidean::shouldBlockEncEdit()
{
    if (midiModeception)
    {
        return midiKeyboard.shouldBlockEncEdit();
    }

    return false;
}

void OmxModeEuclidean::saveActivePattern(uint8_t pattIndex)
{
    // grids_.saveSnapShot(pattIndex);
    // omxDisp.displayMessage((String) "Saved " + (pattIndex + 1));
}

void OmxModeEuclidean::loadActivePattern(uint8_t pattIndex)
{
    // grids_.loadSnapShot(pattIndex);
    // omxDisp.displayMessage((String) "Load " + (pattIndex + 1));
}

void OmxModeEuclidean::onKeyUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();

    if (midiModeception)
    {
        midiKeyboard.onKeyUpdate(e);

        if (midiSettings.keyState[0] && e.down() && thisKey == 26)
        {
            midiModeception = false;
            midiSettings.midiAUX = false;
            omxDisp.setDirty();
            omxLeds.setDirty();
        }

        return;
    }

    // if (instLockView_)
    // {
    //     onKeyUpdateChanLock(e);
    //     return;
    // }
    // // auto keyState = midiSettings.keyState;
    if (!e.held())
    {
        if (e.down() && thisKey == 0) // Aux key down
        {
            // Sequencer shouldn't be a dependancy here but current is used to advance clocks. 
            if (sequencer.playing && aux_)
            {
                aux_ = false;
                stopSequencers();
                sequencer.playing = false;
            }
            else
            {
                aux_ = true;
                startSequencers();
                sequencer.playing = true;
            }
        }
        // else if (e.down() && e.clicks() == 0 && (thisKey > 2 && thisKey < 11))
        // {
        //     int patt = thisKey - 3;
            
        //     if (f2_)
        //     { 
        //         saveActivePattern(patt);
        //     }
        //     else if(fNone_)
        //     {
        //         loadActivePattern(patt);
        //     }
        // }
    }

    // if (fNone_)
    // {
    //     // Select Grid X param
    //     if (e.down() && (thisKey > 10 && thisKey < 15))
    //     {
    //         gridsSelected[thisKey - 11] = true;
    //         setParam(GRIDS_XY, 2);
    //         omxDisp.setDirty();
    //     }
    //     else if (!e.down() && (thisKey > 10 && thisKey < 15))
    //     {
    //         gridsSelected[thisKey - 11] = false;
    //         omxDisp.setDirty();
    //     }

    //     // Select Grid Y param
    //     if (e.down() && (thisKey > 14 && thisKey < 19))
    //     {
    //         gridsSelected[thisKey - 15] = true;
    //         setParam(GRIDS_XY, 3);
    //         omxDisp.setDirty();
    //     }
    //     else if (!e.down() && (thisKey > 14 && thisKey < 19))
    //     {
    //         gridsSelected[thisKey - 15] = false;
    //         omxDisp.setDirty();
    //     }

    //     // Select Grid X param
    //     if (e.down() && thisKey == 23) // Accent
    //     {
    //         setParam(GRIDS_XY, 1);
    //     }
    //     else if (e.down() && thisKey == 24) // Xaos
    //     {
    //         setParam(GRIDS_XY, 4);
    //     }
    //     else if (e.down() && thisKey == 26) // BPM
    //     {
    //         setParam(GRIDS_CONFIG, 4);
    //     }
    // }
    // if(f1_)
    // {
    //     // Quick Select Note
    //     if (e.down() && (thisKey > 10 && thisKey < 15))
    //     {
    //         quickSelectInst(thisKey - 11);
    //     }

    //     if (e.down() && thisKey == 26)
    //     {
    //         midiKeyboard.onModeActivated();
    //         midiModeception = true;
    //         omxDisp.setDirty();
    //         omxLeds.setDirty();
    //     }

    //     // else if (!e.down() && (thisKey > 10 && thisKey < 15))
    //     // {
    //     // }

    //     // Select Grid Y param
    //     // if (e.down() && (thisKey > 14 && thisKey < 19))
    //     // {
    //     //     setParam(GRIDS_NOTES * NUM_DISP_PARAMS + (thisKey - 14));
    //     //     omxDisp.setDirty();
    //     // }
    //     // else if (!e.down() && (thisKey > 14 && thisKey < 19))
    //     // {
            
    //     // }
    // }
}

void OmxModeEuclidean::quickSelectInst(uint8_t instIndex)
{
    // if(instLockView_ && lockedInst_ == instIndex) return;

    // instLockView_ = true;
    // // justLocked_ = true; // Uncomment to immediately switch to channel view
    // lockedInst_ = instIndex;

    // if (page == GRIDS_DENSITY || page == GRIDS_NOTES)
    // {
    //     setParam(page, lockedInst_ + 1);
    // }

    // omxDisp.displayMessage((String) "Inst " + (lockedInst_ + 1));
    // omxDisp.setDirty();
}

void OmxModeEuclidean::onKeyHeldUpdate(OMXKeypadEvent e)
{
    if (midiModeception)
    {
        midiKeyboard.onKeyHeldUpdate(e);
        return;
    }
}

void OmxModeEuclidean::updateLEDs()
{
    if (midiModeception)
    {
        return;
    }

    omxLeds.updateBlinkStates();

    bool blinkState = omxLeds.getBlinkState();

    // if (instLockView_)
    // {
    //     int64_t instLockColor =  paramSelColors[lockedInst_];

    //     // Always blink to show you're in mode, don't need differation between playing or not since the playhead makes this obvious
    //     // auto color1 = blinkState ? instLockColor : LEDOFF;
    //     // strip.setPixelColor(0, color1);

    //     if (sequencer.playing)
    //     {
    //         // Blink left/right keys for octave select indicators.
    //         auto color1 = blinkState ? instLockColor : LEDOFF;
    //         strip.setPixelColor(0, color1);
    //     }
    //     else
    //     {
    //         strip.setPixelColor(0, instLockColor);
    //     }
    // }
    // else
    // {
    //     if (sequencer.playing)
    //     {
    //         // Blink left/right keys for octave select indicators.
    //         auto color1 = blinkState ? LIME : LEDOFF;
    //         strip.setPixelColor(0, color1);
    //     }
    //     else
    //     {
    //         strip.setPixelColor(0, LEDOFF);
    //     }
    // }

    // Function Keys
    if (f3_)
    {
        auto f3Color = blinkState ? LEDOFF : FUNKTHREE;
        strip.setPixelColor(1, f3Color);
        strip.setPixelColor(2, f3Color);
    }
    else
    {
        auto f1Color = (f1_ && blinkState) ? LEDOFF : FUNKONE;
        strip.setPixelColor(1, f1Color);

        auto f2Color = (f2_ && blinkState) ? LEDOFF : FUNKTWO;
        strip.setPixelColor(2, f2Color);
    }


    // if (instLockView_)
    // {
    //     updateLEDsChannelView();
    // }
    // else
    // {
    //     updateLEDsPatterns();

    //     // Set 16 key leds to off to prevent them from sticking on after screensaver. 
    //     for (int k = 0; k < 16; k++)
    //     {
    //         strip.setPixelColor(k + 11, LEDOFF);
    //     }

    //     if (fNone_ || f2_)
    //         updateLEDsFNone();
    //     else if (f1_)
    //         updateLEDsF1();
    // }

    omxLeds.setDirty();
}

void OmxModeEuclidean::updateLEDsFNone()
{
    bool blinkState = omxLeds.getBlinkState();

    auto keyState = midiSettings.keyState;

    // for (int k = 0; k < 4; k++)
    // {
    //     // Change color of 4 GridX keys when pushed
    //     // auto kColor = keyState[k + 11] ? (blinkState ? paramSelColors[k] : LEDOFF) : PINK;
    //     auto kColor = keyState[k + 11] ? (blinkState ? paramSelColors[k] : LEDOFF) : BLUE;

    //     strip.setPixelColor(k + 11, kColor);
    // }

    // for (int k = 4; k < 8; k++)
    // {
    //     // Change color of 4 GridY keys when pushed
    //     // auto kColor = keyState[k + 11] ? (blinkState ? paramSelColors[k % 4] : LEDOFF) : GREEN;
    //     auto kColor = keyState[k + 11] ? (blinkState ? paramSelColors[k % 4] : LEDOFF) : LTCYAN;
    //     strip.setPixelColor(k + 11, kColor);
    // }

    // for (int k = 0; k < 4; k++)
    // {
    //     bool triggered = grids_.getChannelTriggered(k);
    //     // Change color of 4 GridY keys when pushed
    //     auto kColor = triggered ? paramSelColors[k] : LEDOFF;
    //     strip.setPixelColor(k + 19, kColor);
    // }

    // strip.setPixelColor(23, (keyState[23] ? LBLUE : BLUE)); // Accent
    // strip.setPixelColor(24, (keyState[24] ? WHITE : ORANGE)); // Xaos
    // strip.setPixelColor(26, (keyState[26] ? WHITE : MAGENTA)); // BPM

}

void OmxModeEuclidean::updateLEDsF1()
{
    // bool blinkState = omxLeds.getBlinkState();
    // auto keyState = midiSettings.keyState;

    // // updateLEDsChannelView();

    // for (int k = 0; k < 4; k++)
    // {
    //     // Change color of 4 GridX keys when pushed
    //     auto kColor = keyState[k + 11] ? (blinkState ? paramSelColors[k] : LEDOFF) : ORANGE;
    //     strip.setPixelColor(k + 11, kColor);
    // }

    // for (int k = 4; k < 8; k++)
    // {
    //     strip.setPixelColor(k + 11, LEDOFF);
    // }

    // strip.setPixelColor(26, ORANGE);
}

void OmxModeEuclidean::updateLEDsPatterns()
{
    // int patternNum = grids_.playingPattern;

    // // LEDS for top row
    // for (int j = 3; j < LED_COUNT - 16; j++)
    // {
    //     auto pColor = (j == patternNum + 3) ? seqColors[patternNum] : LEDOFF;
    //     strip.setPixelColor(j, pColor);
    // }
}

void OmxModeEuclidean::setupPageLegends()
{
    // omxDisp.clearLegends();

    // omxDisp.dispPage = page + 1;

    // switch (page)
    // {
    // case EUCLID_CONFIG:
    // {
    //     // omxDisp.legends[0] = "DS 1";
    //     // omxDisp.legends[1] = "DS 2";
    //     // omxDisp.legends[2] = "DS 3";
    //     // omxDisp.legends[3] = "DS 4";
    //     // omxDisp.legendVals[0] = grids_.getDensity(0);
    //     // omxDisp.legendVals[1] = grids_.getDensity(1);
    //     // omxDisp.legendVals[2] = grids_.getDensity(2);
    //     // omxDisp.legendVals[3] = grids_.getDensity(3);
    // }
    // break;
    // default:
    //     break;
    // }
}

void OmxModeEuclidean::onDisplayUpdate()
{
    if (midiModeception)
    {
        midiKeyboard.onDisplayUpdate();

        if (midiSettings.midiAUX)
        {
            strip.setPixelColor(26, RED); // Highlight aux exit key
        }

        return;
    }

    // updateLEDs();

    if (omxDisp.isDirty())
    { 
        if (!encoderConfig.enc_edit)
        {
            omxDisp.drawEuclidPattern(euclids[0].getPattern() , euclids[0].getSteps());
            // int pselected = param % NUM_DISP_PARAMS;
            // setupPageLegends();
            // omxDisp.dispGenericMode(pselected);
        }
    }

    // if (!encoderConfig.enc_edit)
    // {
    //     omxDisp.drawEuclidPattern(euclids[0].getPattern(), euclids[0].getSteps());
    //     // int pselected = param % NUM_DISP_PARAMS;
    //     // setupPageLegends();
    //     // omxDisp.dispGenericMode(pselected);
    // }

    // if (!encoderConfig.enc_edit)
    //     {
    //         omxDisp.drawEuclidPattern(euclids[0].getPattern() , euclids[0].getSteps());
    //         // int pselected = param % NUM_DISP_PARAMS;
    //         // setupPageLegends();
    //         // omxDisp.dispGenericMode(pselected);
    //     }
}

        


void OmxModeEuclidean::SetScale(MusicScales *scale)
{
    midiKeyboard.SetScale(scale);
}

// int OmxModeGrids::serializedPatternSize(bool eeprom)
// {
//     return sizeof(grids::SnapShotSettings);
// }

// grids::SnapShotSettings* OmxModeGrids::getPattern(uint8_t patternIndex)
// {
//     return grids_.getSnapShot(patternIndex);
// }

// void OmxModeGrids::setPattern(uint8_t patternIndex, grids::SnapShotSettings snapShot)
// {
//     grids_.setSnapShot(patternIndex, snapShot);
// }
