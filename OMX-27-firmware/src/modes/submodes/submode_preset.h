#pragma once

#include "submode_interface.h"

enum SubmodePresetMode
{
	PRESETMODE_LOAD,
	PRESETMODE_SAVE
};

// Submode for saving and loading a drum kit
class SubModePreset : public SubmodeInterface
{
public:
	// Constructor / deconstructor
	SubModePreset();
	~SubModePreset() {}

	void configure(SubmodePresetMode mode, uint8_t selPreset, uint8_t numPresets, bool autoSave);

	void setContextPtr(void *context);
	void setDoSaveFunc(void (*fptr)(void *, uint8_t));
	void setDoLoadFunc(void (*fptr)(void *, uint8_t));

	// Interface methods
	void loopUpdate() override;
	bool updateLEDs() override;
	void onEncoderChanged(Encoder::Update enc);
	void onEncoderButtonDown() override;
	bool onKeyUpdate(OMXKeypadEvent e) override;
	void onDisplayUpdate() override;

	bool shouldBlockEncEdit() override { return true; }

protected:
	// Interface methods
	void onEnabled() override;
	void onDisabled() override;
	void onEncoderChangedEditParam(Encoder::Update enc) override;

private:
	

	SubmodePresetMode mode;
	uint8_t selPreset;
	uint8_t numPresets;

	void *fptrContext_;
	void (*doSaveFptr_)(void *, uint8_t);
	void (*doLoadFptr_)(void *, uint8_t);

	bool autoSave;

	bool auxReleased_ = false; // set to aux state onEnable, must be true to exit mode with aux.

	void doSave(uint8_t presetIndex);
	void doLoad(uint8_t presetIndex);

	// void setupPageLegends();
};
