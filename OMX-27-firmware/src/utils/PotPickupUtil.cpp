#include "PotPickupUtil.h"

void PotPickupUtil::SetVal(uint8_t newValue, bool midiIn)
{
    value = newValue;
    if(midiIn)
    {
        revertValue = value;
    }
    directionDetermined = false;
    pickedUp = false;
}

void PotPickupUtil::SaveRevertVal()
{
    revertValue = value;
}

void PotPickupUtil::RevertVal()
{
    value = revertValue;
    directionDetermined = false;
    pickedUp = false;
}

void PotPickupUtil::UpdatePot(uint8_t prevPot, uint8_t newPot)
{
    if (!directionDetermined)
    {
        directionCW = prevPot < value;
        pickedUp = prevPot == value;
        directionDetermined = true;
    }

    if (!pickedUp)
    {
        if (directionCW)
        {
            pickedUp = newPot >= value;
        }
        else
        {
            pickedUp = newPot <= value;
        }
    }

    if (pickedUp)
    {
        value = newPot;
    }

    potValue = newPot;
}