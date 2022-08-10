#include "midimacro_norns.h"
#include "omx_disp.h"

namespace midimacro
{
    enum NornsPage {
        CHPAGE_1
    };

    MidiMacroNorns::MidiMacroNorns()
    {
        params_.addPage(4);
        encoderSelect_ = true;
    }

    String MidiMacroNorns::getName()
    {
        return String("Chance");
    }

    void MidiMacroNorns::onEnabled()
    {
    }

    void MidiMacroNorns::onDisabled()
    {
    }
   
    void MidiMacroNorns::loopUpdate()
    {
    }

    void MidiMacroNorns::onPotChanged(int potIndex, int prevValue, int newValue, int analogDelta)
    {

    }

    void MidiMacroNorns::onKeyUpdate(OMXKeypadEvent e)
    {
    }

    void MidiMacroNorns::drawLEDs()
    {

    }

    void MidiMacroNorns::onEncoderChangedEditParam(Encoder::Update enc)
    {
        int8_t page = params_.getSelPage();
        int8_t param = params_.getSelParam();

        auto amt = enc.accel(5); 

        if(page == CHPAGE_1)
        {
            if (param == 0)
            {
                // chancePerc_ = constrain(chancePerc_ + amt, 0, 255);
            }
        }
        omxDisp.setDirty();
    }

    void MidiMacroNorns::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        int8_t page = params_.getSelPage();

        switch (page)
        {
        case CHPAGE_1:
        {
            // omxDisp.legends[0] = "PERC";
            // omxDisp.legends[1] = "";
            // omxDisp.legends[2] = "";
            // omxDisp.legends[3] = "";
            // omxDisp.legendVals[0] = -127;
            // omxDisp.legendVals[1] = -127;
            // omxDisp.legendVals[2] = -127;
            // omxDisp.legendVals[3] = -127;

            // uint8_t perc = ((chancePerc_ / 255.0f) * 100);
            // String msg = String(perc) + "%";
            // omxDisp.legendText[0] = msg.c_str();
        }
        break;
        default:
            break;
        }

        omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
    }
}