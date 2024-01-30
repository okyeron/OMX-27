#include "omx_mode_euclidean.h"
#include "config.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
// #include "sequencer.h"
#include "euclidean_sequencer.h"
// #include "ClearUI.h"
#include "noteoffs.h"
#include "MM.h"
#include "logic_util.h"

using namespace euclidean;

// enum EucModePage {
//     EUCLID_DENSITY,
//     EUCLID_XY,
//     EUCLID_NOTES,
//     EUCLID_CONFIG
// };

enum ParamModes {
    PARAMMODE_MIX = 0,
    PARAMMODE_EDIT = 1,
    PARAMMODE_PATTERN = 2
};

enum SelEucModePage {
    SELEUCLID_PAT,
    SELEUCLID_1,
    SELEUCLID_NOTES,
    SELEUCLID_CFG1 // PolyRythm, Rate, Global Rate, BPM
};

const int kSelMixColor = WHITE;
const int kMixColor = ORANGE;
const int kMixTrigger = 0xFCD0A4;
const int kMixMuteColor = 0x080808; // 0x1f1001;

const int kSelSaveColor = WHITE;
const int kSaveColor = DKGREEN;

const int kSelEuclidColor = LBLUE;
const int kSelEuclidTriggerColor = AMBER;
const int kSelEuclidMuteColor = DKBLUE;
const int kEuclidColor = DKRED;
const int kEuclidTrigger = AMBER;
const int kEuclidMuteColor = 0x080808; //0x240000;

const int kSelMidiFXColor = LTCYAN;
const int kMidiFXColor = BLUE;

OmxModeEuclidean::OmxModeEuclidean()
{
    midiKeyboard.setMidiMode();

    // Setup function pointers for note ons. 
    for (uint8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].setNoteOutputFunc(&OmxModeEuclidean::onNoteTriggeredForwarder, this, i);
    }

    polyRhythmMode = false;

    for (uint8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].setPolyRhythmMode(polyRhythmMode);
        euclids[i].setClockDivMult(3);
        euclids[i].setPolyRClockDivMult(3);

        initEuclid_.polyRhythmMode_ = polyRhythmMode;
        initEuclid_.polyRClockDivMultP_ = 3;
    }

    paramMode_ = PARAMMODE_EDIT;

    params_[PARAMMODE_MIX].addPage(1);

    params_[PARAMMODE_EDIT].addPage(1);
    params_[PARAMMODE_EDIT].addPage(4);
    params_[PARAMMODE_EDIT].addPage(4);
    params_[PARAMMODE_EDIT].addPage(4);

    params_[PARAMMODE_PATTERN].addPage(1);

    euclids[0].setNoteNumber(36);
    euclids[1].setNoteNumber(38);
    euclids[2].setNoteNumber(42);
    euclids[3].setNoteNumber(46);

    euclids[4].setNoteNumber(60);
    euclids[5].setNoteNumber(64);
    euclids[6].setNoteNumber(67);
    euclids[7].setNoteNumber(71);

    for(uint8_t i = 0; i < kNumSaves; i++)
    {
        saveActivePattern(i, false);
    }

    selectedSave_ = 0;
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

    isPlaying_ = false;

    // sequencer.playing = false;
    // stopSequencers();
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

    paramMode_ = PARAMMODE_EDIT;
    encoderSelect_ = true;

    for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    {
        subModeMidiFx[i].setNoteOutputFunc(&OmxModeEuclidean::onNotePostFXForwarder, this);
        subModeMidiFx[i].setSelected(true);
        subModeMidiFx[i].onModeChanged();
    }

    pendingNoteOffs.setNoteOffFunction(&OmxModeEuclidean::onPendingNoteOffForwarder, this);
}

void OmxModeEuclidean::onModeDeactivated()
{
    isPlaying_ = false;
    // sequencer.playing = false;
    stopSequencers();

    for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    {
        subModeMidiFx[i].setEnabled(false);
        subModeMidiFx[i].setSelected(false);
        subModeMidiFx[i].onModeChanged();
    }
}

void OmxModeEuclidean::selectMidiFx(uint8_t mfxIndex)
{
    // this->mfxIndex = mfxIndex;

    // for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    // {
    //     subModeMidiFx[i].setSelected(i == mfxIndex);
    // }
}

void OmxModeEuclidean::startSequencers()
{
    // pendingStart_ = true;
    isPlaying_ = true;
    omxUtil.startClocks();

    for (u_int8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].start();
    }

    for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    {
        subModeMidiFx[i].setSelected(true);
    }
    // MM::startClock();

    pendingStart_ = false;
}
void OmxModeEuclidean::stopSequencers()
{
    isPlaying_ = false;
    pendingStart_ = false;

    for (u_int8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].stop();
    }
    omxUtil.stopClocks();
	// MM::stopClock();
    pendingNoteOffs.allOff();

    for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    {
        subModeMidiFx[i].resync();
    }
}

void OmxModeEuclidean::onClockTick()
{
    // if (pendingStart_)
    // {
    //     for (u_int8_t i = 0; i < kNumEuclids; i++)
    //     {
    //         euclids[i].start();
    //     }

    //     for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    //     {
    //         subModeMidiFx[i].setSelected(true);
    //     }
    //     MM::startClock();

    //     pendingStart_ = false;
    // }


    for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
    {
        // Lets them do things in background
        subModeMidiFx[i].onClockTick();
    }

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

ParamManager* OmxModeEuclidean::getSelectedParamMode()
{
    return &params_[paramMode_];
}

void OmxModeEuclidean::setParamMode(uint8_t newParamMode)
{
    switch (newParamMode)
    {
    case PARAMMODE_MIX:
    {
        paramMode_ = PARAMMODE_MIX;
        omxDisp.displayMessageTimed("Mix", 5);
        setPageAndParam(0,0, false);
    }
    break;
    case PARAMMODE_EDIT:
    {
        paramMode_ = PARAMMODE_EDIT;
        omxDisp.displayMessageTimed("Edit", 5);
        setPageAndParam(0,0, false);
    }
    break;
    case PARAMMODE_PATTERN:
    {
        paramMode_ = PARAMMODE_PATTERN;
        omxDisp.displayMessageTimed("Pattern", 5);
        setPageAndParam(0,0, false);
    }
    break;
    default:
        break;
    }
}

void OmxModeEuclidean::setPageAndParam(uint8_t pageIndex, uint8_t paramPosition, bool editParam)
{
    encoderSelect_ = !editParam;
    params_[paramMode_].setSelPage(pageIndex);
    // selEucParams.setSelPage(pageIndex);
    setParam(paramPosition);
    omxDisp.setDirty();
}

void OmxModeEuclidean::setParam(uint8_t paramIndex)
{
    params_[paramMode_].setSelParam(paramIndex);

    // selEucParams.setSelParam(paramIndex);

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

    EuclideanSequencer *activeEuclid = &euclids[selectedEuclid_];

    // Serial.println(String("PotChanged ") + String(potIndex));

    // --- EDIT MODE ---
    if (paramMode_ == PARAMMODE_EDIT)
    {
        // Serial.println("Edit Mode");

        if (analogDelta < 3)
            return;


        if (potIndex == 0)
        {
            // Serial.println("Rotation");

            activeEuclid->setRotation(map(newValue, 0, 127, 0, 32));
        }
        if (potIndex == 1)
        {
            // Serial.println("Events");

            activeEuclid->setEvents(map(newValue, 0, 127, 0, 32));
        }
        if (potIndex == 2)
        {
            // Serial.println("Steps");

            activeEuclid->setSteps(map(newValue, 0, 127, 0, 32));
        }
        if (potIndex == 3)
        {
            // Serial.println("length");

            uint8_t prevLength = activeEuclid->getNoteLength();
            uint8_t newLength = map(newValue, 0, 127, 0, kNumNoteLengths - 1);

            activeEuclid->setNoteLength(newLength);

            if (prevLength != newLength)
            {
                tempString = String(kNoteLengths[newLength]);
                omxDisp.displayMessage(tempString.c_str());
            }
        }
        if (potIndex == 4)
        {
            // Serial.println("Clock");

            uint8_t prevRes = activeEuclid->getClockDivMult();
            uint8_t newres = map(newValue, 0, 127, 0, 6);
            if (polyRhythmMode)
            {
                for (u_int8_t i = 0; i < kNumEuclids; i++)
                {
                    euclids[i].setPolyRClockDivMult(newres);
                }
                initEuclid_.polyRClockDivMultP_ = newres;
            }
            else
            {
                activeEuclid->setClockDivMult(newres);
            }

            if (newres != prevRes)
            {
                tempString = String(multValues[newres]);
                omxDisp.displayMessage(tempString.c_str());
            }
        }
    }

    omxLeds.setDirty();
    omxDisp.setDirty();

    // Serial.println((String)"AnalogDelta: " + analogDelta);
}

void OmxModeEuclidean::loopUpdate(Micros elapsedTime)
{
    // if (isSubmodeEnabled())
    // {
    //     activeSubmode->loopUpdate();
    //     // return;
    // }
    
    if (midiModeception)
    {
        midiKeyboard.loopUpdate(elapsedTime);
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

    uint32_t playstepmicros = seqConfig.currentFrameMicros;

    bool clockAdvanced = false;

    for (u_int8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].clockTick(playstepmicros, clockConfig.step_micros);

        if(euclids[i].getClockAdvanced())
        {
            clockAdvanced = true;
        }
    }

    if(clockAdvanced)
    {
        omxDisp.setDirty();
        omxLeds.setDirty();
    }

    for(uint8_t i = 0; i < 5; i++)
    {
        // Lets them do things in background
        subModeMidiFx[i].loopUpdate();
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

    // --- EDIT MODE ---
    if (paramMode_ == PARAMMODE_EDIT)
    {
        int8_t selPage = getSelectedParamMode()->getSelPage();

        if (encoderSelect_ || selPage == SELEUCLID_PAT)
        {
            onEncoderChangedSelectParam(enc);
            return;
        }

        EuclideanSequencer *activeEuclid = &euclids[selectedEuclid_];

        auto amtSlow = enc.accel(1);
        auto amtFast = enc.accel(5);

        int8_t selParam = getSelectedParamMode()->getSelParam() + 1; // Add one for readability

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
                activeEuclid->setRotation(constrain(activeEuclid->getRotation() + amtSlow, 0, 32));
            }
            else if (selParam == 2)
            {
                activeEuclid->setEvents(constrain(activeEuclid->getEvents() + amtSlow, 0, 32));
            }
            else if (selParam == 3)
            {
                activeEuclid->setSteps(constrain(activeEuclid->getSteps() + amtSlow, 0, 32));
            }
            else if (selParam == 4)
            {
                uint8_t prevLength = activeEuclid->getNoteLength();
                uint8_t newLength = constrain(prevLength + amtSlow, 0, kNumNoteLengths - 1);

                activeEuclid->setNoteLength(newLength);

                if (prevLength != newLength)
                {
                    omxDisp.displayMessageTimed(String(kNoteLengths[newLength]), 10);
                }
            }
        }
        break;
        case SELEUCLID_NOTES:
        {
            if (selParam == 1)
            {
                activeEuclid->setNoteNumber(constrain(activeEuclid->getNoteNumber() + amtFast, 0, 127));
            }
            else if (selParam == 2)
            {
                activeEuclid->setMidiChannel(constrain(activeEuclid->getMidiChannel() + amtSlow, 1, 16));
            }
            else if (selParam == 3)
            {
                activeEuclid->setVelocity(constrain(activeEuclid->getVelocity() + amtFast, 0, 127));
            }
            else if (selParam == 4)
            {
                activeEuclid->setSwing(constrain(activeEuclid->getSwing() + amtFast, 0, 100));
            }
        }
        break;
        case SELEUCLID_CFG1:
        {
            if (selParam == 1)
            {
                bool prevVal = polyRhythmMode;

                polyRhythmMode = (bool)constrain(polyRhythmMode + amtSlow, 0, 1);

                if (prevVal != polyRhythmMode)
                {
                    for (u_int8_t i = 0; i < kNumEuclids; i++)
                    {
                        euclids[i].setPolyRhythmMode(polyRhythmMode);
                    }

                    initEuclid_.polyRhythmMode_ = polyRhythmMode;

                    if (polyRhythmMode)
                    {
                        omxDisp.displayMessage("PolyRhythm");
                    }
                    else
                    {
                        omxDisp.displayMessage("PolyMeter");
                    }
                }
            }
            else if (selParam == 2) // Track Mult
            {
                uint8_t prevRes = activeEuclid->getClockDivMult();
                uint8_t newres = constrain(prevRes + amtSlow, 0, 6);

                if(prevRes != newres)
                {
                    activeEuclid->setClockDivMult(newres);

                    tempString = String(multValues[newres]);
                    omxDisp.displayMessage(tempString.c_str());
                }
            }
            else if (selParam == 3) // Global polyRhythm Mult
            {
                uint8_t prevRes = euclids[0].getPolyRClockDivMult();
                uint8_t newres = constrain(prevRes + amtSlow, 0, 6);

                if(prevRes != newres)
                {
                    for (u_int8_t i = 0; i < kNumEuclids; i++)
                    {
                        euclids[i].setPolyRClockDivMult(newres);
                    }

                    tempString = String(multValues[newres]);
                    omxDisp.displayMessage(tempString.c_str());
                }
            }
            else if (selParam == 4) // BPM
            {
                clockConfig.newtempo = constrain(clockConfig.clockbpm + amtFast, 40, 300);
                if (clockConfig.newtempo != clockConfig.clockbpm)
                {
                    // SET TEMPO HERE
                    clockConfig.clockbpm = clockConfig.newtempo;
                    omxUtil.resetClocks();
                }
            }
        }
        break;
        default:
            break;
        }
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
        getSelectedParamMode()->decrementParam();
    }
    else if (enc.dir() > 0) // if turn CW
    {
        getSelectedParamMode()->incrementParam();
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

    int8_t selPage = getSelectedParamMode()->getSelPage();

    // --- EDIT MODE ---
    if (paramMode_ == PARAMMODE_EDIT)
    {
        if (selPage == SELEUCLID_PAT)
        {
            encoderSelect_ = true;

            // polyRhythmMode = !polyRhythmMode;

            // for (u_int8_t i = 0; i < kNumEuclids; i++)
            // {
            //     euclids[i].setPolyRhythmMode(polyRhythmMode);
            // }

            // if (polyRhythmMode)
            // {
            //     omxDisp.displayMessage("PolyRhythm");
            // }
            // else
            // {
            //     omxDisp.displayMessage("PolyMeter");
            // }
        }
        else
        {
            encoderSelect_ = !encoderSelect_;
        }
    }
    else
    {
        if (selPage == SELEUCLID_PAT)
        {
        }
        else
        {
            encoderSelect_ = !encoderSelect_;
        }
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

void OmxModeEuclidean::saveActivePattern(uint8_t pattIndex, bool showMsg)
{
    for(uint8_t i = 0; i < kNumEuclids; i++)
    {
        saveSlots_[pattIndex].euclids[i] = euclids[i].getSave();
    }

    saveSlots_[pattIndex].polyRhythmMode_ = polyRhythmMode;
    selectedSave_ = pattIndex;

    if (showMsg)
    {
        omxDisp.displayMessageTimed("Saved " + String(pattIndex + 1), 5);
    }
}

void OmxModeEuclidean::loadActivePattern(uint8_t pattIndex)
{
    for(uint8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].loadSave(saveSlots_[pattIndex].euclids[i]);
    }

    polyRhythmMode = saveSlots_[pattIndex].polyRhythmMode_;
    selectedSave_ = pattIndex;

    omxDisp.displayMessageTimed("Load " + String(pattIndex + 1), 5);
}

void OmxModeEuclidean::onKeyUpdate(OMXKeypadEvent e)
{
    omxLeds.setDirty();

    if (isSubmodeEnabled())
    {
        if(activeSubmode->onKeyUpdate(e)) return;
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
            if (isPlaying_ && aux_)
            {
                aux_ = false;
                stopSequencers();
                // sequencer.playing = false;
            }
            else
            {
                aux_ = true;
                startSequencers();
                // sequencer.playing = true;
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

    if(e.down() && thisKey == 3)
    {
        setParamMode(PARAMMODE_MIX);
    }
    else if(e.down() && thisKey == 4)
    {
        setParamMode(PARAMMODE_EDIT);
    }
    else if(e.down() && thisKey == 5)
    {
        setParamMode(PARAMMODE_PATTERN);
    }

    // --- EDIT MODE ---
    if (paramMode_ == PARAMMODE_EDIT || paramMode_ == PARAMMODE_MIX)
    {
        if (fNone_)
        {
            if (e.down() && (thisKey > 10) && thisKey < 19)
            {
                selectEuclid(thisKey - 11);

                if(paramMode_ == PARAMMODE_MIX)
                {
                    toggleMute(thisKey - 11);
                }

                copiedEuclid_ = euclids[thisKey - 11].getSave();
            }

            if (e.down() && thisKey >= 6 && thisKey < 11)
            {
                activeEuclid->midiFXGroup = thisKey - 6;
                // enableSubmode(&subModeMidiFx[thisKey - 8]);
            }

            if (!e.down() && e.clicks() == 2 && thisKey >= 6 && thisKey < 11)
            {
                enableSubmode(&subModeMidiFx[thisKey - 6]);
            }
        }
        else if(f1_) // Mute
        {
            if (e.down() && (thisKey > 10) && thisKey < 19)
            {
                toggleMute(thisKey - 11);
            }
        }
        else if(f2_) // Paste
        {
            if (e.down() && (thisKey > 10) && thisKey < 19)
            {
                euclids[thisKey - 11].loadSave(copiedEuclid_);
                omxDisp.displayMessageTimed("Paste: " + String(thisKey - 11 + 1), 5);
            }
        }
        else if(f3_) // Cut
        {
            if (e.down() && (thisKey > 10) && thisKey < 19)
            {
                selectEuclid(thisKey - 11);
                copiedEuclid_ = euclids[thisKey - 11].getSave();
                euclids[thisKey -11].loadSave(initEuclid_);
                omxDisp.displayMessageTimed("Cut: " + String(thisKey - 11 + 1), 5);
            }
        }
    }
    // --- PATTERN MODE ---
    else if(paramMode_ == PARAMMODE_PATTERN)
    {
        if(f2_)
        {
            if(e.down() && e.clicks() == 0 && thisKey > 10)
            {
                uint8_t patt = thisKey - 11;

                saveActivePattern(patt);
            }
        }
        else
        {
            if(e.down() && e.clicks() == 0 && thisKey > 10)
            {
                uint8_t patt = thisKey - 11;

                loadActivePattern(patt);
            }
        }
    }
    omxDisp.setDirty();
}

void OmxModeEuclidean::toggleMute(uint8_t euclidIndex)
{
    bool muted = !euclids[euclidIndex].getMute();
    euclids[euclidIndex].setMute(muted);

    omxDisp.displayMessageTimed(String(euclidIndex + 1) + (muted ? " Muted" : " Unmuted"), 5);

    omxLeds.setDirty();
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
    if (isSubmodeEnabled())
    {
        if(activeSubmode->onKeyHeldUpdate(e)) return;
    }

    if (midiModeception)
    {
        midiKeyboard.onKeyHeldUpdate(e);
        return;
    }

    int thisKey = e.key();

    // --- EDIT MODE ---
    if (paramMode_ == PARAMMODE_EDIT || paramMode_ == PARAMMODE_MIX)
    {
        // Enter MidiFX mode
        if (thisKey >= 6 && thisKey < 11)
        {
            enableSubmode(&subModeMidiFx[thisKey - 6]);
        }
    }

    omxLeds.setDirty();
    omxDisp.setDirty();
}

void OmxModeEuclidean::updateLEDs()
{
    if (isSubmodeEnabled())
    {
        if(activeSubmode->updateLEDs()) return;
    }

    // Serial.println("Euclidean Leds");

    if (midiModeception)
    {
        return;
    }

    // omxLeds.updateBlinkStates();
    EuclideanSequencer* activeEuclid = &euclids[selectedEuclid_];

    bool blinkState = omxLeds.getBlinkState();

    // turn leds off
    for(uint8_t i = 1; i < 27; i++)
    {
        strip.setPixelColor(i, LEDOFF);
    }

    if (isPlaying_)
    {
        // Blink left/right keys for octave select indicators.
        auto color1 = blinkState ? LIME : LEDOFF;
        strip.setPixelColor(0, color1);
    }
    else
    {
        strip.setPixelColor(0, LEDOFF);
    }

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

    strip.setPixelColor(3, paramMode_ == PARAMMODE_MIX ? WHITE : kMixColor);
    strip.setPixelColor(4, paramMode_ == PARAMMODE_EDIT ? WHITE : kEuclidColor);
    strip.setPixelColor(5, paramMode_ == PARAMMODE_PATTERN ? WHITE : kSaveColor);

    // --- EDIT MODE ---
    if(paramMode_ == PARAMMODE_MIX)
    {
        for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
        {
            auto mfxColor = (i == activeEuclid->midiFXGroup) ? kSelMidiFXColor : kMidiFXColor;

            strip.setPixelColor(6 + i, mfxColor);
        }

        for (uint8_t i = 0; i < kNumEuclids; i++)
        {
            auto eucColor = euclids[i].getMute() ? kMixMuteColor : kMixColor;
            if(isPlaying_)
            {
                eucColor = euclids[i].getTriggered() ? kMixTrigger : eucColor;
            }
            strip.setPixelColor(11 + i, eucColor);
        }
    }
    else if (paramMode_ == PARAMMODE_EDIT)
    {
        for (uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
        {
            auto mfxColor = (i == activeEuclid->midiFXGroup) ? kSelMidiFXColor : kMidiFXColor;

            strip.setPixelColor(6 + i, mfxColor);
        }

        for (uint8_t i = 0; i < kNumEuclids; i++)
        {
            auto eucColor = euclids[i].getMute() ? kEuclidMuteColor : kEuclidColor;
            if(isPlaying_)
            {
                eucColor = euclids[i].getTriggered() ? kEuclidTrigger : eucColor;
            }
            if(i == selectedEuclid_)
            {
                eucColor = euclids[i].getMute() ? kSelEuclidMuteColor : kSelEuclidColor;
                eucColor = euclids[i].getTriggered() ? kSelEuclidTriggerColor : eucColor;
            }
            strip.setPixelColor(11 + i, eucColor);
        }
    }
    else if(paramMode_ == PARAMMODE_PATTERN)
    {
        for (uint8_t i = 0; i < kNumSaves; i++)
        {
            auto saveColor = (i == selectedSave_) ? kSelSaveColor : kSaveColor;
            strip.setPixelColor(11 + i, saveColor);
        }
    }

    if (isSubmodeEnabled())
    {
        bool blinkStateSlow = omxLeds.getSlowBlinkState();

        auto auxColor = (blinkStateSlow ? RED : LEDOFF);
        strip.setPixelColor(0, auxColor);
    }
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

    for(uint8_t i = 0; i < NUM_MIDIFX_GROUPS; i++)
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
    omxLeds.setDirty();
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


        // MM::sendNoteOn(note.noteNumber, note.velocity, note.channel);

        // uint32_t noteOnMicros = seqConfig.currentFrameMicros; // TODO Might need to be set to current micros

        uint32_t noteOffMicros = noteOnMicros + (note.stepLength * clockConfig.step_micros);
        pendingNoteOffs.insert(note.noteNumber, note.channel, noteOffMicros, note.sendCV);
    }

    // Serial.println("\n\n");
}

void OmxModeEuclidean::setupPageLegends()
{
    omxDisp.clearLegends();

    int8_t page = getSelectedParamMode()->getSelPage();

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
    case SELEUCLID_CFG1:
    {
        omxDisp.legends[0] = "MODE";
        omxDisp.legends[1] = "TRAT";
        omxDisp.legends[2] = "PRAT";
        omxDisp.legends[3] = "BPM";
        omxDisp.legendVals[0] = (int)polyRhythmMode;
        omxDisp.useLegendString[1] = true;
        omxDisp.legendString[1] = String(activeEuclid->getClockDivMult());
        omxDisp.useLegendString[2] = true;
        omxDisp.legendString[2] = String(euclids[0].getPolyRClockDivMult());
        omxDisp.legendVals[3] = (int)clockConfig.clockbpm;
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
        if (omxLeds.isDirty())
        {
            updateLEDs();
        }

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

    // omxLeds.updateBlinkStates();

    if (omxLeds.isDirty())
    {
        updateLEDs();
    }

    if (omxDisp.isDirty())
    { 
        if (!encoderConfig.enc_edit)
        {
            auto params = getSelectedParamMode();

            if (!fNone_ && (paramMode_ == PARAMMODE_EDIT || paramMode_ == PARAMMODE_MIX))
            {
                if(f1_)
                {
                    omxDisp.dispGenericModeLabel("Mute", params->getNumPages(), params->getSelPage());
                }
                else if(f2_)
                {
                    omxDisp.dispGenericModeLabel("Paste", params->getNumPages(), params->getSelPage());
                }
                else if(f3_)
                {
                    omxDisp.dispGenericModeLabel("Cut", params->getNumPages(), params->getSelPage());
                }
            }
            else if(paramMode_ == PARAMMODE_PATTERN)
            {
                if(f2_)
                {
                    omxDisp.dispGenericModeLabel("Save To", 0,0);
                }
                else
                {
                    omxDisp.dispGenericModeLabel("Load From", 0,0);
                }
            }
            else
            {
                if (params->getSelPage() == SELEUCLID_PAT)
                {
                    // if (isPlaying_)
                    // {
                    //     omxDisp.setDirty();
                    // }

                    // for (uint8_t i = 0; i < 4; i++)
                    // {
                    //     uint8_t ypos = 7 * (i + 1);
                    //     bool selected = i == selectedEuclid_;
                    //     omxDisp.drawEuclidPattern(euclids[i].getPattern(), euclids[i].getSteps(), ypos, selected, euclids[i].isRunning(), euclids[i].getLastSeqPos());
                    // }

                    EuclideanSequencer *activeEuclid = &euclids[selectedEuclid_];

                    uint8_t ypos = 20;

                    omxDisp.drawEuclidPattern(true, activeEuclid->getPattern(), activeEuclid->getSteps(), ypos, false, activeEuclid->isRunning(), activeEuclid->getLastSeqPos());

                    omxDisp.dispPageIndicators2(params->getNumPages(), 0);

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
                    omxDisp.dispGenericMode2(params->getNumPages(), params->getSelPage(), params->getSelParam(), encoderSelect_);
                }
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

int OmxModeEuclidean::saveToDisk(int startingAddress, Storage *storage)
{
    storage->write(startingAddress, selectedSave_);
    startingAddress++;

    int saveSize = sizeof(EuclidPatternSave);

    for(uint8_t i = 0; i < kNumSaves; i++)
    {
        auto saveBytesPtr = (byte *)(&saveSlots_[i]);
		for (int j = 0; j < saveSize; j++)
		{
			storage->write(startingAddress + j, *saveBytesPtr++);
		}

		startingAddress += saveSize;
    }

    return startingAddress;
}

int OmxModeEuclidean::loadFromDisk(int startingAddress, Storage *storage)
{
    selectedSave_ = storage->read(startingAddress);
    startingAddress++;

    int saveSize = sizeof(EuclidPatternSave);

	for (uint8_t i = 0; i < kNumSaves; i++)
	{
		auto pattern = EuclidPatternSave{};
		auto current = (byte *)&pattern;
		for (int j = 0; j < saveSize; j++)
		{
			*current = storage->read(startingAddress + j);
			current++;
		}

        saveSlots_[i] = pattern;
		startingAddress += saveSize;
	}

    // Load selected save to active
    for(uint8_t i = 0; i < kNumEuclids; i++)
    {
        euclids[i].loadSave(saveSlots_[selectedSave_].euclids[i]);
    }

    polyRhythmMode = saveSlots_[selectedSave_].polyRhythmMode_;

    return startingAddress;
}