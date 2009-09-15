#include "bcg.h"

#include <stdlib.h>

#include "sprites.h"
#include "snge.h"

typedef struct
{
    SpriteClassId scid;
    Sprite **sprites;
    int count;
    int bottom;
    float height;
} GameBcgLayer;

GameBcgLayer layers[_BCG_LAYER_COUNT];

void bcg_Init()
{
    int i, j;

    for(i = 0; i < _BCG_LAYER_COUNT; ++i)
    {
        SpriteClassId scid = sprites_GetIdByNameF("level_0:bcg-layer-%d", i);
        
        int width;
        int height;

        sprites_GetDimensions(scid, &width, &height);

        int xpos = (_ACTION_AREA_WIDTH - width) / 2;

        int count = (_SCREEN_HEIGHT / height) + 2;
        Sprite **sprites = malloc(sizeof(Sprite*) * count);

        for(j = 0; j < count; ++j)
        {
            sprites[j] = snge_AddSprite(scid, point(xpos, (j - 1) * height), i);
            sprites[j]->relative = true;
        }

        layers[i].bottom = count - 1;
        layers[i].sprites = sprites;
        layers[i].count = count;
        layers[i].height = (float)height;
    }
}

void bcg_Move(float vdelta)
{
    int i, j;

    for(i = 0; i < _BCG_LAYER_COUNT; ++i)
    {
        GameBcgLayer *l = &layers[i];
        float pfactor = 1.0 - ((float)(_BCG_LAYER_COUNT - i) * _BCG_PARALLAX_FACTOR);

        for(j = 0; j < l->count; ++j)
            l->sprites[j]->y += vdelta * pfactor;


        float bottomy = l->sprites[l->bottom]->y + l->height;

        if(bottomy <= _SCREEN_HEIGHT)
        {
            int newbottom = (l->bottom + 1) % l->count;
            l->sprites[newbottom]->y = bottomy;
            l->bottom = newbottom;
        }
    }

}

void bcg_Cleanup()
{
    int i, j;
    for(i = 0; i < _BCG_LAYER_COUNT; ++i)
    {
        for(j = 0; j < layers[i].count; ++j)
            layers[i].sprites[j]->destroy = true;

        free(layers[i].sprites);
    }
}
