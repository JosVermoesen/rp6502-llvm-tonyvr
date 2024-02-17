// ---------------------------------------------------------------------------
// TxtDisplay.h
// ---------------------------------------------------------------------------

#ifndef TXTDISPLAY_H
#define TXTDISPLAY_H

#include <cstdint>
#include <stdbool.h>

class TxtDisplay
{
public:
    typedef enum {MOVING, BLINK_ON, BLINK_OFF} cursor_state_t ;

    TxtDisplay() {}; // constructor

    void InitDisplay(uint16_t canvas_struct_address = 0xFF00,
                     uint16_t canvas_data_address = 0x0000,
                     uint8_t  canvas_plane = 0,
                     uint8_t  canvas = 3,
                     uint8_t  font_bpp_opt = 10);

    // the following are only valid AFTER call to InitDisplay():
    void DrawChar(uint16_t row, uint16_t col,
                  char ch, uint16_t bg, uint16_t fg);

    void GetChar(uint16_t row, uint16_t col,
                 char & ch, uint16_t & bg, uint16_t & fg);

    void ClearDisplay(uint16_t display_bg_color = 0,
                      uint16_t display_fg_color = 0);

    void UpdateCursor(uint16_t new_row, uint16_t new_col);
    void DisableCursorBlinking();
    void EnableCursorBlinking();
    void HandleDisplay();

    uint16_t cursor_row(void) {return cur_r;};
    uint16_t cursor_col(void) {return  cur_c;};
    cursor_state_t cursor_state(void) {return cur_state;};

    uint16_t canvas_struct_address(void) {return canvas_struct;};
    uint16_t canvas_data_address(void) {return canvas_data;};
    uint8_t  canvas_opt (void) {return canvas_type;};
    uint16_t canvas_width(void) {return canvas_w;};
    uint16_t canvas_height(void) {return canvas_h;};
    uint16_t canvas_rows(void) {return canvas_r;};
    uint16_t canvas_cols(void){return canvas_c;};
    uint8_t font_width(void) {return font_w;};
    uint8_t font_height(void) {return font_h;};
    int8_t bits_per_pixel(void) {return bpp;};

    uint16_t black(void);
    uint16_t dark_red(void);
    uint16_t dark_green(void);
    uint16_t brown(void);
    uint16_t dark_blue(void);
    uint16_t dark_magenta(void);
    uint16_t dark_cyan(void);
    uint16_t light_gray(void);
    uint16_t dark_gray(void);
    uint16_t red(void);
    uint16_t green(void);
    uint16_t yellow(void);
    uint16_t blue(void);
    uint16_t magenta(void);
    uint16_t cyan(void);
    uint16_t white(void);

private:
    int8_t bpp_from_bpp_opt(uint8_t bpp_opt);

    uint16_t canvas_struct;
    uint16_t canvas_data;
    uint8_t  plane;
    uint8_t  canvas_type;
    uint16_t canvas_w;
    uint16_t canvas_h;
    uint16_t canvas_c;
    uint16_t canvas_r;
    uint8_t  font_w;
    uint8_t  font_h;
    int8_t   bpp;
    uint16_t display_bg;
    uint16_t display_fg;

    cursor_state_t cur_state;
    uint16_t cur_r;
    uint16_t cur_c;
    uint8_t cur_threshold; // cursor blink delay
    bool disable_cur_blinking;
};

#endif // TXTDISPLAY_H