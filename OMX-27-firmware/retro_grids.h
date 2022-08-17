#include <Arduino.h>
#include "config.h"
// #define NUM_GRIDS 8

namespace grids
{

    enum Grid_Resolutions
    {
        HALF = 0,
        NORMAL,
        DOUBLE,
        FOUR,
        COUNT
    };

    struct InstSettings
    {
        uint8_t note = 60;
        uint8_t noteLength = 3;
        uint8_t midiChan = 1;
        uint8_t density = 0;
        uint8_t x = 128;
        uint8_t y = 128;
    };

    struct SnapShotSettings
    {
        InstSettings instruments[4];
        uint8_t chaos = 0;
        uint8_t accent = 128;
        uint8_t resolution = 1;
        uint8_t swing = 1;
    };

    constexpr uint8_t kStepsPerPattern = 32;

    struct ChannelPatternLEDs
    {
        uint8_t levels[kStepsPerPattern];
    };

    class GridsChannel
    {
    public:
        GridsChannel();

        void setStep(uint8_t step);
        uint8_t level(int selector, uint16_t x, uint16_t y);
        static uint8_t U8Mix(uint8_t a, uint8_t b, uint8_t balance);

    private:
        static uint8_t ReadDrumMap(uint8_t step, uint8_t instrument, uint8_t x, uint8_t y);

        int NumParts = 4;
        uint8_t step_;
    };

    class GridsWrapper
    {
    public:
        uint8_t chaos;
        uint8_t accent;

        uint8_t grids_notes[4] = {36, 38, 42, 46};
        static const uint8_t num_notes = sizeof(grids_notes);
        uint8_t playingPattern = 0;

        SnapShotSettings snapshots[8];

        // GridPatterns gridSaves[8][4] = {
        //     {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
        //     {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
        //     {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
        //     {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
        //     {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
        //     {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
        //     {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
        //     {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
        // };

        // GridPatterns gridSaves[8][4] = {
        //     {{.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}},
        //     {{.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}},
        //     {{.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}},
        //     {{.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}},
        //     {{.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}},
        //     {{.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}},
        //     {{.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}},
        //     {{.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}, {.density = 0, .x = 128, .y = 128}}};

        GridsWrapper();

        void start();
        void stop();
        void proceed();
        void gridsTick();

        void clockTick(uint32_t stepmicros, uint32_t microsperstep);

        void setNoteOutputFunc(void (*fptr)(void *, uint8_t, MidiNoteGroup), void *context);

        void saveSnapShot(uint8_t snapShotIndex);
        void loadSnapShot(uint8_t snapShotIndex);
        SnapShotSettings* getSnapShot(uint8_t snapShotIndex);
        void setSnapShot(uint8_t snapShotIndex, SnapShotSettings snapShot);

        void setNoteLength(uint8_t channel, uint8_t newNoteLength);
        uint8_t getNoteLength(uint8_t channel);

        void setDensity(uint8_t channel, uint8_t density);
        uint8_t getDensity(uint8_t channel);

        void setX(uint8_t channel, uint8_t x);
        uint8_t getX(uint8_t channel);

        void setY(uint8_t channel, uint8_t y);
        uint8_t getY(uint8_t channel);

        void setChaos(uint8_t c);
        uint8_t getChaos();

        void setResolution(uint8_t r);

        void setSwing(uint8_t newSwing);
        uint8_t getSwing();

        void setAccent(uint8_t a);
        uint8_t getAccent();

        static uint32_t randomValue(uint32_t init = 0);

        ChannelPatternLEDs getChannelLEDS(uint8_t channel);

        uint8_t getSeqPos();

        bool getChannelTriggered(uint8_t chanIndex);

        void setMidiChan(uint8_t chanIndex, uint8_t channel);
        uint8_t getMidiChan(uint8_t chanIndex);

    private:
        GridsChannel channel_;
        uint32_t divider_;
        uint8_t multiplier_;
        uint32_t tickCount_;
        uint8_t density_[num_notes];
        uint8_t perturbations_[num_notes];
        uint8_t x_[num_notes];
        uint8_t y_[num_notes];
        uint8_t midiChannels_[num_notes];
        uint8_t noteLengths_[num_notes];
        uint32_t noteOffMicros_[num_notes];
        bool channelTriggered_[num_notes];
        uint8_t triggeredNotes_[num_notes]; // Keep track of triggered notes to avoid stuck notes
        uint8_t resolution_;
        uint8_t swing_ = 0;
        bool running_;
        float resMultiplier_ = 1;

        uint8_t defaultMidiChannel_ = 1;

        // Note On pointers
        void * onNoteOnFuncPtrContext_;
        void (*onNoteOnFuncPtr_)(void *, uint8_t, MidiNoteGroup);
        void onNoteOn(uint8_t gridsChannel, uint8_t channel, uint8_t noteNumber, uint8_t velocity, float stepLength, bool sendMidi, bool sendCV, uint32_t noteOnMicros);

        // clock values
        Micros nextStepTimeP_ = 32;
        Micros lastStepTimeP_ = 32;
        uint32_t stepMicroDelta_ = 0;

        // void advanceStep(uint32_t stepmicros);
    };

}
