#pragma once
#include "../config.h"

class PotPickupUtil
{
public:
    uint8_t revertValue;
    uint8_t value;
    uint8_t potValue;

    bool directionDetermined;
    bool directionCW;
    bool pickedUp;

    // set midiIn true if value is coming from midi
    void SetVal(uint8_t newValue, bool midiIn);
    // saves current value to the revert value
    void SaveRevertVal();
    // Reverts the current value to the saved revert value
    // Which gets saved from midiin or if SaveRevertVal() is called
    void RevertVal();
    void UpdatePot(uint8_t prevPot, uint8_t newPot);
};