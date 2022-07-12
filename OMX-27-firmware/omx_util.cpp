#include <U8g2_for_Adafruit_GFX.h>

#include "omx_util.h"
#include "consts.h"
#include "MM.h"


void OmxUtil::setup() {

}


// Pots
void OmxUtil::sendPots(int val, int channel){
    MM::sendControlChange(pots[potSettings.potbank][val], potSettings.analogValues[val], channel);
    potSettings.potCC = pots[potSettings.potbank][val];
    potSettings.potVal = potSettings.analogValues[val];
    potSettings.potValues[val] = potSettings.potVal;
}

OmxUtil omxUtil;