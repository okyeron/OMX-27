#pragma once

#include "submode_interface.h"
#include "midifx_interface.h"
#include "storage.h"

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
    void loopUpdate() override;
    void updateLEDs() override;
    void onEncoderChanged(Encoder::Update enc);
    void onEncoderButtonDown() override;
    void onKeyUpdate(OMXKeypadEvent e) override;
    void onDisplayUpdate() override;

    void noteInput(MidiNoteGroup note);
    void setNoteOutputFunc(void (*fptr)(void *, MidiNoteGroup), void *context);

    void onPendingNoteOff(int note, int channel);

    int saveToDisk(int startingAddress, Storage *storage);
    int loadFromDisk(int startingAddress, Storage *storage);
protected:
// Interface methods
    void onEnabled() override;
    void onDisabled() override;
    void onEncoderChangedEditParam(Encoder::Update enc) override;

private:
    bool midiFXParamView_ = false; // If true, parameters adjust the selected midiFX slot. 
    uint8_t selectedMidiFX_ = 0; // Index of selected midiFX slot

    bool auxReleased_ = false; // set to aux state onEnable, must be true to exit mode with aux. 

    // typedef midifx::MidiFXInterface* MidiFXptr;

    std::vector<midifx::MidiFXInterface*> midifx_;

    // midifx::MidiFXInterface* midiFX1_ = nullptr;
    // midifx::MidiFXInterface* midiFX2_ = nullptr;
    // midifx::MidiFXInterface* midiFX3_ = nullptr;
    // midifx::MidiFXInterface* midiFX4_ = nullptr;

    // MidiFXptr* midifx_[4] = {nullptr, nullptr, nullptr, nullptr};

    uint8_t midifxTypes_[NUM_MIDIFX_SLOTS];

    MidiNoteGroup onNoteGroups[32];

    midifx::MidiFXInterface *getMidiFX(uint8_t index);
    void setMidiFX(uint8_t index, midifx::MidiFXInterface* midifx);
    void setupPageLegends();

    void onDisplayUpdateMidiFX();

    void displayMidiFXName(uint8_t index);

    const char* getMFXDispName(uint8_t index);

    void selectMidiFX(uint8_t fxIndex);
    void changeMidiFXType(uint8_t slotIndex, uint8_t typeIndex, bool fromLoad = false);

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

    void reconnectInputsOutputs();
};

// static const u_int8_t kNumMidiFXGroups = 5;
extern SubModeMidiFxGroup subModeMidiFx[NUM_MIDIFX_GROUPS];