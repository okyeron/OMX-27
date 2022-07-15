#include <Arduino.h>
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

    struct GridPatterns
    {
        uint8_t density = 0;
        uint8_t x = 128;
        uint8_t y = 128;
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
        int playingPattern = 0;

        GridPatterns gridSaves[8][4] = {
            {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
            {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
            {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
            {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
            {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
            {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
            {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
            {GridPatterns(), GridPatterns(), GridPatterns(), GridPatterns()},
        };

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

        void setDensity(uint8_t channel, uint8_t density);
        uint8_t getDensity(uint8_t channel);

        void setX(uint8_t channel, uint8_t x);
        uint8_t getX(uint8_t channel);

        void setY(uint8_t channel, uint8_t y);
        uint8_t getY(uint8_t channel);

        void setChaos(uint8_t c);
        uint8_t getChaos();

        void setResolution(uint8_t r);

        void setAccent(uint8_t a);
        uint8_t getAccent();

        static uint32_t randomValue(uint32_t init = 0);

        ChannelPatternLEDs getChannelLEDS(uint8_t channel);

        uint8_t getSeqPos();

        bool getChannelTriggered(int chanIndex);


    private:
        GridsChannel channel_;
        uint32_t divider_;
        uint8_t multiplier_;
        uint32_t tickCount_;
        uint8_t density_[num_notes];
        uint8_t perturbations_[num_notes];
        uint8_t x_[num_notes];
        uint8_t y_[num_notes];
        bool channelTriggered_[num_notes];
        bool running_;

        uint8_t midiChannel_ = 1;
    };

}
