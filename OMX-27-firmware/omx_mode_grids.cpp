#include "omx_mode_grids.h"
#include "config.h"
#include "colors.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"

using namespace grids;

void OmxModeGrids::InitSetup()
{
    initSetup = true;
}

void OmxModeGrids::onModeActivated()
{
    if(!initSetup){
        InitSetup();
    }
}

void OmxModeGrids::onClockTick() {
    grids_.gridsTick();
}

void OmxModeGrids::onPotChanged(int potIndex, int potValue)
{
    if (potIndex < 4)
    {
        grids_.setDensity(potIndex, potSettings.analogValues[potIndex] * 2);
    }
    else if (potIndex == 4)
    {
        int newres = (float(potSettings.analogValues[potIndex]) / 128.f) * 3;
        grids_.setResolution(newres);
    }
}

void OmxModeGrids::loopUpdate()
{
}

void OmxModeGrids::onEncoderChanged(Encoder::Update enc)
{
}

void OmxModeGrids::onEncoderButtonDown()
{
    
}

void OmxModeGrids::onEncoderButtonDownLong()
{
    
}

bool OmxModeGrids::shouldBlockEncEdit()
{
    return false;
}

void OmxModeGrids::onKeyUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();
    int keyPos = thisKey - 11;
    
}

void OmxModeGrids::onKeyHeldUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();

    
}

void OmxModeGrids::updateLEDs()
{
}

void OmxModeGrids::onDisplayUpdate()
{
    
}