#include "graphics.h"

#include <math.h>
#include <string.h>

#include "glout.h"

#include "message.h"

OutType outType = OT_SDLOPENGL;

typedef struct
{
    OutLoadBitmapFunc loadBitmap;
    OutInitSubsystemFunc initSubsystem;
    OutBlitBitmapFunc blitBitmap;
    OutSetBackground setBackground;
    OutClearBuffer clearBuffer;
    OutBlitBuffer blitBuffer;
    OutSetFadeColor setFadeColor;
    OutDrawRectangle drawRectangle;
    OutBlitPartBitmap blitPartBitmap;
} OutFuncs;

OutFuncs outFuncs[OT_LAST] = {
    {
        loadBitmap: &gloutLoadBitmap,
        initSubsystem: &gloutInitSubsystem,
        blitBitmap: &gloutBlitBitmap,
        setBackground: &gloutSetBackground,
        clearBuffer: &gloutClearBuffer,
        blitBuffer: &gloutBlitBuffer,
        setFadeColor: &gloutSetFadeColor,
        drawRectangle: &gloutDrawRectangle,
        blitPartBitmap: &gloutBlitPartBitmap,
    },
};

void graphics_Init(int screen_width, int screen_height)
{
    outFuncs[outType].initSubsystem(screen_width, screen_height);
    graphics_LoadBitmap = outFuncs[outType].loadBitmap;
    graphics_BlitBitmap = outFuncs[outType].blitBitmap;
    graphics_BlitPartBitmap = outFuncs[outType].blitPartBitmap;
    graphics_SetBackground = outFuncs[outType].setBackground;
    graphics_ClearBuffer = outFuncs[outType].clearBuffer;
    graphics_BlitBuffer = outFuncs[outType].blitBuffer;
    graphics_SetFadeColor = outFuncs[outType].setFadeColor;
    graphics_DrawRectangle = outFuncs[outType].drawRectangle;
}



