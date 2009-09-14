#include "graphics.h"

#include <math.h>
#include <string.h>

#include "glout.h"
#include "glesout.h"

#include "message.h"

OutType outType = OT_SDLOPENGLVBO;

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
    },
    {
        loadBitmap: &glesoutLoadBitmap,
        initSubsystem: &glesoutInitSubsystem,
        blitBitmap: &glesoutBlitBitmap,
        setBackground: &glesoutSetBackground,
        clearBuffer: &glesoutClearBuffer,
        blitBuffer: &glesoutBlitBuffer,
        setFadeColor: &glesoutSetFadeColor,
        drawRectangle: &glesoutDrawRectangle,
        blitPartBitmap: &glesoutBlitPartBitmap,
        blitText: &glesoutBlitText
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



