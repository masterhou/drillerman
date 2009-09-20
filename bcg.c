#include "bcg.h"

#include <stdlib.h>

#include "sprites.h"

Bcg bcg_Create(float vShift, int levelNum)
{
    int i, j;
    Bcg bcg;

    bcg.slideOut = false;

    for(i = 0; i < _BCG_LAYER_COUNT; ++i)
    {
        SpriteClassId scid = sprites_GetIdByNameF("level_%d:bcg-layer-%d", levelNum, i);
        
        int width;
        int height;

        sprites_GetDimensions(scid, &width, &height);

        int xpos = (_ACTION_AREA_WIDTH - width) / 2;
        int count = (_SCREEN_HEIGHT / height) + 2;

        Sprite **sprites = malloc(sizeof(Sprite*) * count);

        for(j = 0; j < count; ++j)
        {
            sprites[j] = snge_AddSprite(scid, point(xpos, (j - 1) * height + vShift), i);
            sprites[j]->relative = true;
        }

        bcg.layers[i].bottom = count - 1;
        bcg.layers[i].sprites = sprites;
        bcg.layers[i].count = count;
        bcg.layers[i].height = (float)height;
    }

    return bcg;
}

void bcg_Move(Bcg *pBcg, float vdelta)
{
    int i, j;

    for(i = 0; i < _BCG_LAYER_COUNT; ++i)
    {
        BcgLayer *l = &pBcg->layers[i];
        float pfactor = 1.0 - ((float)(_BCG_LAYER_COUNT - i) * _BCG_PARALLAX_FACTOR);

        for(j = 0; j < l->count; ++j)
        {
            l->sprites[j]->y += vdelta * pfactor;
        }

        if(pBcg->slideOut)
            continue;

        float bottomy = l->sprites[l->bottom]->y + l->height;

        if(bottomy <= _SCREEN_HEIGHT)
        {
            int newbottom = (l->bottom + 1) % l->count;
            l->bottom = newbottom;
            l->sprites[newbottom]->y = bottomy;
        }
    }
}

void bcg_Cleanup(Bcg *pBcg)
{
    int i, j;
    for(i = 0; i < _BCG_LAYER_COUNT; ++i)
    {
        for(j = 0; j < pBcg->layers[i].count; ++j)
            pBcg->layers[i].sprites[j]->destroy = true;

        free(pBcg->layers[i].sprites);
    }
}
