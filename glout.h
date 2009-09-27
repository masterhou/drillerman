#ifndef GLOUT_H
#define GLOUY_H

#include <GL/gl.h>
#include <GL/glu.h>

#include "defs.h"
#include "common.h"
#include "graphics.h"

#define FILTER_LINEAR 0
#define FILTER_MIPMAP 1
#define FILTER_NEAREST 2

void gloutInitSubsystem(int screen_width, int screen_height);

void gloutBlitBitmap(BitmapId textureID, Transformations *transfs);
void gloutBlitPartBitmap(BitmapId textureID, Transformations *transfs, IntPoint *leftTop, IntPoint *partSize);
BitmapId gloutLoadBitmap(const char *file, int *w, int *h);
void gloutSetBackground(FillType bt, Color mainColor, Color auxColor, GradientType gt);
void gloutClearBuffer();
void gloutBlitBuffer();
void gloutSetFadeColor(Color fcolor);
void gloutDrawRectangle(Point left_top, Point right_bottom, FillType ft, GradientType gt, Color mainColor, Color auxColor);


#endif
