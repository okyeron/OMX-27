#pragma once

#include "submode_interface.h"
#include "../../midifx/midifx_interface.h"
#include "../../hardware/storage.h"
#include "../../midifx/midifx_arpeggiator.h"
#include "../../midifx/midifx_selector.h"

#define NUM_MIDIFX_GROUPS 5
#define NUM_MIDIFX_SLOTS 8

// Holds a group of 4 midi fx slots.
class SubModeMidiFxGroup : public SubmodeInterface
{
public:
	// Constructor / deconstructor
	SubModeMidiFxGroup();
	~SubModeMidiFxGroup() {}

	// Interface methods
	void onModeChanged() override;
	void onClockTick() override;
	void loopUpdate() override;
	void resync();
	bool updateLEDs() override;
	void onEncoderChanged(Encoder::Update enc);
	void onEncoderButtonDown() override;
	bool onKeyUpdate(OMXKeypadEvent e) override;
	void onDisplayUpdate() override;
	bool getEncoderSelect() override;

	void setSelected(bool newSelected);

	void noteInput(MidiNoteGroup note);
	void setNoteOutputFunc(void (*fptr)(void *, MidiNoteGroup), void *context);

	void onPendingNoteOff(int note, int channel);

	int saveToDisk(int startingAddress, Storage *storage);
	int loadFromDisk(int startingAddress, Storage *storage);

	void toggleArp();
	void toggleArpHold();
	bool isArpOn();
	bool isArpHoldOn();
	void nextArpPattern();
	void nextArpOctRange();
	void gotoArpParams();
	uint8_t getArpOctaveRange();

	midifx::MidiFXArpeggiator *getArp(bool autoCreate);

protected:
	// Interface methods
	void onEnabled() override;
	void onDisabled() override;
	void onEncoderChangedEditParam(Encoder::Update enc) override;

private:
	bool selected_ = false;
	bool midiFXParamView_ = false; // If true, parameters adjust the selected midiFX slot.
	bool arpParamView_ = false;	   // If true, parameters adjust the selected midiFX slot.

	uint8_t selectedMidiFX_ = 0; // Index of selected midiFX slot

	int8_t heldMidiFX_ = -1;
	uint8_t heldAnimPos_ = 0;
	Micros prevAnimTime_;

	uint8_t funcKeyMode_ = 0;

	bool auxDown_ = false; // set to aux state onEnable, must be true to exit mode with aux.

	bool auxReleased_ = false; // set to aux state onEnable, must be true to exit mode with aux.

	// typedef midifx::MidiFXInterface* MidiFXptr;

	std::vector<midifx::MidiFXInterface *> midifx_;

	std::vector<midifx::MidiFXInterface *> tempMidiFX_;

	// midifx::MidiFXInterface* midiFX1_ = nullptr;
	// midifx::MidiFXInterface* midiFX2_ = nullptr;
	// midifx::MidiFXInterface* midiFX3_ = nullptr;
	// midifx::MidiFXInterface* midiFX4_ = nullptr;

	// MidiFXptr* midifx_[4] = {nullptr, nullptr, nullptr, nullptr};

	uint8_t midifxTypes_[NUM_MIDIFX_SLOTS];

	MidiNoteGroup onNoteGroups[32];

	midifx::MidiFXInterface *getMidiFX(uint8_t index);
	void setMidiFX(uint8_t index, midifx::MidiFXInterface *midifx);
	uint8_t getArpIndex();
	void setupPageLegends();
	void setAuxDown(bool auxDown);

	void updateFuncKeyMode();

	void onDisplayUpdateMidiFX();

	void displayMidiFXName(uint8_t index);

	const char *getMFXDispName(uint8_t index);

	void selectMidiFX(uint8_t fxIndex);
	void changeMidiFXType(uint8_t slotIndex, uint8_t typeIndex, bool fromLoad = false);

	void copyMidiFX(uint8_t fxIndex);
	void cutMidiFX(uint8_t fxIndex);
	void pasteMidiFX(uint8_t fxIndex);

	void moveSelectedMidiFX(int8_t direction);

	midifx::MidiFXInterface *copyBuffer;

	// Static glue to link a pointer to a member function
	static void noteFuncForwarder(void *context, MidiNoteGroup note)
	{
		static_cast<SubModeMidiFxGroup *>(context)->noteOutputFunc(note);
	}

	// sends the final notes out of midifx
	void noteOutputFunc(MidiNoteGroup note);

	// Pointer to external function that notes are sent out of fxgroup to
	void *sendNoteOutFuncPtrContext_;
	void (*sendNoteOutFuncPtr_)(void *, MidiNoteGroup);

	// internal function link, will point to noteInput of first FX, or to noteOutputFunc if no FX
	void *doNoteOutputContext_;
	void (*doNoteOutput_)(void *, MidiNoteGroup);
	// // Static glue to link a pointer to a member function
	// static void doNoteOutputForwarder(void *context, MidiNoteGroup note)
	// {
	//     static_cast<SubModeMidiFxGroup *>(context)->noteOutputFunc(note);
	// }

	static void midiFxSelNoteInputForwarder(void *context, midifx::MidiFXSelector *mfxSelector, uint8_t midiFXIndex, MidiNoteGroup note)
	{
		static_cast<SubModeMidiFxGroup *>(context)->midiFxSelNoteInput(mfxSelector, midiFXIndex, note);
	}

	void midiFxSelNoteInput(midifx::MidiFXSelector *mfxSelector, uint8_t midiFXIndex, MidiNoteGroup note);
	void reconnectInputsOutputs();
};

// static const u_int8_t kNumMidiFXGroups = 5;
extern SubModeMidiFxGroup subModeMidiFx[NUM_MIDIFX_GROUPS];
