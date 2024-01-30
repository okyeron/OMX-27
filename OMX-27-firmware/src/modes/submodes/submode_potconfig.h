#pragma once

#include "submode_interface.h"

// Holds a group of 4 midi fx slots.
class SubModePotConfig : public SubmodeInterface
{
public:
	// Constructor / deconstructor
	SubModePotConfig();
	~SubModePotConfig() {}

	// Interface methods
	void loopUpdate() override;
	bool updateLEDs() override;
	void onEncoderChanged(Encoder::Update enc);
	void onEncoderButtonDown() override;
	bool onKeyUpdate(OMXKeypadEvent e) override;
	void onDisplayUpdate() override;

protected:
	// Interface methods
	void onEnabled() override;
	void onDisabled() override;
	void onEncoderChangedEditParam(Encoder::Update enc) override;

private:
	bool auxReleased_ = false; // set to aux state onEnable, must be true to exit mode with aux.
	// int bankIndex = 0;

	void setupPageLegends();
};
