#include <Arduino.h>
// #define NUM_GRIDS 8

namespace euclidean
{
    // enum Grid_Resolutions
    // {
    //     HALF = 0,
    //     NORMAL,
    //     DOUBLE,
    //     FOUR,
    //     COUNT
    // };

    // struct InstSettings
    // {
    //     uint8_t note = 60;
    //     uint8_t midiChan = 1;
    //     uint8_t density = 0;
    //     uint8_t x = 128;
    //     uint8_t y = 128;
    // };

    // struct SnapShotSettings
    // {
    //     InstSettings instruments[4];
    //     uint8_t chaos = 0;
    //     uint8_t accent = 128;
    //     uint8_t resolution = 1;
    // };

    // constexpr uint8_t kStepsPerPattern = 32;

    // struct ChannelPatternLEDs
    // {
    //     uint8_t levels[kStepsPerPattern];
    // };

    class EuclideanMath
    {
    public:
        static const uint8_t kPatternSize = 32; // All pattern arrays are 32 length
        EuclideanMath();

        // bool array should be of length kPatternSize
        static void generateEuclidPattern(bool* pattern, uint8_t rotation, uint8_t events, uint8_t steps);
        // bool array should be of length kPatternSize
        static void clearPattern(bool* pattern);
        // bool array should be of length kPatternSize
        static void flipPattern(bool* pattern, uint8_t steps);
        // bool array should be of length kPatternSize
        static void rotatePattern(bool* pattern, uint8_t steps, uint8_t rotation);
    };

    class EuclideanSequencer
    {
    public:
        uint8_t grids_notes[4] = {36, 38, 42, 46};
        static const uint8_t num_notes = sizeof(grids_notes);
        uint8_t playingPattern = 0;

        // SnapShotSettings snapshots[8];
     
        EuclideanSequencer();

        void start();
        void stop();
        void proceed();
        void gridsTick();

        // void saveSnapShot(uint8_t snapShotIndex);
        // void loadSnapShot(uint8_t snapShotIndex);
        // SnapShotSettings* getSnapShot(uint8_t snapShotIndex);
        // void setSnapShot(uint8_t snapShotIndex, SnapShotSettings snapShot);
        
        static uint32_t randomValue(uint32_t init = 0);

        // ChannelPatternLEDs getChannelLEDS(uint8_t channel);

        // uint8_t getSeqPos();

        // bool getChannelTriggered(uint8_t chanIndex);

        // void setMidiChan(uint8_t chanIndex, uint8_t channel);
        // uint8_t getMidiChan(uint8_t chanIndex);

    private:
        // GridsChannel channel_;
        uint32_t divider_;
        uint8_t multiplier_;
        uint32_t tickCount_;
        // uint8_t density_[num_notes];
        // uint8_t perturbations_[num_notes];
        // uint8_t x_[num_notes];
        // uint8_t y_[num_notes];
        // uint8_t midiChannels_[num_notes];
        // bool channelTriggered_[num_notes];
        // uint8_t triggeredNotes_[num_notes]; // Keep track of triggered notes to avoid stuck notes
        // uint8_t resolution_;
        bool running_;

        // uint8_t defaultMidiChannel_ = 1;
    };

}
