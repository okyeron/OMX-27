#pragma once
#include "config.h"

class OmxUtil
{
public:
    OmxUtil(){
        potCC = pots[potbank][0];
    }

    void setup();

    void sendPots(int val, int channel);

    void setSubmode(int submode);

private:
    int potbank = 0;
    int analogValues[5] = {0,0,0,0,0};		// default values
    int potValues[5] = {0,0,0,0,0};
    int potCC = pots[potbank][0];
    int potVal = analogValues[0];
};

extern OmxUtil omxutil;