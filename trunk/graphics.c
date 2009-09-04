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
    OutBlitText blitText;

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
        blitText: &gloutBlitText
    }
};

void graphicsInitSubsytem(int screen_width, int screen_height)
{
    outFuncs[outType].initSubsystem(screen_width, screen_height);
    graphicsLoadBitmap = outFuncs[outType].loadBitmap;
    graphicsBlitBitmap = outFuncs[outType].blitBitmap;
    graphicsBlitPartBitmap = outFuncs[outType].blitPartBitmap;
    graphicsSetBackground = outFuncs[outType].setBackground;
    graphicsClearBuffer = outFuncs[outType].clearBuffer;
    graphicsBlitBuffer = outFuncs[outType].blitBuffer;
    graphicsSetFadeColor = outFuncs[outType].setFadeColor;
    graphicsDrawRectangle = outFuncs[outType].drawRectangle;
    graphicsBlitText = outFuncs[outType].blitText;
}



