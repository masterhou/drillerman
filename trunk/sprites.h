#ifndef SPRITES_H
#define SPRITES_H

#include <GL/gl.h>

#include "graphics.h"
#include "common.h"

typedef int SpriteClassId;

typedef enum {SSC_ANIM, SSC_STILL, SSC_BFNT} SpriteSubClass;

typedef struct 
{

    BitmapId image;
    int w;
    int h;

} SpriteFrame;

typedef struct
{
    char *name;
    int fcount;
    double fps;
    int areverse;
    int arepeat;
    SpriteFrame *frame;
    SpriteSubClass ssc;
    Font font;

} SpriteClass;

void sprites_Init();

SpriteClassId sprites_GetIdByName(const char *name);
SpriteClassId sprites_GetIdByNameF(const char *namefmt, ...);

void sprites_LoadFromCfg(const char *cfgpathrel, const char *namePrefix);
void sprites_LoadFromCfgF(const char *cfgpathrelfmt, const char *namePrefix, ...);

void sprites_LoadFontsFromCfg(char *cfgpathrel);
void sprites_FreeAll();

void sprites_GetDimensions(SpriteClassId id, int *width, int *height);
SpriteClass* sprites_GetClass(SpriteClassId id);


#endif
