#include "submode_midifxgroup.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "colors.h"
#include "midifx_randomizer.h"
#include "midifx_chance.h"

using namespace midifx;

enum MidiFxPage
{
    MFXPAGE_FX,
    MFXPAGE_TEST,
    MFXPAGE_EXIT
};

SubModeMidiFxGroup::SubModeMidiFxGroup()
{
    params_.addPage(4); // 4 Midi FX slots
    params_.addPage(3); // 3 test slots
    params_.addPage(1); // Exit submode

    for (uint8_t i = 0; i < 4; i++)
    {
        midifx_.push_back(nullptr);
    }

    doNoteOutput_ = &SubModeMidiFxGroup::noteFuncForwarder;
    doNoteOutputContext_ = this;
}

void SubModeMidiFxGroup::onEnabled()
{
    params_.setSelPageAndParam(0, 0);
    encoderSelect_ = true;
    omxLeds.setDirty();
    omxDisp.setDirty();
}

void SubModeMidiFxGroup::onDisabled()
{
    strip.clear();
    omxLeds.setDirty();
    omxDisp.setDirty();
}

void SubModeMidiFxGroup::loopUpdate()
{
}

void SubModeMidiFxGroup::updateLEDs()
{
    strip.clear();

    bool blinkState = omxLeds.getBlinkState();
    bool blinkStateSlow = omxLeds.getSlowBlinkState();

    // Serial.println("MidiFX Leds");
    auto auxColor = midiFXParamView_ ? (blinkStateSlow ? ORANGE : LEDOFF) : RED;
    strip.setPixelColor(0, auxColor);

    // for(uint8_t i = 1; i < 26; i++)
    // {
    //     strip.setPixelColor(i, LEDOFF);
    // }

    for(uint8_t i = 0; i < 4; i++)
    {
        auto fxColor = midiFXParamView_ ? (i == selectedMidiFX_ ? WHITE : ORANGE) : BLUE;

        strip.setPixelColor(11 + i, fxColor);
    }

    if (midiFXParamView_)
    {
        uint8_t selFXType = 0;

        if(getMidiFX(selectedMidiFX_) != nullptr)
        {
            // Serial.println("Selected MidiFX not null");
            selFXType = getMidiFX(selectedMidiFX_)->getFXType();
        }

        for (uint8_t i = 0; i < 8; i++)
        {
            auto fxColor = (i == selFXType ? GREEN : DKGREEN);

            strip.setPixelColor(19 + i, fxColor);
        }
    }
}

void SubModeMidiFxGroup::onEncoderChanged(Encoder::Update enc)
{
    if (midiFXParamView_)
    {
        if (getMidiFX(selectedMidiFX_) != nullptr)
        {
            getMidiFX(selectedMidiFX_)->onEncoderChanged(enc);
        }
    }
    else
    {
        SubmodeInterface::onEncoderChanged(enc);
    }
}

void SubModeMidiFxGroup::onEncoderChangedEditParam(Encoder::Update enc)
{
    omxDisp.setDirty();
    omxLeds.setDirty();
}

void SubModeMidiFxGroup::onEncoderButtonDown()
{
    if (midiFXParamView_)
    {
        if (getMidiFX(selectedMidiFX_) != nullptr)
        {
            getMidiFX(selectedMidiFX_)->onEncoderButtonDown();
        }
    }
    else
    {
        if (params_.getSelPage() == MFXPAGE_EXIT && params_.getSelParam() == 0)
        {
            setEnabled(false);
        }
        else
        {
            SubmodeInterface::onEncoderButtonDown();
        }
    }
    omxDisp.setDirty();
    omxLeds.setDirty();
}

void SubModeMidiFxGroup::onKeyUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();
	auto keyState = midiSettings.keyState;

    if(e.down())
    {
        if (thisKey == 0)
        {
            // Exit MidiFX view
            if(midiFXParamView_) 
            {
                midiFXParamView_ = false;
            }
            // Exit submode
            else 
            {
                setEnabled(false);
            }
        }

        // Quick Select FX Slot
        if (thisKey > 10 && thisKey < 15)
        {
            selectMidiFX(thisKey - 11);
        }

        // Change FX type
        if (midiFXParamView_)
        {
            if (thisKey >= 19 && thisKey < 19 + 8)
            {
                changeMidiFXType(selectedMidiFX_, thisKey - 19);
                // selectMidiFX(thisKey - 19);
            }
        }
    }

    omxDisp.setDirty();
    omxLeds.setDirty();
}

void SubModeMidiFxGroup::selectMidiFX(uint8_t fxIndex)
{
    midiFXParamView_ = true;
    selectedMidiFX_ = fxIndex;
}

midifx::MidiFXInterface *SubModeMidiFxGroup::getMidiFX(uint8_t index)
{
    return midifx_[index];

    // if (index == 0)
    //     return midiFX1_;
    // else if (index == 1)
    //     return midiFX2_;
    // else if (index == 2)
    //     return midiFX3_;
    // else if (index == 3)
    //     return midiFX4_;

    // return nullptr;
}

void SubModeMidiFxGroup::setMidiFX(uint8_t index, midifx::MidiFXInterface* midifx)
{
    midifx_[index] = midifx;

    // if (index == 0)
    //     midiFX1_ = midifx;
    // else if (index == 1)
    //     midiFX2_ = midifx;
    // else if (index == 2)
    //     midiFX3_ = midifx;
    // else if (index == 3)
    //     midiFX4_ = midifx;
}

void SubModeMidiFxGroup::changeMidiFXType(uint8_t slotIndex, uint8_t typeIndex)
{
    Serial.println(String("changeMidiFXType slotIndex: ") + String(slotIndex) + " typeIndex: " + String(typeIndex));

    if (!midiFXParamView_)
        return;

    if (typeIndex == midifxTypes_[slotIndex])
        return;

    // if (midifx_[slotIndex] != nullptr)
    // {
    //     delete midifx_[slotIndex];
    // }
    if(getMidiFX(slotIndex) != nullptr)
    {
        Serial.println("Deleting FX");

        midifx::MidiFXInterface* midifxptr = midifx_[slotIndex];

        midifx_[slotIndex] = nullptr;

        delete midifxptr;

        // if (slotIndex == 0)
        //     delete midiFX1_;
        // else if (slotIndex == 1)
        //     delete midiFX2_;
        // else if (slotIndex == 2)
        //     delete midiFX3_;
        // else if (slotIndex == 3)
        //     delete midiFX4_;
    }

    switch (typeIndex)
    {
    case MIDIFX_RANDOMIZER:
    {
        setMidiFX(slotIndex, new MidiFXRandomizer());
    }
    break;
    case MIDIFX_CHANCE:
    {
        setMidiFX(slotIndex, new MidiFXChance());

        // auto mfx = getMidiFX(slotIndex);
        // mfx->setNoteOutput(SubModeMidiFxGroup::noteFuncForwarder, this);

        // MidiNoteGroup testNote;
        // testNote.noteNumber = 10;
        // mfx->noteInput(testNote);
    }
        break;
    default:
        break;
    }

    if(getMidiFX(slotIndex) != nullptr)
    {
        omxDisp.displayMessageTimed(getMidiFX(slotIndex)->getName(), 5);
    }

    midifxTypes_[slotIndex] = typeIndex;

    reconnectInputsOutputs();
}

// Where the magic happens
void SubModeMidiFxGroup::reconnectInputsOutputs()
{
    Serial.println("SubModeMidiFxGroup::reconnectInputsOutputs");
    bool validMidiFXFound = false;
    midifx::MidiFXInterface* lastValidMidiFX = nullptr;

    for (int8_t i = 4 - 1; i >= 0; --i)
    {
        Serial.println("i = " + String(i));

        midifx::MidiFXInterface* fx = getMidiFX(i);

        if (fx == nullptr)
        {
            Serial.println("midifx is null");

            continue;
        }

        // Last valid MidiFX, connect it's output to the main midifxgroup output
        if (!validMidiFXFound)
        {
            Serial.println("connecting midifx to main output");

            fx->setNoteOutput(&SubModeMidiFxGroup::noteFuncForwarder, this);
            lastValidMidiFX = fx;
            validMidiFXFound = true;
        }
        // connect the output of this midiFX to the input of the next one
        else
        {
            if(lastValidMidiFX == nullptr)
            {
                Serial.println("lastValidMidiFX is null");
            }

            Serial.println("connecting midifx to previous midifx");

            fx->setNoteOutput(&MidiFXInterface::onNoteInputForwarder, lastValidMidiFX);
            lastValidMidiFX = fx;
        }
    }

    // Connect doNoteOutput_ to the lastValidMidiFX
    if (validMidiFXFound)
    {
        Serial.println("connecting group to lastValidMidiFX");

        doNoteOutput_ = &MidiFXInterface::onNoteInputForwarder;
        doNoteOutputContext_ = lastValidMidiFX;
    }
    // No valid midifx, connect groups input to it's output
    else
    {
        Serial.println("connecting group to self output");

        doNoteOutput_ = &SubModeMidiFxGroup::noteFuncForwarder;
        doNoteOutputContext_ = this;
    }
}

 void SubModeMidiFxGroup::noteInput(MidiNoteGroup note)
 {
    if(doNoteOutputContext_ == nullptr)
    {
        // bypass effects, sends out
        noteOutputFunc(note);
        return;
    }

    // Sends to connected function ptr
    doNoteOutput_(doNoteOutputContext_, note);
 }

// Sets function pointer to send notes out of FX Group
void SubModeMidiFxGroup::setNoteOutputFunc(void (*fptr)(void *, MidiNoteGroup), void *context)
{
    sendNoteOutFuncPtr_ = fptr;
    sendNoteOutFuncPtrContext_ = context;
}

void SubModeMidiFxGroup::noteOutputFunc(MidiNoteGroup note)
{
    Serial.println("SubModeMidiFxGroup::noteOutputFunc testNoteFunc: " + String(note.noteNumber) + " selectedMidiFX_: " + String(selectedMidiFX_));

    // Send the note out of FX group
    if(sendNoteOutFuncPtrContext_ == nullptr) return;
    sendNoteOutFuncPtr_(sendNoteOutFuncPtrContext_, note);
}



void SubModeMidiFxGroup::setupPageLegends()
{
    omxDisp.clearLegends();

    // omxDisp.dispPage = page + 1;

    int8_t page = params_.getSelPage();

    switch (page)
    {
    case MFXPAGE_FX:
    {
        omxDisp.legends[0] = "FX 1";
        omxDisp.legends[1] = "FX 2";
        omxDisp.legends[2] = "FX 3";
        omxDisp.legends[3] = "FX 4";
        omxDisp.legendVals[0] = -127;
        omxDisp.legendVals[1] = -127;
        omxDisp.legendVals[2] = -127;
        omxDisp.legendVals[3] = -127;
    }
    break;
    case MFXPAGE_TEST:
    {
        omxDisp.legends[0] = "Test";
        omxDisp.legends[1] = "Test";
        omxDisp.legends[2] = "Test";
        omxDisp.legends[3] = "";
        omxDisp.legendVals[0] = 1;
        omxDisp.legendVals[1] = 2;
        omxDisp.legendVals[2] = 3;
        omxDisp.legendVals[3] = -127;
        omxDisp.legendText[3] = "";
    }
    break;
    case MFXPAGE_EXIT:
    {
        omxDisp.legends[0] = "Exit";
        omxDisp.legends[1] = "";
        omxDisp.legends[2] = "";
        omxDisp.legends[3] = "";
        omxDisp.legendVals[0] = -127;
        omxDisp.legendVals[1] = -127;
        omxDisp.legendVals[2] = -127;
        omxDisp.legendVals[3] = -127;
        omxDisp.legendText[0] = "Exit";
        omxDisp.legendText[1] = "";
        omxDisp.legendText[2] = "";
        omxDisp.legendText[3] = "";
    }
    break;
    default:
        break;
    }
}

void SubModeMidiFxGroup::onDisplayUpdateMidiFX()
{
    MidiFXInterface* selFX = getMidiFX(selectedMidiFX_);

    if(selFX == nullptr)
    {
        omxDisp.displayMessage("No FX");
    }
    else
    {
        Serial.println("Selected MidiFX not null");

        selFX->onDisplayUpdate();
    }
}

void SubModeMidiFxGroup::onDisplayUpdate()
{
    omxLeds.updateBlinkStates();

    if (omxLeds.isDirty())
    {
        updateLEDs();
    }

    if (omxDisp.isDirty())
    { 
        if (!encoderConfig.enc_edit)
        {
            if (midiFXParamView_)
            {
                onDisplayUpdateMidiFX();
            }
            else
            {
                setupPageLegends();
                omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
            }
        }
    }
}