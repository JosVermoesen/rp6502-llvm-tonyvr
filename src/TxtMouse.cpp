// ---------------------------------------------------------------------------
// TxtMouse.cpp
// ---------------------------------------------------------------------------

#include <rp6502.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <stdbool.h>
#include "TxtDisplay.h"
#include "TxtMouse.h"

#define MOUSE_DIV 1 // Mouse speed divider

bool TxtMouse::InitMouse(uint16_t mouse_struct_address,
                         uint16_t mouse_pointer_data_address,
                         uint16_t mouse_state_address,
                         TxtDisplay * pDisplay)
{
    if (pDisplay == NULL) {
        puts("Error: pDisplay == NULL in TxtMouse::InitMouse");
        return false;
    }

    // defaults
    mouse_struct = 0xFF50;
    mouse_data = 0xFE60; // to 0xFEC3, since 100 bytes are needed for mouse pointer data
    mouse_state = 0xF40;
    mouse_X = 0;
    mouse_Y = 0;

    txtdisplay = pDisplay;

    if (mouse_struct_address > 0x0000 && mouse_struct_address < 0xFFFF) {
        mouse_struct = mouse_struct_address;
    }
    if (mouse_pointer_data_address > 0x0000 && mouse_pointer_data_address < 0xFFFF) {
        mouse_data = mouse_pointer_data_address;
    }
    if (mouse_state_address > 0x0000 && mouse_state_address < 0xFFFF) {
        mouse_state = mouse_state_address;
    }
    if (mouse_struct_address != mouse_struct) {
        printf("Asked for mouse_struct_address of 0x%04X, but got 0x%04X\n", mouse_struct_address, mouse_struct);
    }
    if (mouse_pointer_data_address != mouse_data) {
        printf("Asked for mouse_pointer_data_address of 0x%04X, but got 0x%04X\n", mouse_pointer_data_address, mouse_data);
    }
    if (mouse_state_address != mouse_state) {
        printf("Asked for mouse_state_address of 0x%04X, but got 0x%04X\n", mouse_state_address, mouse_state);
    }

    // initialize bitmap mode structure (to draw mouse pointer)
    xram0_struct_set(mouse_struct, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(mouse_struct, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(mouse_struct, vga_mode3_config_t, width_px, 10);
    xram0_struct_set(mouse_struct, vga_mode3_config_t, height_px, 10);
    xram0_struct_set(mouse_struct, vga_mode3_config_t, xram_data_ptr, mouse_data);
    xram0_struct_set(mouse_struct, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    //xreg_vga_mode(3, 3, mouse_struct, 2); // mode3 (bitmap), 8-bit color, plane2
    xregn( 1, 0, 1, 4, 3, 3, mouse_struct, 2);

    //xreg_ria_mouse(mouse_state);
    xregn( 0, 0, 1, 1, mouse_state);

    DrawMousePointer();

    return true;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void TxtMouse::DrawMousePointer(void)
{
    int i;
    static const uint8_t data[100] = {
        16, 16, 16, 16, 16, 16, 16,  0,  0,  0,
        16,255,255,255,255,255, 16,  0,  0,  0,
        16,255,255,255,255, 16,  0,  0,  0,  0,
        16,255,255,255,255, 16,  0,  0,  0,  0,
        16,255,255,255,255,255, 16,  0,  0,  0,
        16,255, 16, 16,255,255,255, 16,  0,  0,
        16, 16,  0,  0, 16,255,255,255, 16,  0,
         0,  0,  0,  0,  0, 16,255,255,255, 16,
         0,  0,  0,  0,  0,  0, 16,255, 16,  0,
         0,  0,  0,  0,  0,  0,  0, 16,  0,  0,
    };


    RIA.addr0 = mouse_data;
    RIA.step0 = 1;
    for (i = 0; i < 100; i++) {
        RIA.rw0 = data[i];
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
bool TxtMouse::HandleMouse(void)
{
    static int sx, sy;
    static uint8_t mb, mx, my;
    int16_t x, y;
    uint8_t rw, changed, pressed, released;
    bool xchg = false;
    bool ychg = false;

    // read current X
    RIA.addr0 = mouse_state + 1;
    rw = RIA.rw0;
    if (mx != rw)
    {
        xchg = true;
        sx += (int8_t)(rw - mx);
        mx = rw;
        if (sx < 0)
            sx = 0;
        if (sx > (txtdisplay->canvas_width() - 2) * MOUSE_DIV)
            sx = (txtdisplay->canvas_width() - 2) * MOUSE_DIV;
    }

    // read current y
    RIA.addr0 = mouse_state + 2;
    rw = RIA.rw0;
    if (my != rw)
    {
        ychg = true;
        sy += (int8_t)(rw - my);
        my = rw;
        if (sy < 0)
            sy = 0;
        if (sy > (txtdisplay->canvas_height() - 2) * MOUSE_DIV)
            sy = (txtdisplay->canvas_height() - 2) * MOUSE_DIV;
    }

    // update mouse pointer on screen
    x = sx / MOUSE_DIV;
    y = sy / MOUSE_DIV;
    xram0_struct_set(mouse_struct, vga_mode3_config_t, x_pos_px, x);
    xram0_struct_set(mouse_struct, vga_mode3_config_t, y_pos_px, y);
    x++, y++;

    mouse_X = x;
    mouse_Y = y;

    // read button states, and detect changes
    RIA.addr0 = mouse_state + 0;
    rw = RIA.rw0;
    changed = mb ^ rw;
    pressed = rw & changed;
    released = mb & changed;
    mb = rw;

    // handle button changes
    if (pressed & 1) {
        LeftBtnPressed(x, y);
    }
    if (released & 1) {
        LeftBtnReleased(x, y);
    }
    if (pressed & 2) {
        RightBtnPressed(x, y);
    }
    if (released & 2) {
        RightBtnReleased(x, y);
    }

    // if mouse moved, return true
    if (xchg || ychg) {
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
bool TxtMouse::LeftBtnPressed(int16_t x, int16_t y)
{
    int16_t r = y/txtdisplay->font_height();
    int16_t c = x/txtdisplay->font_width();

    printf("left btn pressed at x = %u, y = %u, row = %u, col = %u\n", x, y, r, c);

    // move the cursor to the current mouse position
    txtdisplay->UpdateCursor(r, c);

    return true;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
bool TxtMouse::LeftBtnReleased(int16_t x, int16_t y)
{
    printf("left btn released at x = %u, y = %u, row = %u, col = %u\n",
    x, y, y/txtdisplay->font_height(), x/txtdisplay->font_width());
    return true;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
bool TxtMouse::RightBtnPressed(int16_t x, int16_t y)
{
    printf("right btn pressed at x = %u, y = %u, row = %u, col = %u\n",
            x, y, y/txtdisplay->font_height(), x/txtdisplay->font_width());
    return true;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
bool TxtMouse::RightBtnReleased(int16_t x, int16_t y)
{
    printf("right btn released at x = %u, y = %u, row = %u, col = %u\n",
           x, y, y/txtdisplay->font_height(), x/txtdisplay->font_width());
    return true;
}