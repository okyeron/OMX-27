#pragma once

#include "submode_interface.h"
#include "MidiFX/midifx_interface.h"

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

protected:
// Interface methods
    void onEnabled() override;
    void onDisabled() override;
    void onEncoderChangedEditParam(Encoder::Update enc) override;

private:
    bool midiFXParamView_ = false; // If true, parameters adjust the selected midiFX slot. 
    uint8_t selectedMidiFX_ = 0; // Index of selected midiFX slot

    // typedef midifx::MidiFXInterface* MidiFXptr;

    std::vector<midifx::MidiFXInterface*> midifx_;

    // midifx::MidiFXInterface* midiFX1_ = nullptr;
    // midifx::MidiFXInterface* midiFX2_ = nullptr;
    // midifx::MidiFXInterface* midiFX3_ = nullptr;
    // midifx::MidiFXInterface* midiFX4_ = nullptr;

    // MidiFXptr* midifx_[4] = {nullptr, nullptr, nullptr, nullptr};

    uint8_t midifxTypes_[4] = {0,0,0,0};

    midifx::MidiFXInterface *getMidiFX(uint8_t index);
    void setMidiFX(uint8_t index, midifx::MidiFXInterface* midifx);
    void setupPageLegends();

    void onDisplayUpdateMidiFX();

    void selectMidiFX(uint8_t fxIndex);
    void changeMidiFXType(uint8_t slotIndex, uint8_t typeIndex);

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