#include "snge.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "message.h"
#include "defs.h"

#define OUTSIDE_SCREEN(tx, ty, tw, th) (						\
        ((_SCREEN_WIDTH - 1) < (tx)) || 		\
        ((_SCREEN_HEIGHT - 1) < (ty)) || 		\
        (0 > ((tx) + (tw) - 1)) || 			\
        (0 > ((ty) + (th) - 1))			\
        )

static int count;
static int capacity;
static Sprite **sprites;
static Point viewportPos;

void snge_RelativizeSprite(Sprite *sprite)
{
    if(sprite->relative)
        return;

    sprite->relative = true;
    sprite->x -= viewportPos.x;
    sprite->y -= viewportPos.y;
}

static void sortByLayer()
{
    int i, j;
    int change;
    Sprite *tmp;

    for(i = 0; i < (count - 1); ++i)
    {
        change = 0;

        for(j = 0; j < (count - 1 - i); ++j)
            if(sprites[j + 1]->layer < sprites[j]->layer)
            {
                tmp = sprites[j + 1];
                sprites[j + 1] = sprites[j];
                sprites[j] = tmp;
                change = 1;
            }

        if(!change) return;
    }

}

void snge_Init()
{
    count = 0;
    capacity = _SPRITE_ENGINE_CAPACITY_OVERHEAD;
    sprites = malloc(sizeof(Sprite*) * capacity);
    viewportPos = point(0, 0);
}

void snge_MoveViewport(Point newpos)
{
    viewportPos = newpos;
}

Sprite *snge_AddSprite(SpriteClassId sprclass, Point pos, int layer)
{
    count++;

    if(count == capacity)
    {
        capacity += _SPRITE_ENGINE_CAPACITY_OVERHEAD;
        sprites = realloc(sprites, sizeof(Sprite*) * capacity);
    }

    sprites[count - 1] = malloc(sizeof(Sprite));

    Sprite *s = sprites[count - 1];

    s->sclass = sprclass;
    s->x = pos.x;
    s->y = pos.y;
    s->animdir = 1.0;
    s->destroy = false;

    s->layer = layer;
    s->opacity = 1.0;

    s->sx = 1;
    s->sy = 1;
    s->angle = 0;
    s->frame = 0;

    s->hflip = 0;
    s->vflip = 0;

    s->aended = false;
    s->relative = false;

    return s;
}

inline Sprite *snge_AddFontSprite(SpriteClassId fontclass, Point pos, int layer, char *string)
{
    Sprite *s = snge_AddSprite(fontclass, pos, layer);
    strcpy(s->text, string);

    return s;
}


void snge_FreeSprites()
{
    if(!sprites) return;

    int i;

    for(i = 0; i < count; ++i)
        free(sprites[i]);

    free(sprites);

    count = 0;
    sprites = NULL;
}

Point snge_GetTextSize(Sprite *psprite)
{
    SpriteClass *sc = sprites_GetClass(psprite->sclass);
    Point sz = {
        sc->font.char_width * strlen(psprite->text),
        sc->font.char_height
    };

    return sz;
}

void snge_CleanupSprites()
{
    Sprite **stmp = malloc(sizeof(Sprite*) * capacity);
    int scount = 0;
    int i;

    for(i = 0; i < count; ++i)
    {
        Sprite *sp = sprites[i];
        const bool destroy = sp->destroy;

        if(destroy)
        {
            free(sp);
        }
        else
        {
            stmp[scount] = sp;
            scount++;
        }
    }

    count = scount;
    free(sprites);
    sprites = stmp;
}

void snge_UpdateAnim(float lag)
{
    int i;

    for(i = 0; i < count; ++i)
    {
        SpriteClass *sc = sprites_GetClass(sprites[i]->sclass);
        Sprite *sp = sprites[i];

        int fc = sc->fcount;
        double fps = sc->fps;

        if(sp->angle > 360)
            sp->angle = fmodf(sp->angle, 360.0);

        if(sc->ssc == SSC_ANIM)
        {
            float delta = lag * fps * sp->animdir;
            sp->frame += delta;

            if(sp->frame >= fc)
            {
                if(sc->arepeat)
                {
                    if(sc->areverse)
                    {
                        sp->animdir = -1.0;
                        sp->frame -= delta;
                    }
                    else
                        sp->frame -= fc;
                }
                else
                {
                    sp->frame = fc - 1;
                    sp->aended = true;
                }

            }

            if(sp->frame < 0)
            {
                sp->animdir = 1.0;
                sp->frame -= delta;
            }
        }

    }
}

void snge_Draw()
{
    sortByLayer();

    int i;
    int oldlayer = -100;

    for(i = 0; i < count; ++i)
    {
        Sprite *s = sprites[i];
        SpriteClassId scid = s->sclass;
        SpriteClass *sc = sprites_GetClass(scid);

        int frame = (int)floorf(s->frame);

        if(frame >= sc->fcount)
        {
        /*
            Ideally this shouldn't be reached.
            Probably sprite's class has been
            changed but without resetting frame
            counter.
        */
            frame = 0;
        }

        if(sc->ssc == SSC_STILL)
            frame = 0;

        float bw;
        float bh;

        if(sc->ssc == SSC_BFNT)
        {
            bw = sc->font.char_width * strlen(s->text);
            bh = sc->font.char_height;
        }
        else
        {
            bw = (float)sc->frame[frame].w;
            bh = (float)sc->frame[frame].h;
        }

        float w = bw * s->sx;
        float h = bh * s->sy;

        //TODO: obrot i bounding box !!! (test kolizji z ekranem...)

        Transformations transfs = {
                                    trans: {x: s->x, y: s->y},
                                    size: {x: (float)sc->frame[frame].w, y: (float)sc->frame[frame].h},
                                    scale: {x: s->sx, y: s->sy},
                                    angle: (int)floorf(s->angle),
                                            s->opacity,
                                            s->vflip,
                                            s->hflip
                                    };

        if(!s->relative)
        {
            transfs.trans.x -= viewportPos.x;
            transfs.trans.y -= viewportPos.y;
        }

        if(!OUTSIDE_SCREEN(transfs.trans.x, transfs.trans.y, w, h))
        {
            if(sc->ssc == SSC_BFNT)
                graphicsBlitText(sc->frame->image, &transfs, s->text, sc->font);
            else
                graphicsBlitBitmap(sc->frame[frame].image, &transfs);
        }

        oldlayer = s->layer;
    }


}





