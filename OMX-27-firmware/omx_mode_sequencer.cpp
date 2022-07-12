#include "omx_mode_sequencer.h"
#include "config.h"
#include "colors.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "sequencer.h"
#include "omx_leds.h"

void OmxModeSequencer::InitSetup()
{
    initPatterns();
}

void OmxModeSequencer::OnPotChanged(int potIndex, int potValue)
{
    if (noteSelect && noteSelection)
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
    else if (stepRecord)
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
    else if (!noteSelect || !stepRecord)
    {
        omxUtil.sendPots(potIndex, sequencer.getPatternChannel(sequencer.playingPattern));
    }
}

void OmxModeSequencer::onEncoderChanged(Encoder::Update enc)
{
    if (!noteSelect && !patternParams && !stepRecord)
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
    if (sqparam == 0 || sqparam == 5)
    {
        sqpage = constrain(sqpage + amt, 0, 1);
        sqparam = sqpage * NUM_DISP_PARAMS;
    }

    // PAGE ONE
    if (sqparam == 1)
    {
        sequencer.playingPattern = constrain(sequencer.playingPattern + amt, 0, 7);
        if (sequencer.getCurrentPattern()->solo)
        {
            setAllLEDS(0, 0, 0);
        }
    }
    else if (sqparam == 2)
    {                                                // SET TRANSPOSE
        transposeSeq(sequencer.playingPattern, amt); //
        int newtransp = constrain(transpose + amt, -64, 63);
        transpose = newtransp;
    }
    else if (sqparam == 3)
    {                                                                                          // SET SWING
        int newswing = constrain(sequencer.getCurrentPattern()->swing + amt, 0, maxswing - 1); // -1 to deal with display values
        swing = newswing;
        sequencer.getCurrentPattern()->swing = newswing;
        //	setGlobalSwing(newswing);
    }
    else if (sqparam == 4)
    { // SET TEMPO
        newtempo = constrain(clockbpm + amt, 40, 300);
        if (newtempo != clockbpm)
        {
            // SET TEMPO HERE
            clockbpm = newtempo;
            resetClocks();
        }
    }

    // PAGE TWO
    if (sqparam == 6)
    { //  MIDI SOLO
        //						playingPattern = constrain(playingPattern + amt, 0, 7);
        sequencer.getCurrentPattern()->solo = constrain(sequencer.getCurrentPattern()->solo + amt, 0, 1);
        if (sequencer.getCurrentPattern()->solo)
        {
            setAllLEDS(0, 0, 0);
        }
    }
    else if (sqparam == 7)
    { // SET PATTERN LENGTH
        auto newPatternLen = constrain(sequencer.getPatternLength(sequencer.playingPattern) + amt, 1, NUM_STEPS);
        sequencer.setPatternLength(sequencer.playingPattern, newPatternLen);
        if (sequencer.seqPos[sequencer.playingPattern] >= newPatternLen)
        {
            sequencer.seqPos[sequencer.playingPattern] = newPatternLen - 1;
            sequencer.patternPage[sequencer.playingPattern] = getPatternPage(sequencer.seqPos[sequencer.playingPattern]);
        }
    }
    else if (sqparam == 8)
    { // SET CLOCK DIV/MULT
        sequencer.getCurrentPattern()->clockDivMultP = constrain(sequencer.getCurrentPattern()->clockDivMultP + amt, 0, NUM_MULTDIVS - 1);
    }
    else if (sqparam == 9)
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
    if (patternParams && !encoderConfig.enc_edit)
    { // SEQUENCE PATTERN PARAMS SUB MODE
        // CHANGE PAGE
        if (ppparam == 0 || ppparam == 5 || ppparam == 10)
        {
            pppage = constrain(pppage + amt, 0, 2); // HARDCODED - FIX WITH SIZE OF PAGES?
            ppparam = pppage * NUM_DISP_PARAMS;
        }

        // PAGE ONE
        if (ppparam == 1)
        { // SET PLAYING PATTERN
            sequencer.playingPattern = constrain(sequencer.playingPattern + amt, 0, 7);
        }
        if (ppparam == 2)
        { // SET LENGTH
            auto newPatternLen = constrain(sequencer.getPatternLength(sequencer.playingPattern) + amt, 1, NUM_STEPS);
            sequencer.setPatternLength(sequencer.playingPattern, newPatternLen);
            if (sequencer.seqPos[sequencer.playingPattern] >= newPatternLen)
            {
                sequencer.seqPos[sequencer.playingPattern] = newPatternLen - 1;
                sequencer.patternPage[sequencer.playingPattern] = getPatternPage(sequencer.seqPos[sequencer.playingPattern]);
            }
        }
        if (ppparam == 3)
        { // SET PATTERN ROTATION
            int rotator;
            (u.dir() < 0 ? rotator = -1 : rotator = 1);
            //							int rotator = constrain(rotcc, (sequencer.PatternLength(sequencer.playingPattern))*-1, sequencer.PatternLength(sequencer.playingPattern));
            rotationAmt = rotationAmt + rotator;
            if (rotationAmt < 16 && rotationAmt > -16)
            { // NUM_STEPS??
                rotatePattern(sequencer.playingPattern, rotator);
            }
            rotationAmt = constrain(rotationAmt, (sequencer.getPatternLength(sequencer.playingPattern) - 1) * -1, sequencer.getPatternLength(sequencer.playingPattern) - 1);
        }

        if (ppparam == 4)
        { // SET PATTERN CHANNEL
            sequencer.getCurrentPattern()->channel = constrain(sequencer.getCurrentPattern()->channel + amt, 0, 15);
        }
        // PATTERN PARAMS PAGE 2
        // TODO: convert to case statement ??
        if (ppparam == 6)
        { // SET AUTO START STEP
            sequencer.getCurrentPattern()->startstep = constrain(sequencer.getCurrentPattern()->startstep + amt, 0, sequencer.getCurrentPattern()->len);
            // sequencer.getCurrentPattern()->startstep--;
        }
        if (ppparam == 7)
        { // SET AUTO RESET STEP
            int tempresetstep = sequencer.getCurrentPattern()->autoresetstep + amt;
            sequencer.getCurrentPattern()->autoresetstep = constrain(tempresetstep, 0, sequencer.getCurrentPattern()->len + 1);
        }
        if (ppparam == 8)
        {                                                                                                                        // SET AUTO RESET FREQUENCY
            sequencer.getCurrentPattern()->autoresetfreq = constrain(sequencer.getCurrentPattern()->autoresetfreq + amt, 0, 15); // max every 16 times
        }
        if (ppparam == 9)
        {                                                                                                                         // SET AUTO RESET PROB
            sequencer.getCurrentPattern()->autoresetprob = constrain(sequencer.getCurrentPattern()->autoresetprob + amt, 0, 100); // never, 100% - 33%
        }

        // PAGE THREE
        if (ppparam == 11)
        {                                                                                                                                      // SET CLOCK-DIV-MULT
            sequencer.getCurrentPattern()->clockDivMultP = constrain(sequencer.getCurrentPattern()->clockDivMultP + amt, 0, NUM_MULTDIVS - 1); // set clock div/mult
        }
        if (ppparam == 12)
        { // SET MIDI SOLO
            sequencer.getCurrentPattern()->solo = constrain(sequencer.getCurrentPattern()->solo + amt, 0, 1);
        }

        // STEP RECORD SUB MODE
    }
    else if (stepRecord && !enc_edit)
    {
        // CHANGE PAGE
        if (srparam == 0 || srparam == 5)
        {
            srpage = constrain(srpage + amt, 0, 1); // HARDCODED - FIX WITH SIZE OF PAGES?
            srparam = srpage * NUM_DISP_PARAMS;
        }

        // PAGE ONE
        if (srparam == 1)
        { // OCTAVE SELECTION
            newoctave = constrain(octave + amt, -5, 4);
            if (newoctave != octave)
            {
                octave = newoctave;
            }
        }
        if (srparam == 2)
        { // STEP SELECTION
            if (u.dir() > 0)
            {
                step_ahead();
            }
            else if (u.dir() < 0)
            {
                step_back();
            }
            selectedStep = sequencer.seqPos[sequencer.playingPattern];
        }
        if (srparam == 3)
        { // SET NOTE NUM
            int tempNote = getSelectedStep()->note;
            getSelectedStep()->note = constrain(tempNote + amt, 0, 127);
        }
        if (srparam == 4)
        {
            //							playingPattern = constrain(playingPattern + amt, 0, 7);
        }
        // PAGE TWO
        if (srparam == 6)
        { // STEP TYPE
            changeStepType(amt);
        }
        if (srparam == 7)
        { // STEP PROB
            int tempProb = getSelectedStep()->prob;
            getSelectedStep()->prob = constrain(tempProb + amt, 0, 100); // Note Len between 1-16
        }
        if (srparam == 8)
        { // STEP CONDITION
            int tempCondition = getSelectedStep()->condition;
            getSelectedStep()->condition = constrain(tempCondition + amt, 0, 35); // 0-32
        }

        // NOTE SELECT MODE
    }
    else if (noteSelect && noteSelection && !enc_edit)
    {
        // CHANGE PAGE
        if (nsparam == 0 || nsparam == 5 || nsparam == 10)
        {
            nspage = constrain(nspage + amt, 0, 2); // HARDCODED - FIX WITH SIZE OF PAGES?
            nsparam = nspage * NUM_DISP_PARAMS;
        }

        // PAGE THREE
        if (nsparam > 10 && nsparam < 14)
        {
            if (u.dir() < 0)
            { // RESET PLOCK IF TURN CCW
                int tempmode = nsparam - 11;
                getSelectedStep()->params[tempmode] = -1;
            }
        }
        // PAGE ONE
        if (nsparam == 1)
        { // SET NOTE NUM
            int tempNote = getSelectedStep()->note;
            getSelectedStep()->note = constrain(tempNote + amt, 0, 127);
        }
        if (nsparam == 2)
        { // SET OCTAVE
            newoctave = constrain(octave + amt, -5, 4);
            if (newoctave != octave)
            {
                octave = newoctave;
            }
        }
        if (nsparam == 3)
        { // SET VELOCITY
            int tempVel = getSelectedStep()->vel;
            getSelectedStep()->vel = constrain(tempVel + amt, 0, 127);
        }
        if (nsparam == 4)
        { // SET NOTE LENGTH
            int tempLen = getSelectedStep()->len;
            getSelectedStep()->len = constrain(tempLen + amt, 0, 15); // Note Len between 1-16
        }
        // PAGE TWO
        if (nsparam == 6)
        { // SET STEP TYPE
            changeStepType(amt);
        }
        if (nsparam == 7)
        { // SET STEP PROB
            int tempProb = getSelectedStep()->prob;
            getSelectedStep()->prob = constrain(tempProb + amt, 0, 100); // Note Len between 1-16
        }
        if (nsparam == 8)
        { // SET STEP TRIG CONDITION
            int tempCondition = getSelectedStep()->condition;
            getSelectedStep()->condition = constrain(tempCondition + amt, 0, 35); // 0-32
        }
    }
    else
    {
        newtempo = constrain(clockbpm + amt, 40, 300);
        if (newtempo != clockbpm)
        {
            // SET TEMPO HERE
            clockbpm = newtempo;
            resetClocks();
        }
    }
    dirtyDisplay = true;
}

void OmxModeSequencer::onEncoderButtonDown()
{
    if (noteSelect && noteSelection && !patternParams)
    {
        nsparam = (nsparam + 1) % 15;
        if (nsparam > 9)
        {
            nspage = 2;
        }
        else if (nsparam > 4)
        {
            nspage = 1;
        }
        else
        {
            nspage = 0;
        }
    }
    else if (patternParams)
    {
        ppparam = (ppparam + 1) % 15;
        if (ppparam > 9)
        {
            pppage = 2;
        }
        else if (ppparam > 4)
        {
            pppage = 1;
        }
        else
        {
            pppage = 0;
        }
    }
    else if (stepRecord)
    {
        srparam = (srparam + 1) % 10;
        if (srparam > 4)
        {
            srpage = 1;
        }
        else
        {
            srpage = 0;
        }
    }
    else
    {
        sqparam = (sqparam + 1) % 10;
        if (sqparam > 4)
        {
            sqpage = 1;
        }
        else
        {
            sqpage = 0;
        }
    }
}

void OmxModeSequencer::onEncoderButtonDownLong()
{
    if (stepRecord)
    {
        resetPatternDefaults(sequencer.playingPattern);
        clearedFlag = true;
    }
}

bool OmxModeSequencer::shouldBlockEncEdit()
{
    return stepRecord;
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
        if (noteSelect)
        {
            if (noteSelection)
            { // SET NOTE
                // left and right keys change the octave
                if (thisKey == 11 || thisKey == 26)
                {
                    int amt = thisKey == 11 ? -1 : 1;
                    newoctave = constrain(octave + amt, -5, 4);
                    if (newoctave != octave)
                    {
                        octave = newoctave;
                    }
                    // otherwise select the note
                }
                else
                {
                    stepSelect = false;
                    selectedNote = thisKey;
                    int adjnote = notes[thisKey] + (octave * 12);
                    getSelectedStep()->note = adjnote;
                    if (!sequencer.playing)
                    {
                        seqNoteOn(thisKey, defaultVelocity, sequencer.playingPattern);
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
                selectedStep = seqKey; // was keyPos // set noteSelection to this step
                stepSelect = true;
                noteSelection = true;
                omxDisp.setDirty();
            }

            // PATTERN PARAMS
        }
        else if (patternParams)
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
                if (keyState[1] && !keyState[2])
                {
                    copyPattern(sequencer.playingPattern);
                    displayMessagef("COPIED P-%d", sequencer.playingPattern + 1);
                }
                else if (!keyState[1] && keyState[2])
                {
                    pastePattern(sequencer.playingPattern);
                    displayMessagef("PASTED P-%d", sequencer.playingPattern + 1);
                }
                else if (keyState[1] && keyState[2])
                {
                    clearPattern(sequencer.playingPattern);
                    displayMessagef("CLEARED P-%d", sequencer.playingPattern + 1);
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
        else if (stepRecord)
        {
            selectedNote = thisKey;
            selectedStep = sequencer.seqPos[sequencer.playingPattern];

            int adjnote = notes[thisKey] + (octave * 12);
            getSelectedStep()->note = adjnote;

            if (!sequencer.playing)
            {
                seqNoteOn(thisKey, defaultVelocity, sequencer.playingPattern);
            } // see RELEASE events for more
            stepDirty = true;
            omxDisp.setDirty();

            // MIDI SOLO
        }
        else if (sequencer.getCurrentPattern()->solo)
        {
            midiNoteOn(thisKey, defaultVelocity, sequencer.getCurrentPattern()->channel + 1);

            // REGULAR SEQ MODE
        }
        else
        {
            if (keyState[1] && keyState[2])
            {
                seqPages = true;
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
                if (keyState[1] && !keyState[2] && !sequencer.playing)
                {
                    sequencer.playingPattern = thisKey - 3;
                    sequencer.seqPos[sequencer.playingPattern] = 0;
                    sequencer.patternPage[sequencer.playingPattern] = 0; // Step Record always starts from first page
                    stepRecord = true;
                    displayMessagef("STEP RECORD");
                    //								omxDisp.setDirty();;

                    // If KEY 2 is down + pattern = PATTERN MUTE
                }
                else if (keyState[2])
                {
                    if (sequencer.getPattern(thisKey - 3)->mute)
                    {
                        displayMessagef("UNMUTE P-%d", (thisKey - 3) + 1);
                    }
                    else
                    {
                        displayMessagef("MUTE P-%d", (thisKey - 3) + 1);
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

                if (keyState[1] && keyState[2])
                { // F1+F2 HOLD
                    if (!stepRecord)
                    { // IGNORE LONG PRESSES IN STEP RECORD
                        if (keyPos <= getPatternPage(sequencer.getCurrentPattern()->len))
                        {
                            sequencer.patternPage[sequencer.playingPattern] = keyPos;
                        }
                        displayMessagef("PATT PAGE %d", keyPos + 1);
                    }
                }
                else if (keyState[1])
                { // F1 HOLD
                    if (!stepRecord && !patternParams)
                    {                                // IGNORE LONG PRESSES IN STEP RECORD and Pattern Params
                        selectedStep = thisKey - 11; // set noteSelection to this step
                        noteSelect = true;
                        stepSelect = true;
                        noteSelection = true;
                        omxDisp.setDirty();
                        displayMessagef("NOTE SELECT");
                        // re-toggle the key you just held
                        //										if (getSelectedStep()->trig == TRIGTYPE_PLAY || getSelectedStep()->trig == TRIGTYPE_MUTE ) {
                        //											getSelectedStep()->trig = (getSelectedStep()->trig == TRIGTYPE_PLAY ) ? TRIGTYPE_MUTE : TRIGTYPE_PLAY;
                        //										}
                    }
                }
                else if (keyState[2])
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
            midiNoteOff(thisKey, sequencer.getCurrentPattern()->channel + 1);
        }
    }

    if (!e.down() && thisKey != 0 && (noteSelection || stepRecord) && selectedNote > 0)
    {
        if (!sequencer.playing)
        {
            seqNoteOff(thisKey, sequencer.playingPattern);
        }
        if (stepRecord && stepDirty)
        {
            step_ahead();
            stepDirty = false;
            // EXIT STEP RECORD AFTER THE LAST STEP IN PATTERN
            if (sequencer.seqPos[sequencer.playingPattern] == 0)
            {
                stepRecord = false;
            }
        }
    }

    // AUX KEY PRESS EVENTS

    if (e.down() && thisKey == 0)
    {

        if (noteSelect)
        {
            if (noteSelection)
            {
                selectedStep = 0;
                selectedNote = 0;
            }
            else
            {
            }
            noteSelection = false;
            noteSelect = !noteSelect;
            omxDisp.setDirty();
        }
        else if (patternParams)
        {
            patternParams = !patternParams;
            omxDisp.setDirty();
        }
        else if (stepRecord)
        {
            stepRecord = !stepRecord;
            omxDisp.setDirty();
        }
        else if (seqPages)
        {
            seqPages = false;
        }
        else
        {
            if (keyState[1] || keyState[2])
            { // CHECK keyState[] FOR LONG PRESS OF FUNC KEYS
                if (keyState[1])
                {
                    sequencer.seqResetFlag = true; // RESET ALL SEQUENCES TO FIRST/LAST STEP
                    displayMessagef("RESET");
                }
                else if (keyState[2])
                { // CHANGE PATTERN DIRECTION
                    sequencer.getCurrentPattern()->reverse = !sequencer.getCurrentPattern()->reverse;
                    if (sequencer.getCurrentPattern()->reverse)
                    {
                        displayMessagef("<< REV");
                    }
                    else
                    {
                        displayMessagef("FWD >>");
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
    if (!sequencer.getCurrentPattern()->solo)
    {
        // TODO: access key state directly in omx_keypad.h
        if (keyState[1] && keyState[2])
        {
            seqPages = true;
        }
        else if (!keyState[1] && !keyState[2])
        { // SKIP LONG PRESS IF FUNC KEYS ARE ALREDY HELD
            if (thisKey > 2 && thisKey < 11)
            { // skip AUX key, get pattern keys
                patternParams = true;
                omxDisp.setDirty();
                displayMessagef("PATT PARAMS");
            }
            else if (thisKey > 10)
            {
                if (!stepRecord && !patternParams)
                {                                                                                                     // IGNORE LONG PRESSES IN STEP RECORD and Pattern Params
                    selectedStep = (thisKey - 11) + (sequencer.patternPage[sequencer.playingPattern] * NUM_STEPKEYS); // set noteSelection to this step
                    noteSelect = true;
                    stepSelect = true;
                    noteSelection = true;
                    omxDisp.setDirty();
                    displayMessagef("NOTE SELECT");
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
    else if (noteSelect && blinkState)
    {
        strip.setPixelColor(0, NOTESEL);
    }
    else if (patternParams && blinkState)
    {
        strip.setPixelColor(0, seqColors[patternNum]);
    }
    else if (stepRecord && blinkState)
    {
        strip.setPixelColor(0, seqColors[patternNum]);
    }
    else
    {
        switch (sysSettings.omxMode)
        {
        case MODE_S1:
            strip.setPixelColor(0, SEQ1C);
            break;
        case MODE_S2:
            strip.setPixelColor(0, SEQ2C);
            break;
        default:
            strip.setPixelColor(0, LEDOFF);
            break;
        }
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

    if (noteSelect && noteSelection)
    {
        // 27 LEDS so use LED_COUNT
        for (int j = 1; j < LED_COUNT; j++)
        {
            auto pixelpos = j;
            auto selectedStepPixel = (selectedStep % NUM_STEPKEYS) + 11;

            if (pixelpos == selectedNote)
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
    else if (stepRecord)
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
            else if (pixelpos != selectedNote)
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
    else if (seqPages)
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
                    if (patternParams && blinkState)
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
                    else if (!patternParams && sequencer.patterns[patternNum].steps[i].trig == TRIGTYPE_MUTE)
                    {
                        strip.setPixelColor(pixelpos, LEDOFF); // DO WE NEED TO MARK PLAYHEAD WHEN STOPPED?
                    }
                    else if (patternParams)
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
                else if (!patternParams && steps[i].trig == TRIGTYPE_MUTE)
                {
                    strip.setPixelColor(pixelpos, LEDOFF);
                }
                else if (patternParams)
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
        midi_leds();
    }
    if (dirtyDisplay)
    { // DISPLAY
        if (!encoderConfig.enc_edit && messageTextTimer == 0)
        { // show only if not encoder edit or dialog display
            if (!noteSelect and !patternParams and !stepRecord)
            {
                int pselected = sqparam % NUM_DISP_PARAMS;
                if (sqpage == 0)
                {
                    dispGenericMode(SUBMODE_SEQ, pselected);
                }
                else if (sqpage == 1)
                {
                    dispGenericMode(SUBMODE_SEQ2, pselected);
                }
            }
            if (noteSelect)
            {
                int pselected = nsparam % NUM_DISP_PARAMS;
                if (nspage == 0)
                {
                    dispGenericMode(SUBMODE_NOTESEL, pselected);
                }
                else if (nspage == 1)
                {
                    dispGenericMode(SUBMODE_NOTESEL2, pselected);
                }
                else if (nspage == 2)
                {
                    dispGenericMode(SUBMODE_NOTESEL3, pselected);
                }
            }
            if (patternParams)
            {
                int pselected = ppparam % NUM_DISP_PARAMS;
                if (pppage == 0)
                {
                    dispGenericMode(SUBMODE_PATTPARAMS, pselected);
                }
                else if (pppage == 1)
                {
                    dispGenericMode(SUBMODE_PATTPARAMS2, pselected);
                }
                else if (pppage == 2)
                {
                    dispGenericMode(SUBMODE_PATTPARAMS3, pselected);
                }
            }
            if (stepRecord)
            {
                int pselected = srparam % NUM_DISP_PARAMS;
                if (srpage == 0)
                {
                    dispGenericMode(SUBMODE_STEPREC, pselected);
                }
                else if (srpage == 1)
                {
                    dispGenericMode(SUBMODE_NOTESEL2, pselected);
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
