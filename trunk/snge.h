#ifndef SNGE_H
#define SNGE_H

#include "graphics.h"
#include "sprites.h"

typedef int SpriteId;

typedef struct
{
    SpriteId sid;
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
    bool destroy; /* destroy on animation end */
    bool aended;
    bool relative; /* position relative to screen */

    char text[256];


} Sprite;

void sngeInit();
Sprite *sngeAddSprite(SpriteClassId sprclass, Point pos, int layer);
inline Sprite *sngeAddFontSprite(SpriteClassId fontclass, Point pos, int layer, char *string);
Sprite *sngeGetSpriteById(SpriteId sid);
void sngeRemSprite(SpriteId sid);
void sngeFreeSprites();
void sngeUpdateAnim(float lag);
void sngeDraw();
Point sngeGetTextSize(Sprite *psprite);
void sngeMoveViewport(Point newpos);
void sngeRelativizeSprite(Sprite *sprite);




#endif
