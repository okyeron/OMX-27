#include "omx_keypad.h"

/**************************************************************************/
/*!
    @brief  default constructor
    @param  holdThreshold time a button must be held to trigger the "held" state
    @param  clickWindow time for registering multiple clicks, resets on each click
    @param  userKeymap a multidimensional array of key characters
    @param  row an array of GPIO pins that are connected to each row of the
keypad
    @param  col an array of GPIO pins that are connected to each column of the
keypad
    @param  numRows the number of rows on the keypad
    @param  numCols the number of columns on the keypad
*/
/**************************************************************************/
OMXKeypad::OMXKeypad(uint32_t holdThreshold, uint32_t clickWindow, byte *userKeymap, byte *row, byte *col, int numRows, int numCols):
    numRows(numRows),
    numCols(numCols),
    holdThreshold(holdThreshold),
    clickWindow(clickWindow),
    keypad(userKeymap, row, col, numRows, numCols),
    keys(numRows * numCols)
{
}

void OMXKeypad::tick() {
    keypad.tick();

    uint32_t now = millis();
    while (keypad.available()) {
        keypadEvent e = keypad.read();
        // the key isn't an index.
        uint8_t index = (e.bit.ROW * numCols) + e.bit.COL;
        keystate* key = &(keys[index]);

        switch(e.bit.EVENT) {
            case KEY_JUST_PRESSED:
                // first press.
                if (key->lastClickedAt == 0) {
                    key->key = e.bit.KEY;
                    key->index = index;
                    key->held = false;
					if (key->releasedAt < now - clickWindow){
						key->clicks = 0;
					}
                    active.push_back(key);
                }
				
                key->lastClickedAt = now;
                key->down = true;
                key->held = false;
                // "press" is always available
                _available.push_back(key); // this is what triggers the key to show up with a state change
                break;
            case KEY_JUST_RELEASED:
                key->down = false;
				key->clicks++;
				key->releasedAt = now;

                if (key->held) {
                    // hold release event.
                    key->held = false;
                }
                _available.push_back(key);	// on key release, this is the only event added.
                break;
            default:
                // unknown event
                break;
        };
    }

    // exit early if there are no active keys to update.
    if (active.size() == 0) return;

    // Check if any active keys are ready to become available.
    uint32_t click_window_close = now - clickWindow;
    uint32_t held = now - holdThreshold;
    auto it = active.begin();
    while (it != active.end()) {
        auto key = *it;
        if (key->down && key->lastClickedAt < held) {
            key->held = true;
            _available.push_back(key);
            active.erase(it);
        } else if (!key->down && key->lastClickedAt < click_window_close) {
//             _available.push_back(key);
            active.erase(it);
//         } else if (!key->down && key->lastClickedAt < now) {
//             active.erase(it);
        } else {
            // it is not ready to become active, move to next.
            it++;
        }
    }
}

OMXKeypadEvent OMXKeypad::next() {
    if (!available()) {
        return OMXKeypadEvent{0, 0, false, false};
    }

    auto key = _available.back();
    _available.pop_back();

    // Simple press event.
    if (key->down && !key->held) {
        return OMXKeypadEvent{key->key, key->clicks, false, true};
    } 

	// Click or hold event
	key->lastClickedAt = 0;
	return OMXKeypadEvent{key->key, key->clicks, key->held, key->down};

}