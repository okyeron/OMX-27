#pragma once

#include "submode_interface.h"
#include "../../hardware/storage.h"

// Submode for saving and loading a drum kit
class SubModeClearStorage : public SubmodeInterface
{
public:
	// Constructor / deconstructor
	SubModeClearStorage();
	~SubModeClearStorage() {}

	void setStoragePtr(Storage *storagePtr);

	// Interface methods
	void loopUpdate() override;
	bool updateLEDs() override;
	void onEncoderChanged(Encoder::Update enc) override;
	void onEncoderButtonDown() override;
	bool onKeyUpdate(OMXKeypadEvent e) override;
	void onDisplayUpdate() override;

	bool shouldBlockEncEdit() override { return true; }
	bool usesPots() override { return true; }
protected:
	// Interface methods
	void onEnabled() override;
	void onDisabled() override;
	void onEncoderChangedEditParam(Encoder::Update enc) override;

private:
    uint8_t state;
	bool auxReleased_ = false; // set to aux state onEnable, must be true to exit mode with aux.
    
    Storage *storagePtr;
    
    void eraseStorage();
};
