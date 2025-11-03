// MIDIClockStats.h
#pragma once

using Micros = unsigned long;

class MIDIClockStats
{
private:
    static const size_t WINDOW_SIZE = 48; // Store 2 quarter notes worth of timing data (24 PPQ * 2)
    static const size_t PPQ_IN = 24;         // Incoming MIDI Clock Pulses Per Quarter Note
    static constexpr float MICROS_PER_MINUTE = 60.0f * 1000000.0f;

    // Simple circular buffer for last N intervals
    Micros intervals[WINDOW_SIZE];
    size_t writeIndex;
    size_t count;

    // Last timestamp for interval calculation
    Micros lastTimestamp;

    // Running statistics
    float currentBPM;
    bool isRunning;

    // Debug
    Micros lastInterval;

public:
    MIDIClockStats() : writeIndex(0),
                       count(0),
                       lastTimestamp(0),
                       currentBPM(120.0f),
                       isRunning(false),
                       lastInterval(0)
    {
        for (size_t i = 0; i < WINDOW_SIZE; i++)
        {
            intervals[i] = 0;
        }
    }

    void clockPulse(Micros currentTime)
    {
        if (!isRunning)
            return;

        // Calculate interval if we have a previous timestamp
        if (lastTimestamp > 0)
        {
            Micros interval = currentTime - lastTimestamp;

            // Store in circular buffer
            intervals[writeIndex] = interval;
            writeIndex = (writeIndex + 1) % WINDOW_SIZE;
            if (count < WINDOW_SIZE)
                count++;

            // Calculate average interval
            Micros totalInterval = 0;
            size_t samplesToUse = count;

            for (size_t i = 0; i < samplesToUse; i++)
            {
                totalInterval += intervals[i];
            }

            float avgInterval = float(totalInterval) / samplesToUse;
            lastInterval = interval; // Store for debugging

            // Calculate BPM
            if (avgInterval > 0)
            {
                float newBPM = MICROS_PER_MINUTE / (avgInterval * PPQ_IN);
                // Simple low-pass filter
                // More aggressive smoothing to handle jittery sources like DAWs
                currentBPM = currentBPM * 0.95f + newBPM * 0.05f;
            }
        }

        lastTimestamp = currentTime;
    }

    void start()
    {
        reset();
        isRunning = true;
    }

    void stop()
    {
        isRunning = false;
    }

    void reset()
    {
        writeIndex = 0;
        count = 0;
        lastTimestamp = 0;
        // Don't reset currentBPM to allow for smooth restarts

        for (size_t i = 0; i < WINDOW_SIZE; i++)
        {
            intervals[i] = 0;
        }
    }

    // Getters
    float getBPM() const { return currentBPM; }
    bool isClockRunning() const { return isRunning; }

    // Debug getters
    Micros getLastInterval() const { return lastInterval; }
    size_t getSampleCount() const { return count; }
};