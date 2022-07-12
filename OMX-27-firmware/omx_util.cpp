#include <U8g2_for_Adafruit_GFX.h>

#include "omx_util.h"
#include "consts.h"
#include "MM.h"


U8G2_FOR_ADAFRUIT_GFX u8g2_display;


void OmxUtil::setup() {

}


// Pots
void OmxUtil::sendPots(int val, int channel){
    MM::sendControlChange(pots[potbank][val], analogValues[val], channel);
    potCC = pots[potbank][val];
    potVal = analogValues[val];
    potValues[val] = potVal;
}

OmxUtil omxutil;