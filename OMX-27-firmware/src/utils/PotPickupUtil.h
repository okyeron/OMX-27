#pragma once
#include "../config.h"

class PotPickupUtil
{
public:
    uint8_t value;

    bool directionDetermined;
    bool directionCW;
    bool pickedUp;

    void SetVal(uint8_t newValue);
    void UpdatePot(uint8_t prevPot, uint8_t newPot);
};