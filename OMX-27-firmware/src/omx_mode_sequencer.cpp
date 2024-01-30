#include "omx_mode_sequencer.h"
#include "config.h"
#include "colors.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "sequencer.h"
#include "omx_leds.h"

enum SequencerMode
{
    SEQMODE_MAIN,
    SEQMODE_NOTESEL,
    SEQMODE_PAT,
    SEQMODE_STEPRECORD
};

StepNote stepCopyBuffer_;
// String tempString_;

OmxModeSequencer::OmxModeSequencer() {
    // seq params
    seqParams.addPage(4);
    seqParams.addPage(4);

    // note select params
    noteSelParams.addPage(4);
    noteSelParams.addPage(4);
    noteSelParams.addPage(4);

    // pattern params
    patParams.addPage(4);
    patParams.addPage(4);
    patParams.addPage(4);

    // step record params
    sRecParams.addPage(4);
    sRecParams.addPage(4);
}

void OmxModeSequencer::InitSetup()
{
    initSetup = true;
}

void OmxModeSequencer::onModeActivated()
{
    if(!initSetup){
        InitSetup();
    }

    changeSequencerMode(SEQMODE_MAIN);
}

uint8_t OmxModeSequencer::getAdjustedNote(uint8_t keyNumber)
{
    uint8_t adjnote = notes[keyNumber] + (midiSettings.octave * 12);
    return adjnote;
}

// Set state defaults when changing modes
// Helps keep things from getting in weird states and makes code more readable
void OmxModeSequencer::changeSequencerMode(uint8_t newMode)
{
    // Serial.println((String)"changeSequencerMode: " + String((SequencerMode)newMode));
    noteSelect_ = false;
    // noteSelection_ = false;
    // stepSelect_ = false;

    stepRecord_ = false;
    patternParams_ = false;

    switch (newMode)
    {
    case SEQMODE_MAIN:
    {
        seqParams.setSelPageAndParam(0, 0);
        encoderSelect_ = true;
    }
    break;
    case SEQMODE_NOTESEL:
    {
        noteSelect_ = true;
        // stepSelect_ = true;
        // noteSelection_ = true;
        noteSelParams.setSelPageAndParam(0, 0);
        encoderSelect_ = false;
        omxDisp.displayMessagef("NOTE SELECT");
    }
    break;
    case SEQMODE_PAT:
    {
        patternParams_ = true;
        patParams.setSelPageAndParam(0, 1);
        encoderSelect_ = false;
        omxDisp.displayMessagef("PATT PARAMS");
    }
    break;
    case SEQMODE_STEPRECORD:
    {
        stepRecord_ = true;
        sRecParams.setSelPageAndParam(0, 1);
        encoderSelect_ = false;
        omxDisp.displayMessagef("STEP RECORD");
    }
    break;
    default:
        break;
    }

    omxDisp.setDirty();
    omxLeds.setDirty();
}

uint8_t OmxModeSequencer::getSequencerMode()
{
    if(noteSelect_){
        return SEQMODE_NOTESEL;
    }
    else if(patternParams_){
        return SEQMODE_PAT;
    }
    else if(stepRecord_){
        return SEQMODE_STEPRECORD;
    }
    
    return SEQMODE_MAIN;
}

void OmxModeSequencer::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
{
    uint8_t seqMode = getSequencerMode();

    // note selection - do P-Locks
    if (seqMode == SEQMODE_NOTESEL)
    { 
        potSettings.potNum = potIndex;
        potSettings.potCC = pots[potSettings.potbank][potIndex];
        potSettings.potVal = potSettings.analogValues[potIndex];

        if (potIndex < 4)
        { // only store p-lock value for first 4 knobs
            getSelectedStep()->params[potIndex] = potSettings.analogValues[potIndex];
            omxUtil.sendPots(potIndex, sequencer.getPatternChannel(sequencer.playingPattern));
        }
        omxUtil.sendPots(potIndex, sequencer.getPatternChannel(sequencer.playingPattern));
        omxDisp.setDirty();
    }
    else if (seqMode == SEQMODE_STEPRECORD)
    {
        potSettings.potNum = potIndex;
        potSettings.potCC = pots[potSettings.potbank][potIndex];
        potSettings.potVal = potSettings.analogValues[potIndex];

        if (potIndex < 4)
        { // only store p-lock value for first 4 knobs
            sequencer.getCurrentPattern()->steps[sequencer.seqPos[sequencer.playingPattern]].params[potIndex] = potSettings.analogValues[potIndex];
            omxUtil.sendPots(potIndex, sequencer.getPatternChannel(sequencer.playingPattern));
        }
        else if (potIndex == 4)
        {
            sequencer.getCurrentPattern()->steps[sequencer.seqPos[sequencer.playingPattern]].vel = potSettings.analogValues[potIndex]; // SET POT 5 to NOTE VELOCITY HERE
        }
        omxDisp.setDirty();
    }
    else if (seqMode == SEQMODE_MAIN || seqMode == SEQMODE_PAT)
    {
        omxUtil.sendPots(potIndex, sequencer.getPatternChannel(sequencer.playingPattern));
    }
}

void OmxModeSequencer::loopUpdate(Micros elapsedTime)
{
    if (!seq2Mode) // S1
    {
        doStepS1();
    }
    else // S2
    { 
        doStepS2();
    }

    // renders leds for the playing pattern
    updateLEDs();
}

// Handles selecting params using encoder
void OmxModeSequencer::onEncoderChangedSelectParam(Encoder::Update enc)
{
    if(enc.dir() == 0) return;

    uint8_t seqMode = getSequencerMode();

    if (seqMode == SEQMODE_MAIN)
    {
        seqParams.changeParam(enc.dir());
    }
    else if (seqMode == SEQMODE_NOTESEL)
    {
        noteSelParams.changeParam(enc.dir());
    }
    else if (seqMode == SEQMODE_PAT)
    {
        patParams.changeParam(enc.dir());
    }
    else if (seqMode == SEQMODE_STEPRECORD)
    {
        sRecParams.changeParam(enc.dir());
    }

    omxDisp.setDirty();
}

void OmxModeSequencer::onEncoderChanged(Encoder::Update enc)
{
    if (encoderSelect_)
    {
        onEncoderChangedSelectParam(enc);
    }
    else
    {
        if (getSequencerMode() == SEQMODE_MAIN)
        {
            onEncoderChangedNorm(enc);
        }
        else
        {
            onEncoderChangedStep(enc);
        }
    }
}

void OmxModeSequencer::onEncoderChangedNorm(Encoder::Update enc)
{
    auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)

    int8_t selPage = seqParams.getSelPage() + 1; // Add one for readability
    int8_t selParam = seqParams.getSelParam() + 1;

    // PAGE ONE
    if (selPage == 1)
    {
        if (selParam == 1) // CHANGE PATTERN
        {
            sequencer.playingPattern = constrain(sequencer.playingPattern + amt, 0, 7);
            if (sequencer.getCurrentPattern()->solo)
            {
                omxLeds.setAllLEDS(0, 0, 0);
            }
        }
        else if (selParam == 2) // SET TRANSPOSE
        {                                                
            transposeSeq(sequencer.playingPattern, amt); //
            int newtransp = constrain(midiSettings.transpose + amt, -64, 63);
            midiSettings.transpose = newtransp;
        }
        else if (selParam == 3) // SET SWING
        {                                                                                                       
            int newswing = constrain(sequencer.getCurrentPattern()->swing + amt, 0, midiSettings.maxswing - 1); // -1 to deal with display values
            midiSettings.swing = newswing;
            sequencer.getCurrentPattern()->swing = newswing;
            //	setGlobalSwing(newswing);
        }
        else if (selParam == 4) // SET TEMPO
        { 
            clockConfig.newtempo = constrain(clockConfig.clockbpm + amt, 40, 300);
            if (clockConfig.newtempo != clockConfig.clockbpm)
            {
                // SET TEMPO HERE
                clockConfig.clockbpm = clockConfig.newtempo;
                omxUtil.resetClocks();
            }
        }
    }
    // PAGE TWO
    else if (selPage == 2)
    {
        if (selParam == 1) //  MIDI SOLO
        { 
            //						playingPattern = constrain(playingPattern + amt, 0, 7);
            sequencer.getCurrentPattern()->solo = constrain(sequencer.getCurrentPattern()->solo + amt, 0, 1);
            if (sequencer.getCurrentPattern()->solo)
            {
                omxLeds.setAllLEDS(0, 0, 0);
            }
        }
        else if (selParam == 2) // SET PATTERN LENGTH
        { 
            auto newPatternLen = constrain(sequencer.getPatternLength(sequencer.playingPattern) + amt, 1, NUM_STEPS);
            sequencer.setPatternLength(sequencer.playingPattern, newPatternLen);
            if (sequencer.seqPos[sequencer.playingPattern] >= newPatternLen)
            {
                sequencer.seqPos[sequencer.playingPattern] = newPatternLen - 1;
                sequencer.patternPage[sequencer.playingPattern] = getPatternPage(sequencer.seqPos[sequencer.playingPattern]);
            }
        }
        else if (selParam == 3) // SET CLOCK DIV/MULT
        { 
            sequencer.getCurrentPattern()->clockDivMultP = constrain(sequencer.getCurrentPattern()->clockDivMultP + amt, 0, NUM_MULTDIVS - 1);
        }
        else if (selParam == 4) // SET CV ON/OFF
        { 
            sequencer.getCurrentPattern()->sendCV = constrain(sequencer.getCurrentPattern()->sendCV + amt, 0, 1);
        }
    }
    omxDisp.setDirty();
}

// TODO: break this into separate functions
void OmxModeSequencer::onEncoderChangedStep(Encoder::Update enc)
{
    auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)
    auto amtSlow = enc.accel(1); 

    uint8_t seqMode = getSequencerMode();

    // SEQUENCE PATTERN PARAMS SUB MODE
    if (seqMode == SEQMODE_PAT)
    { 
        int8_t selPage = patParams.getSelPage() + 1; // Add one for readability
        int8_t selParam = patParams.getSelParam() + 1;

        // PAGE ONE
        if (selPage == 1)
        {
            if (selParam == 1) // SET PLAYING PATTERN
            { 
                sequencer.playingPattern = constrain(sequencer.playingPattern + amt, 0, 7);
            }
            if (selParam == 2) // SET LENGTH
            { 
                auto newPatternLen = constrain(sequencer.getPatternLength(sequencer.playingPattern) + amt, 1, NUM_STEPS);
                sequencer.setPatternLength(sequencer.playingPattern, newPatternLen);
                if (sequencer.seqPos[sequencer.playingPattern] >= newPatternLen)
                {
                    sequencer.seqPos[sequencer.playingPattern] = newPatternLen - 1;
                    sequencer.patternPage[sequencer.playingPattern] = getPatternPage(sequencer.seqPos[sequencer.playingPattern]);
                }
            }
            if (selParam == 3) // SET PATTERN ROTATION
            { 
                int rotator;
                (enc.dir() < 0 ? rotator = -1 : rotator = 1);
                //							int rotator = constrain(rotcc, (sequencer.PatternLength(sequencer.playingPattern))*-1, sequencer.PatternLength(sequencer.playingPattern));
                midiSettings.rotationAmt = midiSettings.rotationAmt + rotator;
                if (midiSettings.rotationAmt < 16 && midiSettings.rotationAmt > -16)
                { // NUM_STEPS??
                    rotatePattern(sequencer.playingPattern, rotator);
                }
                midiSettings.rotationAmt = constrain(midiSettings.rotationAmt, (sequencer.getPatternLength(sequencer.playingPattern) - 1) * -1, sequencer.getPatternLength(sequencer.playingPattern) - 1);
            }
            if (selParam == 4) // SET PATTERN CHANNEL
            { 
                sequencer.getCurrentPattern()->channel = constrain(sequencer.getCurrentPattern()->channel + amt, 0, 15);
            }
        }
        // PATTERN PARAMS PAGE 2
        else if (selPage == 2)
        {
            if (selParam == 1) // SET AUTO START STEP
            {
                sequencer.getCurrentPattern()->startstep = constrain(sequencer.getCurrentPattern()->startstep + amt, 0, sequencer.getCurrentPattern()->len);
                // sequencer.getCurrentPattern()->startstep--;
            }
            if (selParam == 2) // SET AUTO RESET STEP
            {
                int tempresetstep = sequencer.getCurrentPattern()->autoresetstep + amt;
                sequencer.getCurrentPattern()->autoresetstep = constrain(tempresetstep, 0, sequencer.getCurrentPattern()->len + 1);
            }
            if (selParam == 3) // SET AUTO RESET FREQUENCY
            {
                sequencer.getCurrentPattern()->autoresetfreq = constrain(sequencer.getCurrentPattern()->autoresetfreq + amt, 0, 15); // max every 16 times
            }
            if (selParam == 4) // SET AUTO RESET PROB
            {
                sequencer.getCurrentPattern()->autoresetprob = constrain(sequencer.getCurrentPattern()->autoresetprob + amt, 0, 100); // never, 100% - 33%
            }
        }
        // PAGE THREE
        else if (selPage == 3)
        {
            if (selParam == 1) // SET CLOCK-DIV-MULT
            {                                                                                                                                      
                sequencer.getCurrentPattern()->clockDivMultP = constrain(sequencer.getCurrentPattern()->clockDivMultP + amt, 0, NUM_MULTDIVS - 1); // set clock div/mult
            }
            if (selParam == 2) // SET MIDI SOLO
            { 
                sequencer.getCurrentPattern()->solo = constrain(sequencer.getCurrentPattern()->solo + amt, 0, 1);
            }
        }
    }
    // STEP RECORD SUB MODE
    else if (seqMode == SEQMODE_STEPRECORD)
    {
        int8_t selPage = sRecParams.getSelPage() + 1; // Add one for readability
        int8_t selParam = sRecParams.getSelParam() + 1;

        // PAGE ONE
        if (selPage == 1)
        {
            if (selParam == 1) // OCTAVE SELECTION
            {
                midiSettings.octave = constrain(midiSettings.octave + amt, -5, 4);
            }
            if (selParam == 2) // STEP SELECTION
            {
                if (enc.dir() > 0)
                {
                    step_ahead();
                }
                else if (enc.dir() < 0)
                {
                    step_back();
                }
                seqConfig.selectedStep = sequencer.seqPos[sequencer.playingPattern];
            }
            if (selParam == 3) // SET NOTE NUM
            {
                int tempNote = getSelectedStep()->note;
                getSelectedStep()->note = constrain(tempNote + amt, 0, 127);
            }
            if (selParam == 4) // Pattern
            {
                // playingPattern = constrain(playingPattern + amt, 0, 7);
            }
        }
        // PAGE TWO
        else if (selPage == 2)
        {
            if (selParam == 1) // STEP TYPE
            {
                changeStepType(amt);
            }
            if (selParam == 2) // STEP PROB
            {
                int tempProb = getSelectedStep()->prob;
                getSelectedStep()->prob = constrain(tempProb + amt, 0, 100); // Note Len between 1-16
            }
            if (selParam == 3) // STEP CONDITION
            {
                int tempCondition = getSelectedStep()->condition;
                getSelectedStep()->condition = constrain(tempCondition + amt, 0, 35); // 0-32
            }
        }
    }
    // NOTE SELECT MODE
    else if (seqMode == SEQMODE_NOTESEL)
    {
        int8_t selPage = noteSelParams.getSelPage() + 1; // Add one for readability
        int8_t selParam = noteSelParams.getSelParam() + 1; 

        // PAGE ONE
        if (selPage == 1)
        {
            if (selParam == 1) // SET NOTE NUM
            {
                int tempNote = getSelectedStep()->note;
                getSelectedStep()->note = constrain(tempNote + amt, 0, 127);
            }
            if (selParam == 2) // SET OCTAVE
            {
                midiSettings.octave = constrain(midiSettings.octave + amt, -5, 4);
            }
            if (selParam == 3) // SET VELOCITY
            {
                int tempVel = getSelectedStep()->vel;
                getSelectedStep()->vel = constrain(tempVel + amt, 0, 127);
            }
            if (selParam == 4) // SET NOTE LENGTH
            {
                auto step = getSelectedStep();

                step->len = constrain(step->len + amtSlow, 0, kNumNoteLengths - 1); // Note Len between 1-16

                // int tempLen = step->len;
                // // int newLen = tempLen + amtSlow;
                // auto newLen = constrain(step->len + amtSlow, 0, kNumNoteLengths - 1); // Note Len between 1-16
                // step->len = (uint8_t)newLen;                                          // Note Len between 1-16

                // Serial.println("amtSlow = " + String(amtSlow));
                // Serial.println("tempLen = " + String(tempLen));
                // Serial.println("newLen = " + String(newLen));
                // Serial.println("len = " + String(step->len));
                // Serial.println("NumNoteLengths = " + String(kNumNoteLengths));
                // Serial.println("NoteLength = " + String(kNoteLengths[step->len]));
            }
        }
        // PAGE TWO
        else if (selPage == 2)
        {
            if (noteSelParams.getSelParam() == 0) // SET STEP TYPE
            { 
                changeStepType(amt);
            }
            if (noteSelParams.getSelParam() == 1) // SET STEP PROB
            { 
                int tempProb = getSelectedStep()->prob;
                getSelectedStep()->prob = constrain(tempProb + amt, 0, 100); // Note Len between 1-16
            }
            if (noteSelParams.getSelParam() == 2) // SET STEP TRIG CONDITION
            { 
                int tempCondition = getSelectedStep()->condition;
                getSelectedStep()->condition = constrain(tempCondition + amt, 0, 35); // 0-32
            }
        }
        // PAGE THREE
        else if (selPage == 3)
        {
            if (enc.dir() < 0)
            { // RESET PLOCK IF TURN CCW
                // int tempmode = seqPageParams.nsparam - 11;
                int tempmode = noteSelParams.getSelParam();
                getSelectedStep()->params[tempmode] = -1;
            }
        }
    }
    else
    {
        // TODO This shouldn't be possible. 
        clockConfig.newtempo = constrain(clockConfig.clockbpm + amt, 40, 300);
        if (clockConfig.newtempo != clockConfig.clockbpm)
        {
            // SET TEMPO HERE
            clockConfig.clockbpm = clockConfig.newtempo;
            omxUtil.resetClocks();
        }
    }
    omxDisp.setDirty();
}

void OmxModeSequencer::onEncoderButtonDown()
{
    encoderSelect_ = !encoderSelect_;
    omxDisp.isDirty();
}

void OmxModeSequencer::onEncoderButtonDownLong()
{
    if (getSequencerMode() == SEQMODE_STEPRECORD)
    {
        resetPatternDefaults(sequencer.playingPattern);
        omxDisp.displayMessagef("RESET PAT");
        omxDisp.setDirty();
        // clearedFlag = true;
    }
}

bool OmxModeSequencer::shouldBlockEncEdit()
{
    return stepRecord_;
}

void OmxModeSequencer::onKeyUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();
    int keyPos = thisKey - 11;
    int seqKey = keyPos + (sequencer.patternPage[sequencer.playingPattern] * NUM_STEPKEYS);

    uint8_t seqMode = getSequencerMode();

    // Sequencer row keys

    // ### KEY PRESS EVENTS

    if (e.down() && thisKey != 0)
    {
        // set key timer to zero
        //					keyPressTime[thisKey] = 0;

        // NOTE SELECT
        if (seqMode == SEQMODE_NOTESEL)
        {
            // SET NOTE
            // left and right keys change the octave
            if (thisKey == 11 || thisKey == 26)
            {
                int amt = thisKey == 11 ? -1 : 1;
                midiSettings.octave = constrain(midiSettings.octave + amt, -5, 4);
                // otherwise select the note
            }
            else
            {
                if (!e.held()) // Prevent held F1 key from changing note. 
                {
                    // stepSelect_ = false;
                    seqConfig.selectedNote = thisKey;

                    uint8_t adjNote = getAdjustedNote(thisKey);
                    // int adjnote = notes[thisKey] + (midiSettings.octave * 12);
                    getSelectedStep()->note = adjNote;
                    if (!sequencer.playing)
                    {
                        seqNoteOn(thisKey, midiSettings.defaultVelocity, sequencer.playingPattern);
                    }
                }
            }
            // see RELEASE events for more
            omxDisp.setDirty();

            // // noteSelection_ 
            // if (seqConfig.noteSelection)
            // { 
            //     // SET NOTE
            //     // left and right keys change the octave
            //     if (thisKey == 11 || thisKey == 26)
            //     {
            //         int amt = thisKey == 11 ? -1 : 1;
            //         midiSettings.newoctave = constrain(midiSettings.octave + amt, -5, 4);
            //         if (midiSettings.newoctave != midiSettings.octave)
            //         {
            //             midiSettings.octave = midiSettings.newoctave;
            //         }
            //         // otherwise select the note
            //     }
            //     else
            //     {
            //         seqConfig.stepSelect = false;
            //         seqConfig.selectedNote = thisKey;

            //         uint8_t adjNote = getAdjustedNote(thisKey);
            //         // int adjnote = notes[thisKey] + (midiSettings.octave * 12);
            //         getSelectedStep()->note = adjNote;
            //         if (!sequencer.playing)
            //         {
            //             seqNoteOn(thisKey, midiSettings.defaultVelocity, sequencer.playingPattern);
            //         }
            //     }
            //     // see RELEASE events for more
            //     omxDisp.setDirty();
            // }
            // else if (thisKey == 1)
            // {
            // }
            // else if (thisKey == 2)
            // {
            // }
            // else if (thisKey > 2 && thisKey < 11)
            // { // Pattern select keys
            //     sequencer.playingPattern = thisKey - 3;
            //     omxDisp.setDirty();
            // }
            // else if (thisKey > 10)
            // {
            //     seqConfig.selectedStep = seqKey; // was keyPos // set noteSelection to this step
            //     seqConfig.stepSelect = true;
            //     seqConfig.noteSelection = true;
            //     omxDisp.setDirty();
            // }
        }
        // PATTERN PARAMS
        else if (seqMode == SEQMODE_PAT)
        {
            if (thisKey == 1)
            { // F1
            }
            else if (thisKey == 2)
            { // F2
            }
            else if (thisKey > 2 && thisKey < 11)
            { // Pattern select keys

                sequencer.playingPattern = thisKey - 3;

                // COPY / PASTE / CLEAR
                if (midiSettings.keyState[1] && !midiSettings.keyState[2])
                {
                    copyPattern(sequencer.playingPattern);
                    omxDisp.displayMessagef("COPIED P-%d", sequencer.playingPattern + 1);
                }
                else if (!midiSettings.keyState[1] && midiSettings.keyState[2])
                {
                    pastePattern(sequencer.playingPattern);
                    omxDisp.displayMessagef("PASTED P-%d", sequencer.playingPattern + 1);
                }
                else if (midiSettings.keyState[1] && midiSettings.keyState[2])
                {
                    clearPattern(sequencer.playingPattern);
                    omxDisp.displayMessagef("CLEARED P-%d", sequencer.playingPattern + 1);
                }

                omxDisp.setDirty();
            }
            else if (thisKey > 10)
            {
                // set pattern length with key
                auto newPatternLen = thisKey - 10;
                sequencer.setPatternLength(sequencer.playingPattern, newPatternLen);
                if (sequencer.seqPos[sequencer.playingPattern] >= newPatternLen)
                {
                    sequencer.seqPos[sequencer.playingPattern] = newPatternLen - 1;
                    sequencer.patternPage[sequencer.playingPattern] = getPatternPage(sequencer.seqPos[sequencer.playingPattern]);
                }
                omxDisp.setDirty();
            }
        }
        // STEP RECORD
        else if (seqMode == SEQMODE_STEPRECORD)
        {
            seqConfig.selectedNote = thisKey;
            seqConfig.selectedStep = sequencer.seqPos[sequencer.playingPattern];

            // int adjnote = notes[thisKey] + (midiSettings.octave * 12);
            uint8_t adjnote = getAdjustedNote(thisKey);
            getSelectedStep()->note = adjnote;

            if (!sequencer.playing)
            {
                seqNoteOn(thisKey, midiSettings.defaultVelocity, sequencer.playingPattern);
            } // see RELEASE events for more
            stepDirty_ = true;
            omxDisp.setDirty();
        }
        else if (seqMode == SEQMODE_MAIN)
        {
            // MIDI SOLO
            if (sequencer.getCurrentPattern()->solo)
            {
                omxUtil.midiNoteOn(thisKey, midiSettings.defaultVelocity, sequencer.getCurrentPattern()->channel + 1);
            }
            // REGULAR SEQ MODE
            else
            {
                if (midiSettings.keyState[1] && midiSettings.keyState[2])
                {
                    seqPages_ = true;
                }
                if (thisKey == 1)
                {
                    //							seqResetFlag = true;					// RESET ALL SEQUENCES TO FIRST/LAST STEP
                    // MOVED DOWN TO AUX KEY
                }
                else if (thisKey == 2)
                { // CHANGE PATTERN DIRECTION
                  //							sequencer.getCurrentPattern()->reverse = !sequencer.getCurrentPattern()->reverse;

                    // BLACK KEYS - PATTERNS
                }
                else if (thisKey > 2 && thisKey < 11)
                { // Pattern select

                    // CHECK keyState[] FOR LONG PRESS THINGS

                    // If ONLY KEY 1 is down + pattern is not playing = STEP RECORD
                    if (midiSettings.keyState[1] && !midiSettings.keyState[2] && !sequencer.playing)
                    {
                        // ENTER STEP RECORD MODE
                        sequencer.playingPattern = thisKey - 3;
                        sequencer.seqPos[sequencer.playingPattern] = 0;
                        sequencer.patternPage[sequencer.playingPattern] = 0; // Step Record always starts from first page

                        changeSequencerMode(SEQMODE_STEPRECORD);
                        //								omxDisp.setDirty();;
                    }
                    // If KEY 2 is down + pattern = PATTERN MUTE
                    else if (midiSettings.keyState[2])
                    {
                        if (sequencer.getPattern(thisKey - 3)->mute)
                        {
                            omxDisp.displayMessagef("UNMUTE P-%d", (thisKey - 3) + 1);
                        }
                        else
                        {
                            omxDisp.displayMessagef("MUTE P-%d", (thisKey - 3) + 1);
                        }
                        sequencer.getPattern(thisKey - 3)->mute = !sequencer.getPattern(thisKey - 3)->mute;
                    }
                    else
                    {
                        sequencer.playingPattern = thisKey - 3;
                    }
                    omxDisp.setDirty();
                }
                // SEQUENCE 1-16 STEP KEYS
                else if (thisKey > 10)
                {

                    // F1+F2 HOLD
                    if (midiSettings.keyState[1] && midiSettings.keyState[2])
                    {
                        // IGNORE LONG PRESSES IN STEP RECORD
                        if (!stepRecord_)
                        {
                            if (keyPos <= getPatternPage(sequencer.getCurrentPattern()->len))
                            {
                                sequencer.patternPage[sequencer.playingPattern] = keyPos;
                            }
                            omxDisp.displayMessagef("PATT PAGE %d", keyPos + 1);
                        }
                    }
                    // F1 HOLD
                    else if (midiSettings.keyState[1])
                    {
                        // IGNORE LONG PRESSES IN STEP RECORD and Pattern Params
                        if (!stepRecord_ && !patternParams_)
                        {
                            seqConfig.selectedStep = (thisKey - 11) + (sequencer.patternPage[sequencer.playingPattern] * NUM_STEPKEYS); // set noteSelection to this step
                            // seqConfig.noteSelect = true;
                            // seqConfig.stepSelect = true;
                            // seqConfig.noteSelection = true;
                            // omxDisp.setDirty();
                            // omxDisp.displayMessagef("NOTE SELECT");

                            auto selectedStep = getSelectedStep();
                            stepCopyBuffer_.CopyFrom(selectedStep);

                            changeSequencerMode(SEQMODE_NOTESEL);
                            // re-toggle the key you just held
                            //										if (getSelectedStep()->trig == TRIGTYPE_PLAY || getSelectedStep()->trig == TRIGTYPE_MUTE ) {
                            //											getSelectedStep()->trig = (getSelectedStep()->trig == TRIGTYPE_PLAY ) ? TRIGTYPE_MUTE : TRIGTYPE_PLAY;
                            //										}
                        }
                    }
                    // F2 HOLD - CUT / PASTE
                    else if (midiSettings.keyState[2])
                    {
                        // paste copied note to current
                        seqConfig.selectedStep = (thisKey - 11) + (sequencer.patternPage[sequencer.playingPattern] * NUM_STEPKEYS); // set noteSelection to this step
                        auto selectedStep = getSelectedStep();

                        if(selectedStep->trig == TRIGTYPE_MUTE) // paste copied note to current if trig is off
                        {
                            selectedStep->CopyFrom(&stepCopyBuffer_);
                            tempString = "Paste " + String(seqConfig.selectedStep);
                            omxDisp.displayMessage(tempString.c_str());
                        }
                        else // Cut - copy and turn trig off if trig on
                        {
                            stepCopyBuffer_.CopyFrom(selectedStep);
                            selectedStep->trig = TrigType::TRIGTYPE_MUTE;
                            tempString = "Cut " + String(seqConfig.selectedStep);
                            omxDisp.displayMessage(tempString.c_str());
                        }
                    }
                    else
                    {
                        // TOGGLE STEP ON/OFF
                        if (sequencer.getCurrentPattern()->steps[seqKey].trig == TRIGTYPE_PLAY || sequencer.getCurrentPattern()->steps[seqKey].trig == TRIGTYPE_MUTE)
                        {
                            sequencer.getCurrentPattern()->steps[seqKey].trig = (sequencer.getCurrentPattern()->steps[seqKey].trig == TRIGTYPE_PLAY) ? TRIGTYPE_MUTE : TRIGTYPE_PLAY;
                        }
                    }
                }
            }
        }
    }

    // ### KEY RELEASE EVENTS
    if (!e.down() && thisKey != 0)
    {
        // MIDI SOLO
        if (sequencer.getCurrentPattern()->solo)
        {
            omxUtil.midiNoteOff(thisKey, sequencer.getCurrentPattern()->channel + 1);
        }
    }

    if (!e.down() && thisKey != 0 && (noteSelect_ || stepRecord_) && seqConfig.selectedNote > 0)
    {
        if (!sequencer.playing)
        {
            seqNoteOff(thisKey, sequencer.playingPattern);
        }
        if (stepRecord_ && stepDirty_)
        {
            step_ahead();
            stepDirty_ = false;

            seqConfig.selectedStep = sequencer.seqPos[sequencer.playingPattern];

            // EXIT STEP RECORD AFTER THE LAST STEP IN PATTERN
            if (sequencer.seqPos[sequencer.playingPattern] == 0)
            {
                changeSequencerMode(SEQMODE_MAIN);
            }
        }
    }

    // AUX KEY PRESS EVENTS

    if (e.down() && thisKey == 0)
    {
        if (seqMode == SEQMODE_NOTESEL)
        {
            // if (seqConfig.noteSelection)
            // {
            //     seqConfig.selectedStep = 0;
            //     seqConfig.selectedNote = 0;
            // }

            seqConfig.selectedStep = 0;
            seqConfig.selectedNote = 0;

            changeSequencerMode(SEQMODE_MAIN);
        }
        else if (seqMode == SEQMODE_PAT || seqMode == SEQMODE_STEPRECORD)
        {
            changeSequencerMode(SEQMODE_MAIN);
        }
        else if (seqPages_)
        {
            seqPages_ = false;
        }
        else
        {
            if (midiSettings.keyState[1] || midiSettings.keyState[2])
            { // CHECK keyState[] FOR LONG PRESS OF FUNC KEYS
                if (midiSettings.keyState[1])
                {
                    sequencer.seqResetFlag = true; // RESET ALL SEQUENCES TO FIRST/LAST STEP
                    omxDisp.displayMessagef("RESET");
                }
                else if (midiSettings.keyState[2])
                { // CHANGE PATTERN DIRECTION
                    sequencer.getCurrentPattern()->reverse = !sequencer.getCurrentPattern()->reverse;
                    if (sequencer.getCurrentPattern()->reverse)
                    {
                        omxDisp.displayMessagef("<< REV");
                    }
                    else
                    {
                        omxDisp.displayMessagef("FWD >>");
                    }
                }
                omxLeds.setDirty();
                omxDisp.setDirty();
            }
            else
            {
                if (sequencer.playing)
                {
                    // stop transport
                    sequencer.playing = 0;
                    allNotesOff();
                    //							Serial.println("stop transport");
                    seqStop();
                }
                else
                {
                    // start transport
                    //							Serial.println("start transport");
                    seqStart();
                }
            }
        }

        // AUX KEY RELEASE EVENTS
    }
    else if (!e.down() && thisKey == 0)
    {
    }

    if (!e.down() && (thisKey == 1 || thisKey == 2))
    {
        if (!midiSettings.keyState[1] || !midiSettings.keyState[2])
        {
            // Release page selection whenever F1 && F2 are released
            seqPages_ = false;
        }
    }

    // if (!midiSettings.keyState[1] && !midiSettings.keyState[2])
    // {
    //     seqPageParams.seqPages = false;
    // }

    //				strip.show();
}

void OmxModeSequencer::onKeyHeldUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();

    if (!sequencer.getCurrentPattern()->solo)
    {
        // TODO: access key state directly in omx_keypad.h
        if (midiSettings.keyState[1] && midiSettings.keyState[2])
        {
            seqPages_ = true;
        }
        // SKIP LONG PRESS IF FUNC KEYS ARE ALREDY HELD
        else if (!midiSettings.keyState[1] && !midiSettings.keyState[2])
        {
            // If in main mode
            if (getSequencerMode() == SEQMODE_MAIN)
            {
                // skip AUX key, get pattern keys
                if (thisKey > 2 && thisKey < 11)
                { 
                    if (!stepRecord_)
                    {
                        changeSequencerMode(SEQMODE_PAT);
                    }
                }
                else if (thisKey > 10)
                {
                    // IGNORE LONG PRESSES IN STEP RECORD and Pattern Params
                    seqConfig.selectedStep = (thisKey - 11) + (sequencer.patternPage[sequencer.playingPattern] * NUM_STEPKEYS); // set noteSelection to this step
                    // seqConfig.noteSelect = true;
                    // seqConfig.stepSelect = true;
                    // seqConfig.noteSelection = true;
                    // omxDisp.setDirty();
                    // omxDisp.displayMessagef("NOTE SELECT");

                    // Copy the step to the buffer
                    auto selectedStep = getSelectedStep();
                    stepCopyBuffer_.CopyFrom(selectedStep);

                    changeSequencerMode(SEQMODE_NOTESEL);
                    // re-toggle the key you just held
                    //									if ( getSelectedStep()->trig == TRIGTYPE_PLAY || getSelectedStep()->trig == TRIGTYPE_MUTE ) {
                    //										getSelectedStep()->trig = ( getSelectedStep()->trig == TRIGTYPE_PLAY ) ? TRIGTYPE_MUTE : TRIGTYPE_PLAY;
                    //									}
                }
            }
        }
    }
}

void OmxModeSequencer::showCurrentStepLEDs(int patternNum)
{
    // omxLeds.updateBlinkStates();

	if(sysSettings.screenSaverMode && !sequencer.playing) return; // Screensaver active and not playing, don't update sequencer LEDs. 

    bool blinkState = omxLeds.getBlinkState();
    bool slowBlinkState = omxLeds.getSlowBlinkState();

    // AUX KEY

    if (sequencer.playing && blinkState)
    {
        strip.setPixelColor(0, WHITE);
    }
    else if (noteSelect_ && blinkState)
    {
        strip.setPixelColor(0, NOTESEL);
    }
    else if (patternParams_ && blinkState)
    {
        strip.setPixelColor(0, seqColors[patternNum]);
    }
    else if (stepRecord_ && blinkState)
    {
        strip.setPixelColor(0, seqColors[patternNum]);
    }
    else
    {
        if (!seq2Mode) // S1
        {
            strip.setPixelColor(0, SEQ1C);
        }
        else
        { // S2
            strip.setPixelColor(0, SEQ2C);
        }

        // Default was strip.setPixelColor(0, LEDOFF); should never happen
    }

    if (sequencer.getPattern(patternNum)->mute)
    {
        colorConfig.stepColor = muteColors[patternNum];
    }
    else
    {
        colorConfig.stepColor = seqColors[patternNum];
        colorConfig.muteColor = muteColors[patternNum];
    }

    auto currentpage = sequencer.patternPage[patternNum];
    auto pagestepstart = (currentpage * NUM_STEPKEYS);

    uint8_t seqMode = getSequencerMode();

    // NOTE SELECTION
    if (seqMode == SEQMODE_NOTESEL)
    {
        uint8_t seqPos = seqConfig.selectedStep;
        uint8_t currentNote = sequencer.patterns[sequencer.playingPattern].steps[seqPos].note;

        // 27 LEDS so use LED_COUNT
        for (int j = 1; j < LED_COUNT; j++)
        {
            auto pixelpos = j;
            auto selectedStepPixel = (seqConfig.selectedStep % NUM_STEPKEYS) + 11;
            auto adjNote = getAdjustedNote(j);

            if (adjNote == currentNote)
            {
                strip.setPixelColor(pixelpos, HALFWHITE);
            }
            else if (pixelpos == selectedStepPixel)
            {
                strip.setPixelColor(pixelpos, SEQSTEP);
            }
            else
            {
                strip.setPixelColor(pixelpos, LEDOFF);
            }

            // Blink left/right keys for octave select indicators.
            auto color1 = blinkState ? ORANGE : WHITE;
            auto color2 = blinkState ? RBLUE : WHITE;
            strip.setPixelColor(11, color1);
            strip.setPixelColor(26, color2);
        }
    }
    // STEP RECORD
    else if (seqMode == SEQMODE_STEPRECORD)
    { 
        uint8_t seqPos = sequencer.seqPos[sequencer.playingPattern];
        uint8_t currentNote = sequencer.patterns[sequencer.playingPattern].steps[seqPos].note;

        int seqPosNoteColor = LEDOFF;

        // 27 LEDS so use LED_COUNT
        // This loop sets the key matching the current note to be on and turns other leds off. 
        for (int j = 1; j < LED_COUNT; j++)
        {
            auto pixelpos = j;
            auto adjNote = getAdjustedNote(j);

            // Serial.println((String)"seqPos: " + seqPos + " currentNote: " + currentNote + " pixelPos: " + pixelpos + " adjNote: " + adjNote);

            if (adjNote == currentNote)
            {
                strip.setPixelColor(pixelpos, HALFWHITE);

                // will be overwritten by step indicator
                if(j - 11 == seqPos % 16)
                {
                    seqPosNoteColor = HALFWHITE;
                }
            }
            else
            {
                strip.setPixelColor(pixelpos, LEDOFF);
            }
        }

        for (int j = pagestepstart; j < (pagestepstart + NUM_STEPKEYS); j++)
        {
            auto pixelpos = j - pagestepstart + 11;
            // ONLY DO LEDS FOR THE CURRENT PAGE
            if (j == seqPos)
            {
                // Blinks with the current note number if overlapped, blinks with LEDOFF otherwise. 
                strip.setPixelColor(pixelpos, slowBlinkState ? SEQCHASE : seqPosNoteColor);
            }
        }
    }
    else if (sequencer.getCurrentPattern()->solo)
    { // MIDI SOLO

        //		for(int i = 0; i < NUM_STEPKEYS; i++){
        //			if (i == seqPos[patternNum]){
        //				if (playing){
        //					strip.setPixelColor(i+11, SEQCHASE); // step chase
        //				} else {
        //					strip.setPixelColor(i+11, LEDOFF);  // DO WE NEED TO MARK PLAYHEAD WHEN STOPPED?
        //				}
        //			} else {
        //				strip.setPixelColor(i+11, LEDOFF);
        //			}
        //		}
    }
    else if (seqPages_)
    {
        // BLINK F1+F2
        auto color1 = blinkState ? FUNKONE : LEDOFF;
        auto color2 = blinkState ? FUNKTWO : LEDOFF;
        strip.setPixelColor(1, color1);
        strip.setPixelColor(2, color2);

        // TURN OFF LEDS
        // 27 LEDS so use LED_COUNT
        for (int j = 3; j < LED_COUNT; j++)
        { // START WITH LEDS AFTER F-KEYS
            strip.setPixelColor(j, LEDOFF);
        }
        // SHOW LEDS FOR WHAT PAGE OF SEQ PATTERN YOURE ON
        auto len = (sequencer.getPattern(patternNum)->len / NUM_STEPKEYS);
        for (int h = 0; h <= len; h++)
        {
            auto currentpage = sequencer.patternPage[patternNum];
            auto color = sequencePageColors[h];
            if (h == currentpage)
            {
                color = blinkState ? sequencePageColors[currentpage] : LEDOFF;
            }
            strip.setPixelColor(11 + h, color);
        }
    }
    // PATTERN or MAIN
    else
    {
        for (int j = 1; j < LED_COUNT; j++)
        {
            if (j < sequencer.getPatternLength(patternNum) + 11)
            {
                if (j == 1)
                {
                    // NOTE SELECT / F1
                    if (midiSettings.keyState[j] && blinkState)
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
                    // PATTERN PARAMS / F2
                    if (midiSettings.keyState[j] && blinkState)
                    {
                        strip.setPixelColor(j, LEDOFF);
                    }
                    else
                    {
                        strip.setPixelColor(j, FUNKTWO);
                    }
                }
                else if (j == patternNum + 3)
                { // PATTERN SELECT
                    strip.setPixelColor(j, colorConfig.stepColor);
                    if (patternParams_ && blinkState)
                    {
                        strip.setPixelColor(j, LEDOFF);
                    }
                }
                else
                {
                    strip.setPixelColor(j, LEDOFF);
                }
            }
            else
            {
                strip.setPixelColor(j, LEDOFF);
            }
        }

        auto pattern = sequencer.getPattern(patternNum);
        auto steps = pattern->steps;
        auto currentpage = sequencer.patternPage[patternNum];
        auto pagestepstart = (currentpage * NUM_STEPKEYS);

        // WHAT TO DO HERE FOR MULTIPLE PAGES
        // NUM_STEPKEYS or NUM_STEPS INSTEAD?
        for (int i = pagestepstart; i < (pagestepstart + NUM_STEPKEYS); i++)
        {
            if (i < sequencer.getPatternLength(patternNum))
            {

                // ONLY DO LEDS FOR THE CURRENT PAGE
                auto pixelpos = i - pagestepstart + 11;
                //				if (patternParams){
                // 					strip.setPixelColor(pixelpos, SEQMARKER);
                // 				}

                if (i % 4 == 0)
                { // MARK GROUPS OF 4
                    if (i == sequencer.lastSeqPos[patternNum])
                    {
                        if (sequencer.playing)
                        {
                            strip.setPixelColor(pixelpos, SEQCHASE); // step chase
                        }
                        else if (steps[i].trig == TRIGTYPE_PLAY)
                        {
                            if (steps[i].stepType != STEPTYPE_NONE)
                            {
                                if (slowBlinkState)
                                {
                                    strip.setPixelColor(pixelpos, colorConfig.stepColor); // STEP EVENT COLOR
                                }
                                else
                                {
                                    strip.setPixelColor(pixelpos, colorConfig.muteColor); // STEP EVENT COLOR
                                }
                            }
                            else
                            {
                                strip.setPixelColor(pixelpos, colorConfig.stepColor); // STEP ON COLOR
                            }
                        }
                        else if (steps[i].trig == TRIGTYPE_MUTE)
                        {
                            strip.setPixelColor(pixelpos, SEQMARKER);
                        }
                    }
                    else if (steps[i].trig == TRIGTYPE_PLAY)
                    {
                        if (steps[i].stepType != STEPTYPE_NONE)
                        {
                            if (slowBlinkState)
                            {
                                strip.setPixelColor(pixelpos, colorConfig.stepColor); // STEP EVENT COLOR
                            }
                            else
                            {
                                strip.setPixelColor(pixelpos, colorConfig.muteColor); // STEP EVENT COLOR
                            }
                        }
                        else
                        {
                            strip.setPixelColor(pixelpos, colorConfig.stepColor); // STEP ON COLOR
                        }
                    }
                    else if (steps[i].trig == TRIGTYPE_MUTE)
                    {
                        strip.setPixelColor(pixelpos, SEQMARKER);
                    }
                }
                else if (i == sequencer.lastSeqPos[patternNum])
                { // STEP CHASE
                    if (sequencer.playing)
                    {
                        strip.setPixelColor(pixelpos, SEQCHASE);
                    }
                    else if (steps[i].trig == TRIGTYPE_PLAY)
                    {
                        if (steps[i].stepType != STEPTYPE_NONE)
                        {
                            if (slowBlinkState)
                            {
                                strip.setPixelColor(pixelpos, colorConfig.stepColor); // STEP EVENT COLOR
                            }
                            else
                            {
                                strip.setPixelColor(pixelpos, colorConfig.muteColor); // STEP EVENT COLOR
                            }
                        }
                        else
                        {
                            strip.setPixelColor(pixelpos, colorConfig.stepColor); // STEP ON COLOR
                        }
                    }
                    else if (!patternParams_ && sequencer.patterns[patternNum].steps[i].trig == TRIGTYPE_MUTE)
                    {
                        strip.setPixelColor(pixelpos, LEDOFF); // DO WE NEED TO MARK PLAYHEAD WHEN STOPPED?
                    }
                    else if (patternParams_)
                    {
                        strip.setPixelColor(pixelpos, SEQMARKER);
                    }
                }
                else if (steps[i].trig == TRIGTYPE_PLAY)
                {
                    if (steps[i].stepType != STEPTYPE_NONE)
                    {
                        if (slowBlinkState)
                        {
                            strip.setPixelColor(pixelpos, colorConfig.stepColor); // STEP EVENT COLOR
                        }
                        else
                        {
                            strip.setPixelColor(pixelpos, colorConfig.muteColor); // STEP EVENT COLOR
                        }
                    }
                    else
                    {
                        strip.setPixelColor(pixelpos, colorConfig.stepColor); // STEP ON COLOR
                    }
                }
                else if (!patternParams_ && steps[i].trig == TRIGTYPE_MUTE)
                {
                    strip.setPixelColor(pixelpos, LEDOFF);
                }
                else if (patternParams_)
                {
                    strip.setPixelColor(pixelpos, SEQMARKER);
                }
            }
        }
    }
    omxLeds.setDirty();
}

void OmxModeSequencer::updateLEDs()
{
    showCurrentStepLEDs(sequencer.playingPattern);
}

void OmxModeSequencer::onDisplayUpdate()
{
    // MIDI SOLO
    if (sequencer.getCurrentPattern()->solo)
    {
        omxLeds.drawMidiLeds(musicScale);
    }
    // DISPLAY
    if (omxDisp.isDirty())
    { 
        // show only if not encoder edit or dialog display
        if (!encoderConfig.enc_edit && omxDisp.isMessageActive() == false)
        { 
            uint8_t seqMode = getSequencerMode();
            if (seqMode == SEQMODE_MAIN)
            {
                if (seqParams.getSelPage() == 0) // SUBMODE_SEQ
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "PTN";
                    omxDisp.legends[1] = "TRSP";
                    omxDisp.legends[2] = "SWNG"; //"TRSP";
                    omxDisp.legends[3] = "BPM";
                    omxDisp.legendVals[0] = sequencer.playingPattern + 1;
                    omxDisp.legendVals[1] = (int)midiSettings.transpose;
                    omxDisp.legendVals[2] = (int)sequencer.getCurrentPattern()->swing; //(int)swing;
                    // legendVals[2] =  swing_values[sequencer.getCurrentPattern()->swing];
                    omxDisp.legendVals[3] = (int)clockConfig.clockbpm;
                }
                else if (seqParams.getSelPage() == 1) // SUBMODE_SEQ2
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "SOLO";
                    omxDisp.legends[1] = "LEN";
                    omxDisp.legends[2] = "RATE";
                    omxDisp.legends[3] = "CV";                                   // cvPattern
                    omxDisp.legendVals[0] = sequencer.getCurrentPattern()->solo; // playingPattern+1;
                    omxDisp.legendVals[1] = sequencer.getPatternLength(sequencer.playingPattern);
                    omxDisp.legendVals[2] = -127;
                    omxDisp.legendText[2] = mdivs[sequencer.getCurrentPattern()->clockDivMultP];
                    omxDisp.legendVals[3] = -127; // TODO is this right?
                    if (sequencer.getCurrentPattern()->sendCV)
                    {
                        omxDisp.legendText[3] = "On";
                    }
                    else
                    {
                        omxDisp.legendText[3] = "Off";
                    }
                }
                omxDisp.dispGenericMode2(2, seqParams.getSelPage(), seqParams.getSelParam(), encoderSelect_);
            }
            else if (seqMode == SEQMODE_NOTESEL)
            {
                if (noteSelParams.getSelPage() == 0) // SUBMODE_NOTESEL
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "NOTE";
                    omxDisp.legends[1] = "OCT";
                    omxDisp.legends[2] = "VEL";
                    omxDisp.legends[3] = "LEN";
                    omxDisp.legendVals[0] = getSelectedStep()->note;
                    omxDisp.legendVals[1] = (int)midiSettings.octave + 4;
                    omxDisp.legendVals[2] = getSelectedStep()->vel;
                    omxDisp.useLegendString[3] = true;
                    omxDisp.legendString[3] = String(kNoteLengths[getSelectedStep()->len]);
                }
                else if (noteSelParams.getSelPage() == 1) // SUBMODE_NOTESEL2
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "TYPE";
                    omxDisp.legends[1] = "PROB";
                    omxDisp.legends[2] = "COND";
                    omxDisp.legends[3] = "";
                    omxDisp.legendVals[0] = -127;
                    omxDisp.legendText[0] = stepTypes[getSelectedStep()->stepType];
                    omxDisp.legendVals[1] = getSelectedStep()->prob;
                    //				String ac = String(trigConditionsAB[][0]);
                    //				String bc = String(trigConditionsAB[getSelectedStep()->condition][1]);

                    omxDisp.legendVals[2] = -127;
                    omxDisp.legendText[2] = trigConditions[getSelectedStep()->condition]; // ac + bc; // trigConditions

                    omxDisp.legendVals[3] = 0;
                }
                else if (noteSelParams.getSelPage() == 2) // SUBMODE_NOTESEL3
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "L-1";
                    omxDisp.legends[1] = "L-2";
                    omxDisp.legends[2] = "L-3";
                    omxDisp.legends[3] = "L-4";
                    for (int j = 0; j < 4; j++)
                    {
                        int stepNoteParam = getSelectedStep()->params[j];
                        if (stepNoteParam > -1)
                        {
                            omxDisp.legendVals[j] = stepNoteParam;
                        }
                        else
                        {
                            omxDisp.legendVals[j] = -127;
                            omxDisp.legendText[j] = "---";
                        }
                    }
                }
                omxDisp.dispGenericMode2(3, noteSelParams.getSelPage(), noteSelParams.getSelParam(), encoderSelect_);
            }
            else if (seqMode == SEQMODE_PAT)
            {
                if (patParams.getSelPage() == 0) // SUBMODE_PATTPARAMS
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "PTN";
                    omxDisp.legends[1] = "LEN";
                    omxDisp.legends[2] = "ROT";
                    omxDisp.legends[3] = "CHAN";
                    omxDisp.legendVals[0] = sequencer.playingPattern + 1;
                    omxDisp.legendVals[1] = sequencer.getPatternLength(sequencer.playingPattern);
                    omxDisp.legendVals[2] = midiSettings.rotationAmt; //(int)transpose;
                    omxDisp.legendVals[3] = sequencer.getPatternChannel(sequencer.playingPattern);
                }
                else if (patParams.getSelPage() == 1) // SUBMODE_PATTPARAMS2
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "START";
                    omxDisp.legends[1] = "END";
                    omxDisp.legends[2] = "FREQ";
                    omxDisp.legends[3] = "PROB";
                    omxDisp.legendVals[0] = sequencer.getCurrentPattern()->startstep + 1; // STRT step to autoreset on
                    omxDisp.legendVals[1] = sequencer.getCurrentPattern()->autoresetstep; // STP step to autoreset on - 0 = no auto reset
                    omxDisp.legendVals[2] = sequencer.getCurrentPattern()->autoresetfreq; // FRQ to autoreset on -- every x cycles
                    omxDisp.legendVals[3] = sequencer.getCurrentPattern()->autoresetprob; // PRO probability of resetting 0=NEVER 1=Always 2=50%
                }
                else if (patParams.getSelPage() == 2) // SUBMODE_PATTPARAMS3
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "RATE";
                    omxDisp.legends[1] = "SOLO";
                    omxDisp.legends[2] = "---";
                    omxDisp.legends[3] = "---";

                    // RATE FOR CURR PATTERN
                    omxDisp.legendVals[0] = -127;
                    omxDisp.legendText[0] = mdivs[sequencer.getCurrentPattern()->clockDivMultP];

                    omxDisp.legendVals[1] = sequencer.getCurrentPattern()->solo;
                    omxDisp.legendVals[2] = 0; // TBD
                    omxDisp.legendVals[3] = 0; // TBD
                }
                omxDisp.dispGenericMode2(3, patParams.getSelPage(), patParams.getSelParam(), encoderSelect_);
            }
            else if (seqMode == SEQMODE_STEPRECORD)
            {
                if (sRecParams.getSelPage() == 0) // SUBMODE_STEPREC
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "OCT";
                    omxDisp.legends[1] = "STEP";
                    omxDisp.legends[2] = "NOTE";
                    omxDisp.legends[3] = "PTN";
                    omxDisp.legendVals[0] = (int)midiSettings.octave + 4;
                    omxDisp.legendVals[1] = sequencer.seqPos[sequencer.playingPattern] + 1;
                    omxDisp.legendVals[2] = getSelectedStep()->note; //(int)transpose;
                    omxDisp.legendVals[3] = sequencer.playingPattern + 1;
                }
                else if (sRecParams.getSelPage() == 1) // SUBMODE_NOTESEL2
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "TYPE";
                    omxDisp.legends[1] = "PROB";
                    omxDisp.legends[2] = "COND";
                    omxDisp.legends[3] = "";
                    omxDisp.legendVals[0] = -127;
                    omxDisp.legendText[0] = stepTypes[getSelectedStep()->stepType];
                    omxDisp.legendVals[1] = getSelectedStep()->prob;
                    //				String ac = String(trigConditionsAB[][0]);
                    //				String bc = String(trigConditionsAB[getSelectedStep()->condition][1]);

                    omxDisp.legendVals[2] = -127;
                    omxDisp.legendText[2] = trigConditions[getSelectedStep()->condition]; // ac + bc; // trigConditions

                    omxDisp.legendVals[3] = 0;
                }
                omxDisp.dispGenericMode2(3, sRecParams.getSelPage(), sRecParams.getSelParam(), encoderSelect_);
            }
        }
    }
}

void OmxModeSequencer::initPatterns()
{
    // default to GM Drum Map for now -- GET THIS FROM patternDefaultNoteMap instead
    //	uint8_t initNotes[NUM_PATTERNS] = {
    //		36,
    //		38,
    //		37,
    //		39,
    //		42,
    //		46,
    //		49,
    //		51 };

    StepNote stepNote = {0, 100, defaultNoteLength, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE};
    // {note, vel, len, TRIGTYPE, {params0, params1, params2, params3, params4}, prob, condition, STEPTYPE}

    for (int i = 0; i < NUM_SEQ_PATTERNS; i++)
    {
        auto pattern = sequencer.getPattern(i);

        stepNote.note = sequencer.patternDefaultNoteMap[i]; // Defined in sequencer.h

        for (int j = 0; j < NUM_STEPS; j++)
        {
            memcpy(&pattern->steps[j], &stepNote, sizeof(StepNote));
        }

        // TODO: move to sequencer.h
        pattern->len = 15;
        pattern->channel = i; // 0 - 15 becomes 1 - 16
        pattern->startstep = 0;
        pattern->autoresetstep = 0;
        pattern->autoresetfreq = 0;
        pattern->current_cycle = 1;
        pattern->rndstep = 3;
        pattern->clockDivMultP = 2;
        pattern->autoresetprob = 0;
        pattern->swing = 0;
        pattern->reverse = false;
        pattern->mute = false;
        pattern->autoreset = false;
        pattern->solo = false;
        pattern->sendCV = false;
    }
}

void OmxModeSequencer::SetScale(MusicScales* scale){
    this->musicScale = scale;
}
