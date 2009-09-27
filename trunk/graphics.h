#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "defs.h"
#include "common.h"

typedef enum
{
    OT_SDLOPENGL,
    OT_SDLOPENGLVBO,
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
typedef void (*OutBlitPartBitmap)(BitmapId textureID, Transformations *transfs, IntPoint *leftTop, IntPoint *partSize);

OutLoadBitmapFunc graphics_LoadBitmap;
OutBlitBitmapFunc graphics_BlitBitmap;
OutBlitPartBitmap graphics_BlitPartBitmap;
OutSetBackground graphics_SetBackground;
OutClearBuffer graphics_ClearBuffer;
OutBlitBuffer graphics_BlitBuffer;
OutSetFadeColor graphics_SetFadeColor;
OutDrawRectangle graphics_DrawRectangle;

void graphics_Init(int screen_width, int screen_height);

#endif
