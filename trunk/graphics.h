#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "defaults.h"
#include "common.h"

typedef enum
{
    OT_SDLOPENGL,
    OT_LAST
} OutType;

typedef enum {GT_NONE, GT_V, GT_H} GradientType;
typedef enum {FT_FLAT, FT_GRADIENT} FillType;

typedef BitmapId (*OutLoadBitmapFunc)(const char *file, int *w, int *h);
typedef void (*OutInitSubsystemFunc)(int screen_width, int screen_height);
typedef void (*OutBlitBitmapFunc)(BitmapId bitmap_id, Transformations *transfs);
typedef void (*OutSetBackground)(FillType bt, Color main_color, Color aux_color, GradientType gt);
typedef void (*OutClearBuffer)();
typedef void (*OutBlitBuffer)();
typedef void (*OutSetFadeColor)(Color fcolor);
typedef void (*OutDrawRectangle)(Point left_top, Point right_bottom, FillType ft, GradientType gt, Color main_color, Color aux_color);
typedef void (*OutBlitPartBitmap)(BitmapId textureID, Transformations *transfs, Point left_top, Point part_size);
typedef void (*OutBlitText)(BitmapId texture_id, Transformations *transfs, const char *text, Font font);

OutLoadBitmapFunc graphicsLoadBitmap;
OutBlitBitmapFunc graphicsBlitBitmap;
OutBlitPartBitmap graphicsBlitPartBitmap;
OutSetBackground graphicsSetBackground;
OutClearBuffer graphicsClearBuffer;
OutBlitBuffer graphicsBlitBuffer;
OutSetFadeColor graphicsSetFadeColor;
OutDrawRectangle graphicsDrawRectangle;
OutBlitText graphicsBlitText;

void graphicsInitSubsytem(int screen_width, int screen_height);

#endif
