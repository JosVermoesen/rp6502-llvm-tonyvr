// ---------------------------------------------------------------------------
// TxtKeyboard.cpp
// ---------------------------------------------------------------------------

#include <rp6502.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <stdbool.h>
#include "TxtDisplay.h"
#include "TxtKeyboard.h"
#include "usb_hid_keys.h"

// 256 bytes HID code max, stored in 32 uint8
uint8_t keystates[KEYBOARD_BYTES] = {0};

// keystates[code>>3] gets contents from correct byte in array
// 1 << (code&7) moves a 1 into proper position to mask with byte contents
// final & gives 1 if key is pressed, 0 if not
#define key(code) (keystates[code >> 3] & (1 << (code & 7)))

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
bool TxtKeyboard::InitKeyboard(uint16_t keybd_status_address, TxtDisplay * pDisplay)
{
    if (pDisplay == NULL) {
        puts("Error: pDisplay == NULL in TxtKeyboard::InitKeyboard()");
        return false;
    }

    // defaults
    keybd_status = 0xFF20;
    key_threshold = 10;

    txtdisplay = pDisplay;

    if (keybd_status_address > 0 && keybd_status_address < 0xFFFF) {
        keybd_status = keybd_status_address;
    }
    if (keybd_status_address != keybd_status) {
        printf("Asked for keyboard_status_address of 0x%04X, but got 0x%04X\n", keybd_status_address, keybd_status);
    }
    //xreg_ria_keyboard(keybd_status);
    xregn( 0, 0, 0, 1, keybd_status);

    return true;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void TxtKeyboard::HandleKeys()
{
    int i;
    static uint16_t key_timer = 0;
    static bool handled_key = false;

    // update timer counts
    key_timer++;

    // slow down the key auto-repeats
    if (key_timer > key_threshold) {
        key_timer = 0;
        handled_key = false;
    }

    // fill the keystates bitmask array
    RIA.addr0 = keybd_status;
    RIA.step0 = 0;
    for (i = 0; i < KEYBOARD_BYTES; i++) {
        uint8_t new_keys;
        RIA.addr0 = keybd_status + i;
        new_keys = RIA.rw0;
/*
        // check for change in any and all keys
        {
            uint8_t j;
            for (j = 0; j < 8; j++) {
                uint8_t new_key = (new_keys & (1<<j));
                if ((((i<<3)+j)>3) && (new_key != (keystates[i] & (1<<j)))) {
                    printf( "key %d %s\n", ((i<<3)+j), (new_key ? "pressed" : "released"));
                }
            }
        }
*/
        keystates[i] = new_keys;
    }

    // check for a key down
    if (!(keystates[0] & 1)) {
        // handle the keystrokes
        if (!handled_key && key(KEY_UP)) {
            if (txtdisplay->cursor_row() > 0) {
                txtdisplay->UpdateCursor(txtdisplay->cursor_row()-1, txtdisplay->cursor_col());
            }
        } else if (!handled_key && key(KEY_DOWN)) {
            if (txtdisplay->cursor_row() < txtdisplay->canvas_rows()-1) {
                txtdisplay->UpdateCursor(txtdisplay->cursor_row()+1, txtdisplay->cursor_col());
            }
        } else if (!handled_key && key(KEY_LEFT)) {
            if (txtdisplay->cursor_col() > 0) {
                txtdisplay->UpdateCursor(txtdisplay->cursor_row(), txtdisplay->cursor_col()-1);
            }
        } else if (!handled_key && key(KEY_RIGHT)) {
            if (txtdisplay->cursor_col() < txtdisplay->canvas_cols()-1) {
                txtdisplay->UpdateCursor(txtdisplay->cursor_row(), txtdisplay->cursor_col()+1);
            }
        }
        handled_key = true;
    } else { // no keys down
        key_timer = 0;
        handled_key = false;
    }
}