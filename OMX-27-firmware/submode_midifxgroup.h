#pragma once

#include "submode_interface.h"

class SubModeMidiFxGroup : public SubmodeInterface
{
public:
// Constructor / deconstructor
    SubModeMidiFxGroup();
    ~SubModeMidiFxGroup() {}

// Interface methods
    void loopUpdate() override;
    void updateLEDs() override;
    void onEncoderButtonDown() override;
    void onKeyUpdate(OMXKeypadEvent e) override;
    void onDisplayUpdate() override;
protected:
// Interface methods
    void onEnabled() override;
    void onDisabled() override;
    void onEncoderChangedEditParam(Encoder::Update enc) override;

private:
    void setupPageLegends();
};