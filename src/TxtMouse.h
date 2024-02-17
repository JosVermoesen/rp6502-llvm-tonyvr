// ---------------------------------------------------------------------------
// TxtMouse.h
// ---------------------------------------------------------------------------

#ifndef TXTMOUSE_H
#define TXTMOUSE_H

#include <cstdint>
#include <stdbool.h>
#include "TxtDisplay.h"

class TxtMouse
{
public:
    TxtMouse() {}; // constructor

    bool InitMouse(uint16_t mouse_struct_address,
                   uint16_t mouse_pointer_data_address,
                   uint16_t mouse_state_address,
                   TxtDisplay * pDisplay);

    // the following are only valid AFTER call to InitMouse():
    bool HandleMouse(void);

    // override these for application-specific behavior
    bool LeftBtnPressed(int16_t x, int16_t y);
    bool LeftBtnReleased(int16_t x, int16_t y);
    bool RightBtnPressed(int16_t x, int16_t y);
    bool RightBtnReleased(int16_t x, int16_t y);

    uint16_t mouse_x(void) {return mouse_X;};
    uint16_t mouse_y(void) {return mouse_Y;};
    uint16_t mouse_row(void) {return mouse_X/txtdisplay->font_width();};
    uint16_t mouse_col(void) {return mouse_Y/txtdisplay->font_height();};

private:
    void DrawMousePointer(void);

    uint16_t mouse_struct;
    uint16_t mouse_data;
    uint16_t mouse_state;
    uint16_t mouse_X;
    uint16_t mouse_Y;
    TxtDisplay * txtdisplay;
};

#endif // TXTMOUSE_H