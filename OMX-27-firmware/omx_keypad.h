#pragma once

#include <Adafruit_Keypad.h>
#include <vector>

#define MAX_CONCURRENT_KEYS 10

// Forward declare
struct OMXKeypadEvent;

/**
 * Keep track of button states.
 */
class OMXKeypad {
    private:
    // This could be optimized to use less space.
    struct keystate {
        // constructor for vector initializer
        keystate(): lastClickedAt(0){};

        uint8_t index;
        uint8_t key;
        bool held;
        bool down;
        uint8_t clicks;
        uint32_t lastClickedAt;
    };

    int numRows;
    int numCols;
    uint32_t holdThreshold;
    uint32_t clickWindow;
    Adafruit_Keypad keypad;
    std::vector<keystate> keys;
    std::vector<keystate*> active;
    std::vector<keystate*> _available;

    public:
    OMXKeypad(uint32_t holdThreshold, uint32_t clickWindow, byte *userKeymap,
        byte *row, byte *col, int numRows, int numCols);
    inline void begin() { keypad.begin(); }
    void tick();

    inline bool available() { return _available.size() > 0; }
    OMXKeypadEvent next();
};


struct OMXKeypadEvent {
    OMXKeypadEvent(uint8_t key, uint8_t clicks, bool held, bool down):
        _key(key),
        _clicks(clicks),
        _held(held),
        _down(down)
    {}

    private:
    uint8_t _key;
    uint8_t _clicks;
    bool _held;
    bool _down;

    public:
    inline uint8_t key() { return _key; }
    inline bool down() { return _down; }
    inline bool held() { return _held; }
    inline uint8_t clicks() { return _clicks; }
};