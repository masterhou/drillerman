#ifndef GLESOUT_H
#define GLESOUT_H

#include <GL/gl.h>
#include <GL/glu.h>

#include "defaults.h"
#include "graphics.h"
#define FILTER_LINEAR 0
#define FILTER_MIPMAP 1
#define FILTER_NEAREST 2

void glesoutInitSubsystem(int screen_width, int screen_height);
void glesoutBlitBitmap(BitmapId textureID, Transformations *transfs);
void glesoutBlitPartBitmap(BitmapId textureID, Transformations *transfs, Point left_top, Point part_size);
BitmapId glesoutLoadBitmap(const char *file, int *w, int *h);
void glesoutSetBackground(FillType bt, Color main_color, Color aux_color, GradientType gt);
void glesoutClearBuffer();
void glesoutBlitBuffer();
void glesoutSetFadeColor(Color fcolor);
void glesoutDrawRectangle(Point left_top, Point right_bottom, FillType ft, GradientType gt, Color main_color, Color aux_color);
void glesoutBlitText(BitmapId texture_id, Transformations *transfs, const char *text, Font font);

#endif
