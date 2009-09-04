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

void spritesInit();

SpriteClassId spritesGetIdByName(const char *name);
SpriteClassId spritesGetIdByNameF(const char *namefmt, ...);

void spritesLoadFromCfg(const char *cfgpathrel);
void spritesLoadFromCfgF(const char *cfgpathrelfmt, ...);

void spritesLoadFontsFromCfg(char *cfgpathrel);
void spritesFreeAll();

void spritesGetDimensions(SpriteClassId id, int *width, int *height);

extern SpriteClass *spritesClasses;


#endif
