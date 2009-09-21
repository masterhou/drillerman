#ifndef BCG_H
#define BCG_H

#include "defs.h"
#include "snge.h"
#include "common.h"

typedef struct
{
    SpriteClassId scid;
    Sprite **sprites;
    int count;
    int bottom;
    float height;
} BcgLayer;

typedef struct
{
    BcgLayer layers[_BCG_LAYER_COUNT];
    float vShift;
    bool slideOut;
    Sprite *startImg;
    float startImgHeight;
}  Bcg;

Bcg bcg_Create(float vShift, int levelNum);
void bcg_Move(Bcg *pBcg, float vdelta);
void bcg_Cleanup(Bcg *pBcg);
void bcg_Relativize(Bcg *pBcg);

#endif
