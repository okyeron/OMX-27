#include "submode_midifxgroup.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "colors.h"

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
}

void SubModeMidiFxGroup::loopUpdate()
{
}

void SubModeMidiFxGroup::updateLEDs()
{
    omxLeds.updateBlinkStates();

    bool blinkState = omxLeds.getBlinkState();

    Serial.println("MidiFX Leds");
    strip.setPixelColor(0, RED);

    for(uint8_t i = 1; i < 26; i++)
    {
        strip.setPixelColor(i, LEDOFF);
    }
}

void SubModeMidiFxGroup::onEncoderChangedEditParam(Encoder::Update enc)
{
    omxDisp.setDirty();
    omxLeds.setDirty();
}

void SubModeMidiFxGroup::onEncoderButtonDown()
{
    if (params_.getSelPage() == MFXPAGE_EXIT && params_.getSelParam() == 0)
    {
        setEnabled(false);
    }
    else
    {
        encoderSelect_ = !encoderSelect_;
        omxDisp.setDirty();
    }
    omxDisp.setDirty();
    omxLeds.setDirty();
}

void SubModeMidiFxGroup::onKeyUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();
	auto keyState = midiSettings.keyState;

    if(e.down() && thisKey == 0){
        setEnabled(false);
    }

    omxDisp.setDirty();
    omxLeds.setDirty();
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

void SubModeMidiFxGroup::onDisplayUpdate()
{
    if (omxLeds.isDirty())
    {
        updateLEDs();
    }

    if (omxDisp.isDirty())
    { 
        if (!encoderConfig.enc_edit)
        {
            setupPageLegends();
            omxDisp.dispGenericMode2(3, params_.getSelPage(), params_.getSelParam(), encoderSelect_);
        }
    }
}