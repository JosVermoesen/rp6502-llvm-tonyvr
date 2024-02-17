// ---------------------------------------------------------------------------
// TxtKeyboard.h
// ---------------------------------------------------------------------------

#ifndef TXTKEYBOARD_H
#define TXTKEYBOARD_H

#include <cstdint>
#include <stdbool.h>
#include "TxtDisplay.h"

#define KEYBOARD_BYTES 32

class TxtKeyboard
{
public:

    TxtKeyboard() {}; // constructor

    bool InitKeyboard(uint16_t keybd_status_address, TxtDisplay * pDisplay);

    // the following are only valid AFTER call to InitKeyboard():
    void HandleKeys();

private:
    TxtDisplay * txtdisplay = NULL;
    uint16_t keybd_status = 0xFF20; // KEYBOARD_BYTES (32) of bitmask data
    uint8_t key_threshold = 10; // key repeat delay
};

#endif // TXTKEYBOARD_H