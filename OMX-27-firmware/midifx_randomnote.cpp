#include "midifx_randomnote.h"
#include "omx_disp.h"

namespace midifx
{
    enum RNotePage {
        RNPAGE_1,
        RNPAGE_2
    };

    MidiFXRandomNote::MidiFXRandomNote()
    {
        params_.addPage(4);
        params_.addPage(4);
        encoderSelect_ = true;
    }

    int MidiFXRandomNote::getFXType()
    {
        return MIDIFX_RANDNOTE;
    }

    String MidiFXRandomNote::getName()
    {
        return String("Rand Note");
    }

    void MidiFXRandomNote::onEnabled()
    {
    }

    void MidiFXRandomNote::onDisabled()
    {
    }

    void MidiFXRandomNote::noteInput(midifxnote note)
    {
    }

    void MidiFXRandomNote::loopUpdate()
    {
    }

    void MidiFXRandomNote::onEncoderChangedEditParam(Encoder::Update enc)
    {
    }

    void MidiFXRandomNote::onDisplayUpdate()
    {
        omxDisp.clearLegends();

        // omxDisp.dispPage = page + 1;

        int8_t page = params_.getSelPage();

        switch (page)
        {
        case RNPAGE_1:
        {
            omxDisp.legends[0] = "LOW";
            omxDisp.legends[1] = "HIGH";
            omxDisp.legends[2] = "";
            omxDisp.legends[3] = "";
            omxDisp.legendVals[0] = -127;
            omxDisp.legendVals[1] = -127;
            omxDisp.legendVals[2] = -127;
            omxDisp.legendVals[3] = -127;
            omxDisp.legendText[0] = "C3";
            omxDisp.legendText[1] = "C4";
        }
        break;
        case RNPAGE_2:
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
        default:
            break;
        }

        omxDisp.dispGenericMode2(params_.getNumPages(), params_.getSelPage(), params_.getSelParam(), encoderSelect_);
    }
}