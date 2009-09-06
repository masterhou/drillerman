#include "snge.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "message.h"
#include "defaults.h"

#define OUTSIDE_SCREEN(tx, ty, tw, th) (						\
        ((_SCREEN_WIDTH - 1) < (tx)) || 		\
        ((_SCREEN_HEIGHT - 1) < (ty)) || 		\
        (0 > ((tx) + (tw) - 1)) || 			\
        (0 > ((ty) + (th) - 1))			\
        )

static int sngeSCount;
static Sprite **Sprites;
static SpriteId highestId;
static Point viewportPos;

void sngeRelativizeSprite(Sprite *sprite)
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

    for(i = 0; i < (sngeSCount - 1); ++i)
    {
        change = 0;

        for(j = 0; j < (sngeSCount - 1 - i); ++j)
            if(Sprites[j + 1]->layer < Sprites[j]->layer)
            {
                tmp = Sprites[j + 1];
                Sprites[j + 1] = Sprites[j];
                Sprites[j] = tmp;
                change = 1;
            }

        if(!change) return;
    }

}

static int getIndexById(SpriteId sid)
{

    if(sngeSCount == 0 || sid > highestId)
        return -1;

    int i;

    for(i = 0; i < sngeSCount; ++i)
        if(Sprites[i]->sid == sid)
            return i;

    return -1;

}


void sngeInit()
{
    sngeSCount = 0;
    Sprites = NULL;
    highestId = 0x1234;

    viewportPos = point(0, 0);
}

void sngeMoveViewport(Point newpos)
{
    viewportPos = newpos;
}

Sprite *sngeAddSprite(SpriteClassId sprclass, Point pos, int layer)
{
    sngeSCount++;
    highestId++;

    Sprites = realloc(Sprites, sizeof(Sprite*) * sngeSCount);

    Sprites[sngeSCount - 1] = malloc(sizeof(Sprite));

    Sprite *s = Sprites[sngeSCount - 1];

    s->sid = highestId;
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

inline Sprite *sngeAddFontSprite(SpriteClassId fontclass, Point pos, int layer, char *string)
{
    Sprite *s = sngeAddSprite(fontclass, pos, layer);
    strcpy(s->text, string);

    return s;
}


Sprite *sngeGetSpriteById(SpriteId sid)
{
    int i = getIndexById(sid);

    if(i > -1)
        return Sprites[i];

    return NULL;
}

static void remSpriteByIndex(int index)
{
    free(Sprites[index]);

    int i;

    for(i = index; i < (sngeSCount - 1); ++i)
        Sprites[i] = Sprites[i + 1];

    sngeSCount--;

    if(sngeSCount == 0)
    {
        free(Sprites);
        Sprites = NULL;
    }
    //else
        //Sprites = realloc(Sprites, sizeof(Sprite*) * sngeSCount);
}

void sngeRemSprite(SpriteId sid)
{
    int s = getIndexById(sid);

    if(s == -1) return;

    remSpriteByIndex(s);
}

void sngeFreeSprites()
{
    if(!Sprites) return;

    int i;

    for(i = 0; i < sngeSCount; ++i)
        free(Sprites[i]);

    free(Sprites);

    sngeSCount = 0;
    Sprites = NULL;
}

Point sngeGetTextSize(Sprite *psprite)
{
    SpriteClass *sc = &spritesClasses[psprite->sclass];
    Point sz = {
        sc->font.char_width * strlen(psprite->text),
        sc->font.char_height
    };

    return sz;
}

void sngeUpdateAnim(float lag)
{
    int i;

    Sprite **stmp = malloc(sizeof(Sprite*) * sngeSCount);
    int scount = 0;

    bool delNeeded = false;

    for(i = 0; i < sngeSCount; ++i)
    {
        SpriteClass *sc = &spritesClasses[Sprites[i]->sclass];
        Sprite *sp = Sprites[i];

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
                    sp->aended = 1;
                    if(sp->destroy)
                        delNeeded = true;
                }

            }

            if(sp->frame < 0)
            {
                sp->animdir = 1.0;
                sp->frame -= delta;
            }
        }

    }

    if(delNeeded)
    {
        for(i = 0; i < sngeSCount; ++i)
        {
            Sprite *sp = Sprites[i];

            if(!(sp->aended && sp->destroy))
            {
                stmp[scount] = sp;
                scount++;
            }
        }

        sngeSCount = scount;
        free(Sprites);
        Sprites = stmp;
    }
}

void sngeDraw()
{
    sortByLayer();

    int i;
    int oldlayer = -100;

    for(i = 0; i < sngeSCount; ++i)
    {
        Sprite *s = Sprites[i];
        SpriteClassId scid = s->sclass;
        SpriteClass *sc = &spritesClasses[scid];

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





