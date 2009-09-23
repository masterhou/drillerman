#ifndef SNGE_H
#define SNGE_H

#include "graphics.h"
#include "sprites.h"

typedef int SpriteId;

typedef struct
{
    SpriteClassId sclass;

    float x;
    float y;

    float sx;
    float sy;

    float angle;

    float frame;
    int layer;

    float opacity;

    char hflip;
    char vflip;

    float animdir;
    bool destroy; /* destroy on spriteCleanup */
    bool aended;
    bool relative; /* position relative to screen */

    char text[256];


} Sprite;

void snge_Init();
Sprite *snge_AddSprite(SpriteClassId sprclass, Point pos, int layer);
inline Sprite *snge_AddFontSprite(SpriteClassId fontclass, Point pos, int layer, char *string);
void snge_FreeSprites();
void snge_UpdateAnim(float lag);
void snge_Draw();
Point snge_GetTextSize(Sprite *psprite);
void snge_MoveViewport(Point newpos);
void snge_RelativizeSprite(Sprite *sprite);
void snge_CleanupSprites();
void snge_SwitchAnim(Sprite *pSprite, SpriteClassId scid);

#endif
