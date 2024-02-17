// ---------------------------------------------------------------------------
// TxtDisplay.cpp
// ---------------------------------------------------------------------------

#include <rp6502.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <stdbool.h>
#include "TxtDisplay.h"

#define COLOR_FROM_RGB5(r,g,b) (((uint16_t)b<<11)|((uint16_t)g<<6)|((uint16_t)r))
#define COLOR_ALPHA_MASK (1u<<5)

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
int8_t TxtDisplay::bpp_from_bpp_opt(uint8_t bpp_opt)
{
    switch(bpp_opt) {
        case 0: return  1;
        case 1: return -4;
        case 2: return  4;
        case 3: return  8;
        case 4: return 16;
    }
    return 4; // default
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
void TxtDisplay::InitDisplay(uint16_t canvas_struct_address,
                             uint16_t canvas_data_address,
                             uint8_t  canvas_plane,
                             uint8_t  canvas,
                             uint8_t  font_bpp_opt)
{
    uint8_t x_offset = 0;
    uint8_t y_offset = 0;
    uint8_t final_font_bpp_opt;
    int8_t f, b;



    // defaults: 640x480 pixels, 80x30 chars,
    //           16 colors (4bpp), bg = black, fg = light_gray
    canvas_struct = 0xFF00;
    canvas_data = 0x0000;
    plane = 0;
    canvas_type = 3;
    canvas_w = 640;
    canvas_h = 480;
    canvas_c = 80;
    canvas_r = 30;
    font_w = 8;
    font_h = 16;
    bpp = 4;
    display_bg = 0;
    display_fg = 8;
    final_font_bpp_opt = 10;
    display_bg = 0;
    display_fg = 7;

    cur_state = BLINK_OFF;
    cur_r = 0;
    cur_c = 0;
    cur_threshold = 15; // cursor blink delay
    disable_cur_blinking = false;

    // validate parameters, forcing to defaults if bad
    if (canvas_struct_address != 0) {
        canvas_struct = canvas_struct_address;
    }
    if (canvas_data_address != 0) {
        canvas_data = canvas_data_address;
    }
    if (/*canvas_plane >= 0 &&*/ canvas_plane <= 2) {
        plane = canvas_plane;
    }
    if (canvas > 0 && canvas <= 4) {
        canvas_type = canvas;
    }
    if (font_bpp_opt != 0) {
        f = ((font_bpp_opt & 0x08) >> 3);
        b = (font_bpp_opt & 0x07);
        if (b > 0 && b <= 4) {
            bpp = bpp_from_bpp_opt(b);
        } else {
            b = 2; // default
            bpp = 4;
        }
        final_font_bpp_opt = (f << 3) | b;
        font_h = (f == 1) ? 16 : 8;

        // now that we know bpp
        display_bg = black();
        display_fg = light_gray();
    }

    // compute canvas rows, cols in chars, and width, height in pixels
    switch (canvas_type) {
        case 1: // 320x240 pixels
            canvas_w = 320;
            canvas_h = 240;
            canvas_c = canvas_w/font_w; // 320/8=40
            canvas_r = canvas_h/font_h; // 240/8=30 or 240/16=15
            break;
        case 2: // 320x180 pixels
            canvas_w = 320;
            canvas_h = 180;
            canvas_c = canvas_w/font_w; // 320/8=40
            canvas_r = canvas_h/font_h; // 180/8=22 or 180/16=11
            canvas_w = canvas_c*font_w; // 40*8=320
            canvas_h = canvas_r*font_h; // 22*8=176 or 11*16=176
            break;
        case 3: // 640x480 pixels
            canvas_w = 640;
            canvas_h = 480;
            canvas_c = canvas_w/font_w; // 640/8=80
            canvas_r = canvas_h/font_h; // 480/8=60 or 480/16=30
            break;
        case 4: // 640x360 pixels
            canvas_w = 640;
            canvas_h = 360;
            canvas_c = canvas_w/font_w; // 640/8=80
            canvas_r = canvas_h/font_h; // 360/8=45 or 360/16=22
            canvas_w = canvas_c*font_w; // 80*8=320
            canvas_h = canvas_r*font_h; // 45*8=360 or 22*16=352
            break;
    }

    if (canvas_struct_address != canvas_struct) {
        printf("Asked for canvas_struct_address of 0x%04X, but got 0x%04X\n", canvas_struct_address, canvas_struct);
    }
    if (canvas_data_address != canvas_data) {
        printf("Asked for canvas_data_address of 0x%04X, but got 0x%04X\n", canvas_struct_address, canvas_data);
    }
    if (canvas_plane != plane) {
        printf("Asked for canvas_plane %u, but got %u\n", canvas_plane, plane);
    }
    if (canvas_type != canvas) {
        printf("Asked for canvas %u, but got %u\n", canvas, canvas_type);
    }
    if (font_bpp_opt != final_font_bpp_opt) {
        printf("Asked for font_bpp_opt of %u, but got %u\n", font_bpp_opt, final_font_bpp_opt);
    }

    // initialize the canvas
    //xreg_vga_canvas(canvas_type); // the old way
    xregn(1, 0, 0, 1, canvas_type);

    xram0_struct_set(canvas_struct, vga_mode1_config_t, x_wrap, false);
    xram0_struct_set(canvas_struct, vga_mode1_config_t, y_wrap, false);
    xram0_struct_set(canvas_struct, vga_mode1_config_t, x_pos_px, x_offset);
    xram0_struct_set(canvas_struct, vga_mode1_config_t, y_pos_px, y_offset);
    xram0_struct_set(canvas_struct, vga_mode1_config_t, width_chars, canvas_c);
    xram0_struct_set(canvas_struct, vga_mode1_config_t, height_chars, canvas_r);
    xram0_struct_set(canvas_struct, vga_mode1_config_t, xram_data_ptr, canvas_data);
    xram0_struct_set(canvas_struct, vga_mode1_config_t, xram_palette_ptr, 0xFFFF);
    xram0_struct_set(canvas_struct, vga_mode1_config_t, xram_font_ptr, 0xFFFF);

    //xreg_vga_mode(1, final_font_bpp_opt, canvas_struct, plane); // the old way
    xregn(1, 0, 1, 4, 1, final_font_bpp_opt, canvas_struct, plane);

    ClearDisplay(display_bg, display_fg);

    cur_r = canvas_r/2;
    cur_c = canvas_c/2;
}

// ----------------------------------------------------------------------------
// Changes display_bg and display_fg, then overwrites display using them
// ----------------------------------------------------------------------------
void TxtDisplay::ClearDisplay(uint16_t  display_bg_color, uint16_t  display_fg_color)
{
    uint16_t i, j;

    if (display_bg_color != display_fg_color) {
        bool bg_validated = false;
        bool fg_validated = false;
        if (bpp == -4 | bpp == 4) {
            bg_validated = (display_bg_color >= 0 && display_bg_color <= 15);
            fg_validated = (display_fg_color >= 0 && display_fg_color <= 15);
        } else if (bpp == 8) {
            bg_validated = (display_bg_color >= 0 && display_bg_color <= 255);
            fg_validated = (display_fg_color >= 0 && display_fg_color <= 255);
        } else if (bpp == 16) {
            // anything goes?
            bg_validated = true;
            fg_validated = true;
        }
        if (bg_validated) {
            display_bg = display_bg_color;
        }
        if (fg_validated) {
            display_fg = display_fg_color;
        }
    }
    if (display_bg_color != display_bg) {
        printf("Asked for display_bg_color of %u, but got %u\n", display_bg_color, display_bg);
    }
    if (display_fg_color != display_fg) {
        printf("Asked for display_fg_color of %u, but got %u\n", display_fg_color, display_fg);
    }

    RIA.addr0 = canvas_data;
    RIA.step0 = 1;

    for (i = 0; i < canvas_r; i++) {
        for (j = 0; j < canvas_c; j++) {
            DrawChar(i, j, ' ', display_bg, display_fg);
        }
    }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void TxtDisplay::DrawChar(uint16_t row, uint16_t col,
                          char ch, uint16_t bg, uint16_t fg)
{
    if (bpp == -4 || bpp == 4) {
        // for 4-bit color, index 2 bytes per ch
        RIA.addr0 = canvas_data + 2*(row*canvas_c + col);
        RIA.step0 = 1;
        RIA.rw0 = ch;
        RIA.rw0 = (uint8_t)(((uint8_t)bg<<4) | (uint8_t)fg);
    } else if (bpp == 8) {
        // for 8-bit color, index 3 bytes per ch
        RIA.addr0 = canvas_data + 3*(row*canvas_c + col);
        RIA.step0 = 1;
        RIA.rw0 = ch;
        RIA.rw0 = (uint8_t)fg;
        RIA.rw0 = (uint8_t)bg;
    } else if (bpp == 16) {
        // for 16-bit color, index 6 bytes per ch
        RIA.addr0 = canvas_data + 6*(row*canvas_c + col);
        RIA.step0 = 1;
        RIA.rw0 = ch;
        RIA.rw0 = 0; // not used
        RIA.rw0 = (uint8_t)fg;
        RIA.rw0 = (uint8_t)(fg >> 8);
        RIA.rw0 = (uint8_t)bg;
        RIA.rw0 = (uint8_t)(bg >> 8);
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
void TxtDisplay::GetChar(uint16_t row, uint16_t col,
                         char & ch, uint16_t & bg, uint16_t & fg)
{
    if (bpp == -4 || bpp == 4) {
        uint8_t bgfg;
        // for 4-bit color, index 2 bytes per ch
        RIA.addr0 = canvas_data + 2*(row*canvas_c + col);
        RIA.step0 = 1;
        ch = RIA.rw0;
        bgfg = RIA.rw0;
        bg = bgfg >> 4;
        fg = bgfg & 0x0F;
    } else if (bpp == 8) {
        // for 8-bit color, index 3 bytes per ch
        RIA.addr0 = canvas_data + 3*(row*canvas_c + col);
        RIA.step0 = 1;
        ch = RIA.rw0;
        fg = RIA.rw0;
        bg = RIA.rw0;
    } else if (bpp == 16) {
        uint8_t attrib, fghi, fglo, bghi, bglo;
        // for 16-bit color, index 6 bytes per ch
        RIA.addr0 = canvas_data + 6*(row*canvas_c + col);
        RIA.step0 = 1;
        ch = RIA.rw0;
        attrib = RIA.rw0; // not used
        fghi = RIA.rw0;
        fglo = RIA.rw0;
        bghi = RIA.rw0;
        bglo = RIA.rw0;
        fg = (((uint16_t)fghi << 8) | ((uint16_t)fglo));
        bg = (((uint16_t)bghi << 8) | ((uint16_t)bglo));
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
void TxtDisplay::UpdateCursor(uint16_t new_row, uint16_t new_col)
{
    char ch;
    uint16_t fg, bg;
    cursor_state_t old_state = cur_state;

    // check if move is desired
    if (new_row != cur_r || new_col != cur_c) {
        cur_state = MOVING;
        printf("cur_r=%d, new_row=%d, cur_c=%d, new_col=%d\n",
               cur_r, new_row, cur_c, new_col);
    }

    // get current data
    GetChar(cur_r, cur_c, ch, bg, fg);

    if (old_state == BLINK_ON) {
        // restore original colors
        DrawChar(cur_r, cur_c, ch, fg, bg);
    }

    if (cur_state == MOVING) {
        // get new data
        GetChar(new_row, new_col, ch, bg, fg);

        // reverse colors at new location
        DrawChar(new_row, new_col, ch, fg, bg);
        cur_state = BLINK_ON;
    } else { // blinking
        if (old_state == BLINK_OFF) {
            if (!disable_cur_blinking) {
                // reverse colors to show cursor
                DrawChar(cur_r, cur_c, ch, fg, bg);
                cur_state = BLINK_ON;
            }
        } else {
            // restore original colors previously
            cur_state = BLINK_OFF;
        }
    }
    cur_r = new_row;
    cur_c = new_col;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
void TxtDisplay::DisableCursorBlinking() {
    disable_cur_blinking = true;
    puts("disabling blinking");
    UpdateCursor(cur_r, cur_c);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
void TxtDisplay::EnableCursorBlinking() {
    disable_cur_blinking = false;
    puts("enabling blinking");
    UpdateCursor(cur_r, cur_c);
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
void TxtDisplay::HandleDisplay(void)
{
    static uint16_t cursor_timer = 0;

    // update timer counts
    cursor_timer++;

    // blink the cursor
    if (cursor_timer > cur_threshold) {
        cursor_timer = 0;
        if (cursor_state() != MOVING) {
            UpdateCursor(cur_r, cur_c);
        }
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::black(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(0,0,0)&~COLOR_ALPHA_MASK): 0;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::dark_red(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(15,0,0)|COLOR_ALPHA_MASK) : 1;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::dark_green(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(0,15,0)|COLOR_ALPHA_MASK) : 2;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::brown(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(15,15,0)|COLOR_ALPHA_MASK) : 3;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::dark_blue(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(0,0,15)|COLOR_ALPHA_MASK) : 4;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::dark_magenta(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(15,0,15)|COLOR_ALPHA_MASK) : 5;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::dark_cyan(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(0,15,15)|COLOR_ALPHA_MASK) : 6;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::light_gray(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(15,15,15)|COLOR_ALPHA_MASK) : 7;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::dark_gray(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(7,7,7)|COLOR_ALPHA_MASK) : 8;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::red(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(31,0,0)|COLOR_ALPHA_MASK) : 9;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::green(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(0,31,0)|COLOR_ALPHA_MASK) : 10;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::yellow(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(31,31,0)|COLOR_ALPHA_MASK) : 11;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::blue(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(0,0,31)|COLOR_ALPHA_MASK) : 12;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::magenta(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(31,0,31)|COLOR_ALPHA_MASK) : 13;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::cyan(void)
{
    return (bpp == 16) ? (COLOR_FROM_RGB5(0,31,31)|COLOR_ALPHA_MASK) : 14;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
uint16_t TxtDisplay::white(void)
{
    uint16_t retval = 0;
    if (bpp == 16) {
        retval = COLOR_FROM_RGB5(31,31,31);
        retval = retval|COLOR_ALPHA_MASK;
    } else {
        retval = 15;
    }
    return retval;
}