#include "omx_mode_euclidean.h"
#include "config.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "sequencer.h"
#include "euclidean_sequencer.h"
// #include "ClearUI.h"
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

enum SelEucModePage {
    SELEUCLID_PAT,
    SELEUCLID_1,
    SELEUCLID_NOTES
};

OmxModeEuclidean::OmxModeEuclidean()
{
    midiKeyboard.setMidiMode();

    // Setup function pointers for note ons. 
    for (int i = 0; i < kNumEuclids; i++)
    {
        euclids[i].setNoteOutputFunc(&OmxModeEuclidean::onNoteTriggeredForwarder, this, i);
    }

    for(int i = 0; i < kNumMidiFXGroups; i++)
    {
        subModeMidiFx[i].setNoteOutputFunc(&OmxModeEuclidean::onNotePostFXForwarder, this);
    }

    polyRhythmMode = false;

    for (u_int8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].setPolyRhythmMode(polyRhythmMode);
        euclids[i].setClockDivMult(3);
        euclids[i].setPolyRClockDivMult(3);
    }

    selEucParams.addPage(1);
    selEucParams.addPage(4);
    selEucParams.addPage(4);

    euclids[0].setNoteNumber(60);
    euclids[1].setNoteNumber(64);
    euclids[2].setNoteNumber(67);
    euclids[3].setNoteNumber(71);
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

    omxLeds.setDirty();
    omxDisp.setDirty();

    encoderSelect_ = true;

    pendingNoteOffs.setNoteOffFunction(&OmxModeEuclidean::onPendingNoteOffForwarder, this);
}

void OmxModeEuclidean::startSequencers()
{
    for (u_int8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].start();
    }

    MM::startClock();
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

void OmxModeEuclidean::setPageAndParam(uint8_t pageIndex, uint8_t paramPosition)
{
    encoderSelect_ = false;
    selEucParams.setSelPage(pageIndex);
    setParam(paramPosition);
    omxDisp.setDirty();
}

void OmxModeEuclidean::setParam(uint8_t paramIndex)
{
    selEucParams.setSelParam(paramIndex);

    // // Select instrument on this page
    // if (instLockView_ && params.getSelPage() == GRIDS_DENSITY)
    // {
    //     lockedInst_ = paramIndex;
    // }
    omxDisp.setDirty();
}

void OmxModeEuclidean::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
    if (isSubmodeEnabled() && activeSubmode->usesPots())
    {
        activeSubmode->onPotChanged(potIndex, prevValue, newValue, analogDelta);
        return;
    }

    if(midiModeception){
        midiKeyboard.onPotChanged(potIndex, prevValue, newValue, analogDelta);
        return;
    }

    if(analogDelta < 2) return;

    // bool valuesChanged = false;

    EuclideanSequencer* activeEuclid = &euclids[selectedEuclid_];

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
    else if(potIndex == 3){
        // uint8_t prevSteps = steps;
        // steps = map(newValue, 0, 127, 0, 32);
        // valuesChanged = steps != prevSteps;

        uint8_t prevLength = activeEuclid->getNoteLength();
        uint8_t newLength = map(newValue, 0, 127, 0, euclidean::kNumEuclidNoteLengths - 1);

        activeEuclid->setNoteLength(newLength);

        if (prevLength != newLength)
        {
            omxDisp.displayMessageTimed(String(euclidean::kEuclidNoteLengths[newLength]), 10);
        }
    }

    else if (potIndex == 4)
    {
        uint8_t prevRes = activeEuclid->getClockDivMult();
        uint8_t newres = map(newValue, 0, 127, 0, 6);
        if (polyRhythmMode)
        {
            for (u_int8_t i = 0; i < kNumEuclids; i++)
            {
                euclids[i].setPolyRClockDivMult(newres);
            }
        }
        else
        {
            activeEuclid->setClockDivMult(newres);
        }

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

    omxLeds.setDirty();
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
    if (isSubmodeEnabled())
    {
        activeSubmode->loopUpdate();
        // return;
    }
    
    if (midiModeception)
    {
        midiKeyboard.loopUpdate();
        // return;
    }

    if (!isSubmodeEnabled() && !midiModeception)
    {
        auto keyState = midiSettings.keyState;

        f1_ = keyState[1] && !keyState[2];
        f2_ = !keyState[1] && keyState[2];
        f3_ = keyState[1] && keyState[2];
        fNone_ = !keyState[1] && !keyState[2];
    }

    // bool testProb = probResult(sequencer.getCurrentPattern()->steps[sequencer.seqPos[sequencer.playingPattern]].prob);

	// if (sequencer.playing)
	// {
	// 	uint32_t playstepmicros = micros();

    //     euclids[0].clockTick(playstepmicros, clockConfig.step_micros);
	// }

    uint32_t playstepmicros = micros();

    for (u_int8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].clockTick(playstepmicros, clockConfig.step_micros);
    }
}

// void OmxModeEuclidean::setParam(uint8_t pageIndex, uint8_t paramPosition)
// {
//     int p = pageIndex * NUM_DISP_PARAMS + paramPosition;
//     setParam(p);
//     omxDisp.setDirty();
// }

// void OmxModeEuclidean::setParam(uint8_t paramIndex)
// {
//     if (paramIndex >= 0)
//     {
//         param = paramIndex % kNumParams;
//     }
//     else
//     {
//         param = (paramIndex + kNumParams) % kNumParams;
//     }
//     page = param / NUM_DISP_PARAMS;

//     // if(instLockView_ && page == GRIDS_DENSITY)
//     // {
//     //     int pIndex = param % NUM_DISP_PARAMS;
//     //     if(pIndex > 0){
//     //         lockedInst_ = pIndex - 1;
//     //     }
//     // }
// }

void OmxModeEuclidean::onEncoderChanged(Encoder::Update enc)
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onEncoderChanged(enc);
        return;
    }

    if (midiModeception)
    {
        midiKeyboard.onEncoderChanged(enc);
        return;
    }

    int8_t selPage = selEucParams.getSelPage(); 

    if (encoderSelect_ || selPage == SELEUCLID_PAT)
    {
        onEncoderChangedSelectParam(enc);
        return;
    }

    EuclideanSequencer* activeEuclid = &euclids[selectedEuclid_];

    auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)

    int8_t selParam = selEucParams.getSelParam() + 1; // Add one for readability

    switch (selPage)
    {
    case SELEUCLID_PAT:
    {
    }
    break;
    case SELEUCLID_1:
    {
        if (selParam == 1)
        {
            activeEuclid->setRotation(constrain(activeEuclid->getRotation() + amt, 0, 32));
        }
        else if (selParam == 2)
        {
            activeEuclid->setEvents(constrain(activeEuclid->getEvents() + amt, 0, 32));
        }
        else if (selParam == 3)
        {
            activeEuclid->setSteps(constrain(activeEuclid->getSteps() + amt, 0, 32));
        }
        else if (selParam == 4)
        {
            uint8_t prevLength = activeEuclid->getNoteLength();
            uint8_t newLength = constrain(prevLength + amt, 0, euclidean::kNumEuclidNoteLengths - 1);

            activeEuclid->setNoteLength(newLength);

            if (prevLength != newLength)
            {
                omxDisp.displayMessageTimed(String(euclidean::kEuclidNoteLengths[newLength]), 10);
            }
        }
    }
    break;
    case SELEUCLID_NOTES:
    {
        if (selParam == 1)
        {
            activeEuclid->setNoteNumber(constrain(activeEuclid->getNoteNumber() + amt, 0, 127));
        }
        else if (selParam == 2)
        {
            activeEuclid->setMidiChannel(constrain(activeEuclid->getMidiChannel() + amt, 1, 16));
        }
        else if (selParam == 3)
        {
            activeEuclid->setVelocity(constrain(activeEuclid->getVelocity() + amt, 0, 127));
        }
        else if (selParam == 4)
        {
            activeEuclid->setSwing(constrain(activeEuclid->getSwing() + amt, 0, 100));
        }
    }
    break;
    default:
        break;
    }

    omxLeds.setDirty();
    omxDisp.setDirty();
}

// Handles selecting params using encoder
void OmxModeEuclidean::onEncoderChangedSelectParam(Encoder::Update enc)
{
    if(enc.dir() == 0) return;

    if (enc.dir() < 0) // if turn CCW
    {
        selEucParams.decrementParam();
    }
    else if (enc.dir() > 0) // if turn CW
    {
        selEucParams.incrementParam();
    }

    omxDisp.setDirty();
}

void OmxModeEuclidean::onEncoderButtonDown()
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onEncoderButtonDown();
        return;
    }

    if (midiModeception)
    {
        midiKeyboard.onEncoderButtonDown();
        return;
    }

    int8_t selPage = selEucParams.getSelPage();

    if (selPage == SELEUCLID_PAT)
    {
        encoderSelect_ = true;

        polyRhythmMode = !polyRhythmMode;

        for (u_int8_t i = 0; i < kNumEuclids; i++)
        {
            euclids[i].setPolyRhythmMode(polyRhythmMode);
        }

        if (polyRhythmMode)
        {
            omxDisp.displayMessage("pRhythm on");
        }
        else
        {
            omxDisp.displayMessage("pRhythm off");
        }
    }
    else
    {
        encoderSelect_ = !encoderSelect_;
    }

    omxLeds.setDirty();
    omxDisp.setDirty();
}

void OmxModeEuclidean::onEncoderButtonDownLong()
{
    // if(isSubmodeEnabled()){
    //     activeSubmode->onEncoderButtonDownLong();
    //     return;
    // }
    
    if (midiModeception)
    {
        midiKeyboard.onEncoderButtonDownLong();
        return;
    }
    
    omxLeds.setDirty();
    omxDisp.setDirty();
}

bool OmxModeEuclidean::shouldBlockEncEdit()
{
    if(isSubmodeEnabled()){
        return activeSubmode->shouldBlockEncEdit();
    }

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
    omxLeds.setDirty();

    if (isSubmodeEnabled())
    {
        activeSubmode->onKeyUpdate(e);
        return;
    }

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

    EuclideanSequencer* activeEuclid = &euclids[selectedEuclid_];

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

    if (fNone_)
    {
        // Quick Select Note
        if (e.down() && (thisKey > 10))
        {
            selectEuclid(thisKey - 11);
        }

        if(e.down() && thisKey >= 6 && thisKey < 11)
        {
            activeEuclid->midiFXGroup = thisKey - 6;

            // enableSubmode(&subModeMidiFx[thisKey - 8]);
        }
    }
    omxDisp.setDirty();
}

void OmxModeEuclidean::selectEuclid(uint8_t euclidIndex)
{
    // if(instLockView_ && lockedInst_ == instIndex) return;

    // instLockView_ = true;
    // // justLocked_ = true; // Uncomment to immediately switch to channel view
    // lockedInst_ = instIndex;

    // if (page == GRIDS_DENSITY || page == GRIDS_NOTES)
    // {
    //     setParam(page, lockedInst_ + 1);
    // }

    selectedEuclid_ = euclidIndex;

    // omxDisp.displayMessage((String) "Euclid " + (euclidIndex + 1));
    omxLeds.setDirty();
    omxDisp.setDirty();
}

void OmxModeEuclidean::onKeyHeldUpdate(OMXKeypadEvent e)
{
    // if (isSubmodeEnabled())
    // {
    //     activeSubmode->onKeyHeldUpdate(e);
    //     return;
    // }

    if (midiModeception)
    {
        midiKeyboard.onKeyHeldUpdate(e);
        return;
    }

    int thisKey = e.key();

    // Enter MidiFX mode
    if (thisKey >= 6 && thisKey < 11)
    {
        enableSubmode(&subModeMidiFx[thisKey - 6]);
    }

    omxLeds.setDirty();
    omxDisp.setDirty();
}

void OmxModeEuclidean::updateLEDs()
{
    // Serial.println("Euclidean Leds");

    if (midiModeception)
    {
        return;
    }

    // omxLeds.updateBlinkStates();
    EuclideanSequencer* activeEuclid = &euclids[selectedEuclid_];

    bool blinkState = omxLeds.getBlinkState();

    // turn leds off
    for(uint8_t i = 1; i < 26; i++)
    {
        strip.setPixelColor(0, LEDOFF);
    }

    if (sequencer.playing)
    {
        // Blink left/right keys for octave select indicators.
        auto color1 = blinkState ? LIME : LEDOFF;
        strip.setPixelColor(0, color1);
    }
    else
    {
        strip.setPixelColor(0, LEDOFF);
    }

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

    for (uint8_t i = 0; i < kNumMidiFXGroups; i++)
    {
        auto mfxColor = (i == activeEuclid->midiFXGroup) ? LTCYAN : BLUE;

        strip.setPixelColor(8 + i, mfxColor);
    }

    for (uint8_t i = 0; i < kNumEuclids; i++)
    {
        auto eucColor = (i == selectedEuclid_) ? WHITE : DKRED;
        strip.setPixelColor(11 + i, eucColor);
    }

    strip.setPixelColor(10, BLUE);

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
}

void OmxModeEuclidean::updateLEDsFNone()
{
    // bool blinkState = omxLeds.getBlinkState();

    // auto keyState = midiSettings.keyState;

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

// Called by pending note offs when a pending note off is sent
void OmxModeEuclidean::onPendingNoteOff(int note, int channel)
{
    // Serial.println("OmxModeEuclidean::onPendingNoteOff " + String(note) + " " + String(channel));
    // subModeMidiFx.onPendingNoteOff(note, channel);

    for(uint8_t i = 0; i < kNumMidiFXGroups; i++)
    {
        subModeMidiFx[i].onPendingNoteOff(note, channel);
    }
}

// Called by a euclid sequencer when it triggers a note
void OmxModeEuclidean::onNoteTriggered(uint8_t euclidIndex, MidiNoteGroup note)
{
    // Serial.println("OmxModeEuclidean::onNoteTriggered " + String(euclidIndex) + " note: " + String(note.noteNumber));

    uint8_t mfxIndex = euclids[euclidIndex].midiFXGroup;
    
    subModeMidiFx[mfxIndex].noteInput(note);

    omxDisp.setDirty();
}

// Called by the midiFX group when a note exits it's FX Pedalboard
void OmxModeEuclidean::onNotePostFX(MidiNoteGroup note)
{
    if (note.noteOff)
    {
        // Serial.println("onNotePostFX note off: " + String(note.noteNumber));
        pendingNoteOns.remove(note.noteNumber, note.channel);
        pendingNoteOffs.sendOffNow(note.noteNumber, note.channel, note.sendCV);
    }
    else
    {
        // Serial.println("onNotePostFX note on: " + String(note.noteNumber));

        // Serial.println("OmxModeEuclidean::onNotePostFX note: " + String(note.noteNumber));

        uint32_t noteOnMicros = note.noteonMicros; // TODO Might need to be set to current micros
        pendingNoteOns.insert(note.noteNumber, note.velocity, note.channel, noteOnMicros, note.sendCV);

        uint32_t noteOffMicros = noteOnMicros + (note.stepLength * clockConfig.step_micros);
        pendingNoteOffs.insert(note.noteNumber, note.channel, noteOffMicros, note.sendCV);
    }

    // Serial.println("\n\n");
}

void OmxModeEuclidean::setupPageLegends()
{
    omxDisp.clearLegends();

    int8_t page = selEucParams.getSelPage();

    EuclideanSequencer* activeEuclid = &euclids[selectedEuclid_];

    switch (page)
    {
    case SELEUCLID_1:
    {
        omxDisp.legends[0] = "ROT";
        omxDisp.legends[1] = "EVTS";
        omxDisp.legends[2] = "STEPS";
        omxDisp.legends[3] = "LEN";
        omxDisp.legendVals[0] = activeEuclid->getRotation();
        omxDisp.legendVals[1] = activeEuclid->getEvents();
        omxDisp.legendVals[2] = activeEuclid->getSteps();
        omxDisp.legendVals[3] = activeEuclid->getNoteLength();
    }
    break;
    case SELEUCLID_NOTES:
    {
        omxDisp.legends[0] = "NOTE";
        omxDisp.legends[1] = "CHAN";
        omxDisp.legends[2] = "VEL";
        omxDisp.legends[3] = "SWNG";
        omxDisp.legendVals[0] = activeEuclid->getNoteNumber();
        omxDisp.legendVals[1] = activeEuclid->getMidiChannel();
        omxDisp.legendVals[2] = activeEuclid->getVelocity();
        omxDisp.legendVals[3] = activeEuclid->getSwing();
    }
    break;
    default:
        break;
    }
}

void OmxModeEuclidean::onDisplayUpdate()
{
    if (isSubmodeEnabled())
    {
        activeSubmode->onDisplayUpdate();
        return;
    }

    if (midiModeception)
    {
        midiKeyboard.onDisplayUpdate();

        if (midiSettings.midiAUX)
        {
            strip.setPixelColor(26, RED); // Highlight aux exit key
        }

        return;
    }

    omxLeds.updateBlinkStates();

    if (omxLeds.isDirty())
    {
        updateLEDs();
    }

    if (omxDisp.isDirty())
    { 
        if (!encoderConfig.enc_edit)
        {
            if (selEucParams.getSelPage() == SELEUCLID_PAT)
            {

                if (sequencer.playing)
                {
                    omxDisp.setDirty();
                }

                // for (uint8_t i = 0; i < 4; i++)
                // {
                //     uint8_t ypos = 7 * (i + 1);
                //     bool selected = i == selectedEuclid_;
                //     omxDisp.drawEuclidPattern(euclids[i].getPattern(), euclids[i].getSteps(), ypos, selected, euclids[i].isRunning(), euclids[i].getLastSeqPos());
                // }

                EuclideanSequencer* activeEuclid = &euclids[selectedEuclid_];

                uint8_t ypos = 20;

                omxDisp.drawEuclidPattern(true, activeEuclid->getPattern(), activeEuclid->getSteps(), ypos, false, activeEuclid->isRunning(), activeEuclid->getLastSeqPos());

                omxDisp.dispPageIndicators2(selEucParams.getNumPages(), 0);

                // for(int i = 0; i < 4; i++){

                //     bool selected = i == 0;

                //     omxDisp.dispPageIndicators(i, selected);
                // }

                // int pselected = param % NUM_DISP_PARAMS;
                // setupPageLegends();
                // omxDisp.dispGenericMode(pselected);
            }
            else
            {
                setupPageLegends();
                omxDisp.dispGenericMode2(selEucParams.getNumPages(), selEucParams.getSelPage(), selEucParams.getSelParam(), encoderSelect_);
            }
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

void OmxModeEuclidean::enableSubmode(SubmodeInterface *subMode)
{
    activeSubmode = subMode;
    activeSubmode->setEnabled(true);
    omxDisp.setDirty();
}

void OmxModeEuclidean::disableSubmode()
{
    activeSubmode = nullptr;
    omxDisp.setDirty();
}

bool OmxModeEuclidean::isSubmodeEnabled()
{
    if(activeSubmode == nullptr) return false;

    if(activeSubmode->isEnabled() == false){
        disableSubmode();
        return false;
    }

    return true;
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
