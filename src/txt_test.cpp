// ---------------------------------------------------------------------------
// txt_test.cpp
// ---------------------------------------------------------------------------

#include <rp6502.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <stdbool.h>
#include "TxtDisplay.h"
#include "TxtKeyboard.h"
#include "TxtMouse.h"

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
static void DrawText(TxtDisplay & txtdisplay)
{
    puts("Yay! I no longer have to declare local variables at top of scope!");

    uint16_t i, j, fg=0, bg=0;
    for (i = 0; i < txtdisplay.canvas_rows(); i++) {
        for (j = 0; j < txtdisplay.canvas_cols(); j++) {
            txtdisplay.DrawChar(i, j, 'a'+i, (bg+j)%16, (fg+i)%16);
        }
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
int main(void)
{
    puts("Hello from txt_test");

    TxtDisplay txtdisplay;
    TxtKeyboard txtkeyboard;
    TxtMouse txtmouse;

    //txtdisplay.InitDisplay(); // canvas=3 (640x480), font_opt=1 (8x16), bpp_opt=2 (4bpp)
    //txtdisplay.InitDisplay(0,0,0,3,10); // canvas=3 (640x480), font_opt=1 (8x16), bpp_opt=2 (4bpp)
    txtdisplay.InitDisplay(0,0,0,1,10); // canvas=1 (320x240), font_opt=1 (8x16), bpp_opt=2 (4bpp)
    //txtdisplay.InitDisplay(0,0,0,1,3); // canvas=1 (320x240), font_opt=0 (8x8), bpp_opt=3 (8bpp)
    //txtdisplay.InitDisplay(0,0,0,2,9); // canvas=2 (320x180), font_opt=1 (8x16), bpp_opt=1 (-4bpp)
    //txtdisplay.InitDisplay(0,0,0,4,12); // canvas=4 (640x360), font_opt=1 (8x16), bpp_opt=4 (16bpp) //??Not working??

    txtdisplay.DisableCursorBlinking();
    DrawText(txtdisplay);
    txtdisplay.EnableCursorBlinking();

    if (txtkeyboard.InitKeyboard(0, &txtdisplay) &&
        txtmouse.InitMouse(0, 0, 0, &txtdisplay)   ) {
        uint8_t v; // vsync counter, incements every 1/60 second, rolls over every 256

        // vsync loop
        v = RIA.vsync;
        while(true) {
            if (v == RIA.vsync) {
                continue; // wait until vsync is incremented
            } else {
                v = RIA.vsync; // new value for v
            }
            txtdisplay.HandleDisplay();
            txtmouse.HandleMouse();
            txtkeyboard.HandleKeys();
        }
    }

    //exit
    puts("Goodbye!");
}
