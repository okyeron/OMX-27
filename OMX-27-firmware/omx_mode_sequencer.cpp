#include "omx_mode_sequencer.h"
#include "config.h"
#include "colors.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "sequencer.h"
#include "omx_leds.h"

void OmxModeSequencer::InitSetup()
{
    initSetup = true;
}

void OmxModeSequencer::onModeActivated()
{
    if(!initSetup){
        InitSetup();
    }
}

void OmxModeSequencer::onPotChanged(int potIndex, int potValue)
{
    if (seqConfig.noteSelect && seqConfig.noteSelection)
    { // note selection - do P-Locks
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
    else if (seqConfig.stepRecord)
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
    else if (!seqConfig.noteSelect || !seqConfig.stepRecord)
    {
        omxUtil.sendPots(potIndex, sequencer.getPatternChannel(sequencer.playingPattern));
    }
}

void OmxModeSequencer::loopUpdate()
{
    if (!seq2Mode) // S1
    {
        doStepS1();
    }
    else
    { // S2
        doStepS2();
    }
}

void OmxModeSequencer::onEncoderChanged(Encoder::Update enc)
{
    if (!seqConfig.noteSelect && !seqPageParams.patternParams && !seqConfig.stepRecord)
    {
        onEncoderChangedNorm(enc);
    }
    else
    {
        onEncoderChangedStep(enc);
    }
}

void OmxModeSequencer::onEncoderChangedNorm(Encoder::Update enc)
{
    auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)

    // CHANGE PAGE
    if (seqPageParams.sqparam  == 0 || seqPageParams.sqparam  == 5)
    {
        seqPageParams.sqpage = constrain(seqPageParams.sqpage + amt, 0, 1);
        seqPageParams.sqparam  = seqPageParams.sqpage * NUM_DISP_PARAMS;
    }

    // PAGE ONE
    if (seqPageParams.sqparam  == 1)
    {
        sequencer.playingPattern = constrain(sequencer.playingPattern + amt, 0, 7);
        if (sequencer.getCurrentPattern()->solo)
        {
            omxLeds.setAllLEDS(0, 0, 0);
        }
    }
    else if (seqPageParams.sqparam  == 2)
    {                                                // SET TRANSPOSE
        transposeSeq(sequencer.playingPattern, amt); //
        int newtransp = constrain(midiSettings.transpose + amt, -64, 63);
        midiSettings.transpose = newtransp;
    }
    else if (seqPageParams.sqparam  == 3)
    {                                                                                                       // SET SWING
        int newswing = constrain(sequencer.getCurrentPattern()->swing + amt, 0, midiSettings.maxswing - 1); // -1 to deal with display values
        midiSettings.swing = newswing;
        sequencer.getCurrentPattern()->swing = newswing;
        //	setGlobalSwing(newswing);
    }
    else if (seqPageParams.sqparam  == 4)
    { // SET TEMPO
        clockConfig.newtempo = constrain(clockConfig.clockbpm + amt, 40, 300);
        if (clockConfig.newtempo != clockConfig.clockbpm)
        {
            // SET TEMPO HERE
            clockConfig.clockbpm = clockConfig.newtempo;
            omxUtil.resetClocks();
        }
    }

    // PAGE TWO
    if (seqPageParams.sqparam  == 6)
    { //  MIDI SOLO
        //						playingPattern = constrain(playingPattern + amt, 0, 7);
        sequencer.getCurrentPattern()->solo = constrain(sequencer.getCurrentPattern()->solo + amt, 0, 1);
        if (sequencer.getCurrentPattern()->solo)
        {
            omxLeds.setAllLEDS(0, 0, 0);
        }
    }
    else if (seqPageParams.sqparam  == 7)
    { // SET PATTERN LENGTH
        auto newPatternLen = constrain(sequencer.getPatternLength(sequencer.playingPattern) + amt, 1, NUM_STEPS);
        sequencer.setPatternLength(sequencer.playingPattern, newPatternLen);
        if (sequencer.seqPos[sequencer.playingPattern] >= newPatternLen)
        {
            sequencer.seqPos[sequencer.playingPattern] = newPatternLen - 1;
            sequencer.patternPage[sequencer.playingPattern] = getPatternPage(sequencer.seqPos[sequencer.playingPattern]);
        }
    }
    else if (seqPageParams.sqparam  == 8)
    { // SET CLOCK DIV/MULT
        sequencer.getCurrentPattern()->clockDivMultP = constrain(sequencer.getCurrentPattern()->clockDivMultP + amt, 0, NUM_MULTDIVS - 1);
    }
    else if (seqPageParams.sqparam  == 9)
    { // SET CV ON/OFF
        sequencer.getCurrentPattern()->sendCV = constrain(sequencer.getCurrentPattern()->sendCV + amt, 0, 1);
    }
    omxDisp.setDirty();
}

// TODO: break this into separate functions
void OmxModeSequencer::onEncoderChangedStep(Encoder::Update enc)
{
    auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)

    // should be no need to check enc_edit, function will only be called if enc_edit is false
    if (seqPageParams.patternParams && !encoderConfig.enc_edit)
    { // SEQUENCE PATTERN PARAMS SUB MODE
        // CHANGE PAGE
        if (seqPageParams.ppparam == 0 || seqPageParams.ppparam == 5 || seqPageParams.ppparam == 10)
        {
            seqPageParams.pppage = constrain(seqPageParams.pppage + amt, 0, 2); // HARDCODED - FIX WITH SIZE OF PAGES?
            seqPageParams.ppparam = seqPageParams.pppage * NUM_DISP_PARAMS;
        }

        // PAGE ONE
        if (seqPageParams.ppparam == 1)
        { // SET PLAYING PATTERN
            sequencer.playingPattern = constrain(sequencer.playingPattern + amt, 0, 7);
        }
        if (seqPageParams.ppparam == 2)
        { // SET LENGTH
            auto newPatternLen = constrain(sequencer.getPatternLength(sequencer.playingPattern) + amt, 1, NUM_STEPS);
            sequencer.setPatternLength(sequencer.playingPattern, newPatternLen);
            if (sequencer.seqPos[sequencer.playingPattern] >= newPatternLen)
            {
                sequencer.seqPos[sequencer.playingPattern] = newPatternLen - 1;
                sequencer.patternPage[sequencer.playingPattern] = getPatternPage(sequencer.seqPos[sequencer.playingPattern]);
            }
        }
        if (seqPageParams.ppparam == 3)
        { // SET PATTERN ROTATION
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

        if (seqPageParams.ppparam == 4)
        { // SET PATTERN CHANNEL
            sequencer.getCurrentPattern()->channel = constrain(sequencer.getCurrentPattern()->channel + amt, 0, 15);
        }
        // PATTERN PARAMS PAGE 2
        // TODO: convert to case statement ??
        if (seqPageParams.ppparam == 6)
        { // SET AUTO START STEP
            sequencer.getCurrentPattern()->startstep = constrain(sequencer.getCurrentPattern()->startstep + amt, 0, sequencer.getCurrentPattern()->len);
            // sequencer.getCurrentPattern()->startstep--;
        }
        if (seqPageParams.ppparam == 7)
        { // SET AUTO RESET STEP
            int tempresetstep = sequencer.getCurrentPattern()->autoresetstep + amt;
            sequencer.getCurrentPattern()->autoresetstep = constrain(tempresetstep, 0, sequencer.getCurrentPattern()->len + 1);
        }
        if (seqPageParams.ppparam == 8)
        {                                                                                                                        // SET AUTO RESET FREQUENCY
            sequencer.getCurrentPattern()->autoresetfreq = constrain(sequencer.getCurrentPattern()->autoresetfreq + amt, 0, 15); // max every 16 times
        }
        if (seqPageParams.ppparam == 9)
        {                                                                                                                         // SET AUTO RESET PROB
            sequencer.getCurrentPattern()->autoresetprob = constrain(sequencer.getCurrentPattern()->autoresetprob + amt, 0, 100); // never, 100% - 33%
        }

        // PAGE THREE
        if (seqPageParams.ppparam == 11)
        {                                                                                                                                      // SET CLOCK-DIV-MULT
            sequencer.getCurrentPattern()->clockDivMultP = constrain(sequencer.getCurrentPattern()->clockDivMultP + amt, 0, NUM_MULTDIVS - 1); // set clock div/mult
        }
        if (seqPageParams.ppparam == 12)
        { // SET MIDI SOLO
            sequencer.getCurrentPattern()->solo = constrain(sequencer.getCurrentPattern()->solo + amt, 0, 1);
        }

        // STEP RECORD SUB MODE
    }
    else if (seqConfig.stepRecord && !encoderConfig.enc_edit)
    {
        // CHANGE PAGE
        if (seqPageParams.srparam == 0 || seqPageParams.srparam == 5)
        {
            seqPageParams.srpage = constrain(seqPageParams.srpage + amt, 0, 1); // HARDCODED - FIX WITH SIZE OF PAGES?
            seqPageParams.srparam = seqPageParams.srpage * NUM_DISP_PARAMS;
        }

        // PAGE ONE
        if (seqPageParams.srparam == 1)
        { // OCTAVE SELECTION
            midiSettings.newoctave = constrain(midiSettings.octave + amt, -5, 4);
            if (midiSettings.newoctave != midiSettings.octave)
            {
                midiSettings.octave = midiSettings.newoctave;
            }
        }
        if (seqPageParams.srparam == 2)
        { // STEP SELECTION
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
        if (seqPageParams.srparam == 3)
        { // SET NOTE NUM
            int tempNote = getSelectedStep()->note;
            getSelectedStep()->note = constrain(tempNote + amt, 0, 127);
        }
        if (seqPageParams.srparam == 4)
        {
            //							playingPattern = constrain(playingPattern + amt, 0, 7);
        }
        // PAGE TWO
        if (seqPageParams.srparam == 6)
        { // STEP TYPE
            changeStepType(amt);
        }
        if (seqPageParams.srparam == 7)
        { // STEP PROB
            int tempProb = getSelectedStep()->prob;
            getSelectedStep()->prob = constrain(tempProb + amt, 0, 100); // Note Len between 1-16
        }
        if (seqPageParams.srparam == 8)
        { // STEP CONDITION
            int tempCondition = getSelectedStep()->condition;
            getSelectedStep()->condition = constrain(tempCondition + amt, 0, 35); // 0-32
        }

        // NOTE SELECT MODE
    }
    else if (seqConfig.noteSelect && seqConfig.noteSelection && !encoderConfig.enc_edit)
    {
        // CHANGE PAGE
        if (seqPageParams.nsparam == 0 || seqPageParams.nsparam == 5 || seqPageParams.nsparam == 10)
        {
            seqPageParams.nspage = constrain(seqPageParams.nspage + amt, 0, 2); // HARDCODED - FIX WITH SIZE OF PAGES?
            seqPageParams.nsparam = seqPageParams.nspage * NUM_DISP_PARAMS;
        }

        // PAGE THREE
        if (seqPageParams.nsparam > 10 && seqPageParams.nsparam < 14)
        {
            if (enc.dir() < 0)
            { // RESET PLOCK IF TURN CCW
                int tempmode = seqPageParams.nsparam - 11;
                getSelectedStep()->params[tempmode] = -1;
            }
        }
        // PAGE ONE
        if (seqPageParams.nsparam == 1)
        { // SET NOTE NUM
            int tempNote = getSelectedStep()->note;
            getSelectedStep()->note = constrain(tempNote + amt, 0, 127);
        }
        if (seqPageParams.nsparam == 2)
        { // SET OCTAVE
            midiSettings.newoctave = constrain(midiSettings.octave + amt, -5, 4);
            if (midiSettings.newoctave != midiSettings.octave)
            {
                midiSettings.octave = midiSettings.newoctave;
            }
        }
        if (seqPageParams.nsparam == 3)
        { // SET VELOCITY
            int tempVel = getSelectedStep()->vel;
            getSelectedStep()->vel = constrain(tempVel + amt, 0, 127);
        }
        if (seqPageParams.nsparam == 4)
        { // SET NOTE LENGTH
            int tempLen = getSelectedStep()->len;
            getSelectedStep()->len = constrain(tempLen + amt, 0, 15); // Note Len between 1-16
        }
        // PAGE TWO
        if (seqPageParams.nsparam == 6)
        { // SET STEP TYPE
            changeStepType(amt);
        }
        if (seqPageParams.nsparam == 7)
        { // SET STEP PROB
            int tempProb = getSelectedStep()->prob;
            getSelectedStep()->prob = constrain(tempProb + amt, 0, 100); // Note Len between 1-16
        }
        if (seqPageParams.nsparam == 8)
        { // SET STEP TRIG CONDITION
            int tempCondition = getSelectedStep()->condition;
            getSelectedStep()->condition = constrain(tempCondition + amt, 0, 35); // 0-32
        }
    }
    else
    {
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
    if (seqConfig.noteSelect && seqConfig.noteSelection && !seqPageParams.patternParams)
    {
        seqPageParams.nsparam = (seqPageParams.nsparam + 1) % 15;
        if (seqPageParams.nsparam > 9)
        {
            seqPageParams.nspage = 2;
        }
        else if (seqPageParams.nsparam > 4)
        {
            seqPageParams.nspage = 1;
        }
        else
        {
            seqPageParams.nspage = 0;
        }
    }
    else if (seqPageParams.patternParams)
    {
        seqPageParams.ppparam = (seqPageParams.ppparam + 1) % 15;
        if (seqPageParams.ppparam > 9)
        {
            seqPageParams.pppage = 2;
        }
        else if (seqPageParams.ppparam > 4)
        {
            seqPageParams.pppage = 1;
        }
        else
        {
            seqPageParams.pppage = 0;
        }
    }
    else if (seqConfig.stepRecord)
    {
        seqPageParams.srparam = (seqPageParams.srparam + 1) % 10;
        if (seqPageParams.srparam > 4)
        {
            seqPageParams.srpage = 1;
        }
        else
        {
            seqPageParams.srpage = 0;
        }
    }
    else
    {
        seqPageParams.sqparam = (seqPageParams.sqparam  + 1) % 10;
        if (seqPageParams.sqparam  > 4)
        {
            seqPageParams.sqpage = 1;
        }
        else
        {
            seqPageParams.sqpage = 0;
        }
    }
}

void OmxModeSequencer::onEncoderButtonDownLong()
{
    if (seqConfig.stepRecord)
    {
        resetPatternDefaults(sequencer.playingPattern);
        // clearedFlag = true;
    }
}

bool OmxModeSequencer::shouldBlockEncEdit()
{
    return seqConfig.stepRecord;
}

void OmxModeSequencer::onKeyUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();
    int keyPos = thisKey - 11;
    int seqKey = keyPos + (sequencer.patternPage[sequencer.playingPattern] * NUM_STEPKEYS);

    // Sequencer row keys

    // ### KEY PRESS EVENTS

    if (e.down() && thisKey != 0)
    {
        // set key timer to zero
        //					keyPressTime[thisKey] = 0;

        // NOTE SELECT
        if (seqConfig.noteSelect)
        {
            if (seqConfig.noteSelection)
            { // SET NOTE
                // left and right keys change the octave
                if (thisKey == 11 || thisKey == 26)
                {
                    int amt = thisKey == 11 ? -1 : 1;
                    midiSettings.newoctave = constrain(midiSettings.octave + amt, -5, 4);
                    if (midiSettings.newoctave != midiSettings.octave)
                    {
                        midiSettings.octave = midiSettings.newoctave;
                    }
                    // otherwise select the note
                }
                else
                {
                    seqConfig.stepSelect = false;
                    seqConfig.selectedNote = thisKey;
                    int adjnote = notes[thisKey] + (midiSettings.octave * 12);
                    getSelectedStep()->note = adjnote;
                    if (!sequencer.playing)
                    {
                        seqNoteOn(thisKey, midiSettings.defaultVelocity, sequencer.playingPattern);
                    }
                }
                // see RELEASE events for more
                omxDisp.setDirty();
            }
            else if (thisKey == 1)
            {
            }
            else if (thisKey == 2)
            {
            }
            else if (thisKey > 2 && thisKey < 11)
            { // Pattern select keys
                sequencer.playingPattern = thisKey - 3;
                omxDisp.setDirty();
            }
            else if (thisKey > 10)
            {
                seqConfig.selectedStep = seqKey; // was keyPos // set noteSelection to this step
                seqConfig.stepSelect = true;
                seqConfig.noteSelection = true;
                omxDisp.setDirty();
            }

            // PATTERN PARAMS
        }
        else if (seqPageParams.patternParams)
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

            // STEP RECORD
        }
        else if (seqConfig.stepRecord)
        {
            seqConfig.selectedNote = thisKey;
            seqConfig.selectedStep = sequencer.seqPos[sequencer.playingPattern];

            int adjnote = notes[thisKey] + (midiSettings.octave * 12);
            getSelectedStep()->note = adjnote;

            if (!sequencer.playing)
            {
                seqNoteOn(thisKey, midiSettings.defaultVelocity, sequencer.playingPattern);
            } // see RELEASE events for more
            seqConfig.stepDirty = true;
            omxDisp.setDirty();

            // MIDI SOLO
        }
        else if (sequencer.getCurrentPattern()->solo)
        {
            omxUtil.midiNoteOn(thisKey, midiSettings.defaultVelocity, sequencer.getCurrentPattern()->channel + 1);

            // REGULAR SEQ MODE
        }
        else
        {
            if (midiSettings.keyState[1] && midiSettings.keyState[2])
            {
                seqPageParams.seqPages = true;
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
                    sequencer.playingPattern = thisKey - 3;
                    sequencer.seqPos[sequencer.playingPattern] = 0;
                    sequencer.patternPage[sequencer.playingPattern] = 0; // Step Record always starts from first page
                    seqConfig.stepRecord = true;
                    omxDisp.displayMessagef("STEP RECORD");
                    //								omxDisp.setDirty();;

                    // If KEY 2 is down + pattern = PATTERN MUTE
                }
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

                // SEQUENCE 1-16 STEP KEYS
            }
            else if (thisKey > 10)
            {

                if (midiSettings.keyState[1] && midiSettings.keyState[2])
                { // F1+F2 HOLD
                    if (!seqConfig.stepRecord)
                    { // IGNORE LONG PRESSES IN STEP RECORD
                        if (keyPos <= getPatternPage(sequencer.getCurrentPattern()->len))
                        {
                            sequencer.patternPage[sequencer.playingPattern] = keyPos;
                        }
                        omxDisp.displayMessagef("PATT PAGE %d", keyPos + 1);
                    }
                }
                else if (midiSettings.keyState[1])
                { // F1 HOLD
                    if (!seqConfig.stepRecord && !seqPageParams.patternParams)
                    {                                // IGNORE LONG PRESSES IN STEP RECORD and Pattern Params
                        seqConfig.selectedStep = thisKey - 11; // set noteSelection to this step
                        seqConfig.noteSelect = true;
                        seqConfig.stepSelect = true;
                        seqConfig.noteSelection = true;
                        omxDisp.setDirty();
                        omxDisp.displayMessagef("NOTE SELECT");
                        // re-toggle the key you just held
                        //										if (getSelectedStep()->trig == TRIGTYPE_PLAY || getSelectedStep()->trig == TRIGTYPE_MUTE ) {
                        //											getSelectedStep()->trig = (getSelectedStep()->trig == TRIGTYPE_PLAY ) ? TRIGTYPE_MUTE : TRIGTYPE_PLAY;
                        //										}
                    }
                }
                else if (midiSettings.keyState[2])
                { // F2 HOLD
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

    // ### KEY RELEASE EVENTS

    if (!e.down() && thisKey != 0)
    {
        // MIDI SOLO
        if (sequencer.getCurrentPattern()->solo)
        {
            omxUtil.midiNoteOff(thisKey, sequencer.getCurrentPattern()->channel + 1);
        }
    }

    if (!e.down() && thisKey != 0 && (seqConfig.noteSelection || seqConfig.stepRecord) && seqConfig.selectedNote > 0)
    {
        if (!sequencer.playing)
        {
            seqNoteOff(thisKey, sequencer.playingPattern);
        }
        if (seqConfig.stepRecord && seqConfig.stepDirty)
        {
            step_ahead();
            seqConfig.stepDirty = false;
            // EXIT STEP RECORD AFTER THE LAST STEP IN PATTERN
            if (sequencer.seqPos[sequencer.playingPattern] == 0)
            {
                seqConfig.stepRecord = false;
            }
        }
    }

    // AUX KEY PRESS EVENTS

    if (e.down() && thisKey == 0)
    {

        if (seqConfig.noteSelect)
        {
            if (seqConfig.noteSelection)
            {
                seqConfig.selectedStep = 0;
                seqConfig.selectedNote = 0;
            }
            else
            {
            }
            seqConfig.noteSelection = false;
            seqConfig.noteSelect = !seqConfig.noteSelect;
            omxDisp.setDirty();
        }
        else if (seqPageParams.patternParams)
        {
            seqPageParams.patternParams = !seqPageParams.patternParams;
            omxDisp.setDirty();
        }
        else if (seqConfig.stepRecord)
        {
            seqConfig.stepRecord = !seqConfig.stepRecord;
            omxDisp.setDirty();
        }
        else if (seqPageParams.seqPages)
        {
            seqPageParams.seqPages = false;
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
            seqPageParams.seqPages = true;
        }
        else if (!midiSettings.keyState[1] && !midiSettings.keyState[2])
        { // SKIP LONG PRESS IF FUNC KEYS ARE ALREDY HELD
            if (thisKey > 2 && thisKey < 11)
            { // skip AUX key, get pattern keys
                seqPageParams.patternParams = true;
                omxDisp.setDirty();
                omxDisp.displayMessagef("PATT PARAMS");
            }
            else if (thisKey > 10)
            {
                if (!seqConfig.stepRecord && !seqPageParams.patternParams)
                {                                                                                                     // IGNORE LONG PRESSES IN STEP RECORD and Pattern Params
                    seqConfig.selectedStep = (thisKey - 11) + (sequencer.patternPage[sequencer.playingPattern] * NUM_STEPKEYS); // set noteSelection to this step
                    seqConfig.noteSelect = true;
                    seqConfig.stepSelect = true;
                    seqConfig.noteSelection = true;
                    omxDisp.setDirty();
                    omxDisp.displayMessagef("NOTE SELECT");
                    // re-toggle the key you just held
                    //									if ( getSelectedStep()->trig == TRIGTYPE_PLAY || getSelectedStep()->trig == TRIGTYPE_MUTE ) {
                    //										getSelectedStep()->trig = ( getSelectedStep()->trig == TRIGTYPE_PLAY ) ? TRIGTYPE_MUTE : TRIGTYPE_PLAY;
                    //									}
                }
            }
        }
    }
}

void OmxModeSequencer::showCurrentStep(int patternNum)
{

    omxLeds.updateLeds();

    bool blinkState = omxLeds.getBlinkState();
    bool slowBlinkState = omxLeds.getSlowBlinkState();

    // AUX KEY

    if (sequencer.playing && blinkState)
    {
        strip.setPixelColor(0, WHITE);
    }
    else if (seqConfig.noteSelect && blinkState)
    {
        strip.setPixelColor(0, NOTESEL);
    }
    else if (seqPageParams.patternParams && blinkState)
    {
        strip.setPixelColor(0, seqColors[patternNum]);
    }
    else if (seqConfig.stepRecord && blinkState)
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

    if (seqConfig.noteSelect && seqConfig.noteSelection)
    {
        // 27 LEDS so use LED_COUNT
        for (int j = 1; j < LED_COUNT; j++)
        {
            auto pixelpos = j;
            auto selectedStepPixel = (seqConfig.selectedStep % NUM_STEPKEYS) + 11;

            if (pixelpos == seqConfig.selectedNote)
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
    else if (seqConfig.stepRecord)
    { // STEP RECORD
        //		int numPages = ceil(float(sequencer.getPatternLength(patternNum))/float(NUM_STEPKEYS));

        // BLANK THE TOP ROW
        for (int j = 1; j < LED_COUNT - NUM_STEPKEYS; j++)
        {
            strip.setPixelColor(j, LEDOFF);
        }

        for (int j = pagestepstart; j < (pagestepstart + NUM_STEPKEYS); j++)
        {
            auto pixelpos = j - pagestepstart + 11;
            //			if (j < sequencer.getPatternLength(patternNum)){
            // ONLY DO LEDS FOR THE CURRENT PAGE
            if (j == sequencer.seqPos[sequencer.playingPattern])
            {
                strip.setPixelColor(pixelpos, SEQCHASE);
            }
            else if (pixelpos != seqConfig.selectedNote)
            {
                strip.setPixelColor(pixelpos, LEDOFF);
            }
            //			} else  {
            //				strip.setPixelColor(pixelpos, LEDOFF);
            //			}
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
    else if (seqPageParams.seqPages)
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
                    if (seqPageParams.patternParams && blinkState)
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
                    if (i == sequencer.seqPos[patternNum])
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
                else if (i == sequencer.seqPos[patternNum])
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
                    else if (!seqPageParams.patternParams && sequencer.patterns[patternNum].steps[i].trig == TRIGTYPE_MUTE)
                    {
                        strip.setPixelColor(pixelpos, LEDOFF); // DO WE NEED TO MARK PLAYHEAD WHEN STOPPED?
                    }
                    else if (seqPageParams.patternParams)
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
                else if (!seqPageParams.patternParams && steps[i].trig == TRIGTYPE_MUTE)
                {
                    strip.setPixelColor(pixelpos, LEDOFF);
                }
                else if (seqPageParams.patternParams)
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
}

void OmxModeSequencer::onDisplayUpdate()
{
    // MIDI SOLO
    if (sequencer.getCurrentPattern()->solo)
    {
        omxLeds.drawMidiLeds();
    }
    if (omxDisp.isDirty())
    { // DISPLAY
        if (!encoderConfig.enc_edit && omxDisp.isMessageActive() == false)
        { // show only if not encoder edit or dialog display
            if (!seqConfig.noteSelect and !seqPageParams.patternParams and !seqConfig.stepRecord)
            {
                int pselected = seqPageParams.sqparam  % NUM_DISP_PARAMS;
                if (seqPageParams.sqpage == 0) // SUBMODE_SEQ
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
                    omxDisp.dispPage = 1;

                    omxDisp.dispGenericMode(pselected);
                }
                else if (seqPageParams.sqpage == 1) // SUBMODE_SEQ2
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "SOLO";
                    omxDisp.legends[1] = "LEN";
                    omxDisp.legends[2] = "RATE";
                    omxDisp.legends[3] = "CV";                                   // cvPattern
                    omxDisp.legendVals[0] = sequencer.getCurrentPattern()->solo; // playingPattern+1;
                    omxDisp.legendVals[1] = sequencer.getPatternLength(sequencer.playingPattern);
                    // omxDisp.legendVals[2] = -127;
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
                    omxDisp.dispPage = 2;

                    omxDisp.dispGenericMode(pselected);
                }
            }
            if (seqConfig.noteSelect)
            {
                int pselected = seqPageParams.nsparam % NUM_DISP_PARAMS;
                if (seqPageParams.nspage == 0) // SUBMODE_NOTESEL
                {
                    omxDisp.clearLegends();
                    omxDisp.legends[0] = "NOTE";
                    omxDisp.legends[1] = "OCT";
                    omxDisp.legends[2] = "VEL";
                    omxDisp.legends[3] = "LEN";
                    omxDisp.legendVals[0] = getSelectedStep()->note;
                    omxDisp.legendVals[1] = (int)midiSettings.octave + 4;
                    omxDisp.legendVals[2] = getSelectedStep()->vel;
                    omxDisp.legendVals[3] = getSelectedStep()->len + 1;
                    omxDisp.dispPage = 1;
                    omxDisp.dispGenericMode(pselected);
                }
                else if (seqPageParams.nspage == 1) // SUBMODE_NOTESEL2
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
                    omxDisp.dispPage = 2;
                    omxDisp.dispGenericMode(pselected);
                }
                else if (seqPageParams.nspage == 2) // SUBMODE_NOTESEL3
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
                    omxDisp.dispPage = 3;
                    omxDisp.dispGenericMode(pselected);
                }
            }
            if (seqPageParams.patternParams)
            {
                int pselected = seqPageParams.ppparam % NUM_DISP_PARAMS;
                if (seqPageParams.pppage == 0) // SUBMODE_PATTPARAMS
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
                    omxDisp.dispPage = 1;
                    omxDisp.dispGenericMode(pselected);
                }
                else if (seqPageParams.pppage == 1) // SUBMODE_PATTPARAMS2
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
                    omxDisp.dispPage = 2;
                    omxDisp.dispGenericMode(pselected);
                }
                else if (seqPageParams.pppage == 2) // SUBMODE_PATTPARAMS3
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
                    omxDisp.dispPage = 3;
                    omxDisp.dispGenericMode(pselected);
                }
            }
            if (seqConfig.stepRecord)
            {
                int pselected = seqPageParams.srparam % NUM_DISP_PARAMS;
                if (seqPageParams.srpage == 0) // SUBMODE_STEPREC
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
                    omxDisp.dispPage = 1;
                    omxDisp.dispGenericMode(pselected);
                }
                else if (seqPageParams.srpage == 1) // SUBMODE_NOTESEL2
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
                    omxDisp.dispPage = 2;
                    omxDisp.dispGenericMode(pselected);
                }
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

    StepNote stepNote = {0, 100, 0, TRIGTYPE_MUTE, {-1, -1, -1, -1, -1}, 100, 0, STEPTYPE_NONE};
    // {note, vel, len, TRIGTYPE, {params0, params1, params2, params3, params4}, prob, condition, STEPTYPE}

    for (int i = 0; i < NUM_PATTERNS; i++)
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
