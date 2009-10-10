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
    Sprite *tmp;

    for(i = 1; i < count; ++i)
    {
        tmp = sprites[i];
        j = i - 1;

        while(j >= 0 && sprites[j]->layer > tmp->layer)
        {
            sprites[j + 1] = sprites[j];
            j--;
        }

        sprites[j + 1] = tmp;
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

    s->gaugeType = GAUGE_NONE;

    return s;
}

Sprite *snge_AddGaugeSprite(SpriteClassId sprclass, Point pos, int layer, GaugeType gaugeType)
{
    Sprite *s = snge_AddSprite(sprclass, pos, layer);
    s->gaugeType = gaugeType;
    s->gaugeFill = 0.0;

    return s;
}

Sprite *snge_AddFontSprite(SpriteClassId fontclass, Point pos, int layer, char *string)
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

    count = 0;
}

IntPoint snge_GetTextSize(Sprite *psprite)
{
    SpriteClass *sc = sprites_GetClass(psprite->sclass);

    IntPoint sz = {
        sc->font->charSize.x * strlen(psprite->text),
        sc->font->charSize.y
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

void snge_SwitchAnim(Sprite *pSprite, SpriteClassId scid)
{
    pSprite->sclass = scid;
    pSprite->frame = 0.0;
    pSprite->aended = false;
    pSprite->animdir = 1.0;
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

        if(sp->angle > 360.0)
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

            if(sp->frame <= 0.0)
            {
                /* If animation is reversed but not repeated then signal end
                   once it was reversed and is back to first frame. */
                if(sc->areverse && sp->animdir < 0.0 && !sc->arepeat)
                    sp->aended = true;

                sp->animdir = 1.0;
                sp->frame = -sp->frame;
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
            bw = sc->font->charSize.x * strlen(s->text);
            bh = sc->font->charSize.y;
        }
        else
        {
            bw = (float)sc->frame[frame].w;
            bh = (float)sc->frame[frame].h;
        }

        float w = bw * s->sx;
        float h = bh * s->sy;

        if(s->angle >= 360.0)
            s->angle = fmodf(s->angle, 360.0);

        //TODO: rotation -> bbox collision test!

        Transformations transfs = {
                                    trans: {x: s->x, y: s->y},
                                    size:  {x: sc->frame[frame].w, y: sc->frame[frame].h},
                                    scale: {x: s->sx, y: s->sy},
                                    angle:  s->angle,
                                    opacity:s->opacity,
                                    vflip:  s->vflip,
                                    hflip:  s->hflip
                                    };

        if(!s->relative)
        {
            transfs.trans.x -= viewportPos.x;
            transfs.trans.y -= viewportPos.y;
        }

        if(!OUTSIDE_SCREEN(transfs.trans.x, transfs.trans.y, w, h))
        {
            if(sc->ssc == SSC_BFNT)
            {
                unsigned char *c = (unsigned char*)s->text;
                Font *pf = sc->font;

                while(*c != '\0')
                {
                    IntPoint *cp = &pf->charPosLut[*c];

                    if(cp->x >= 0)
                    {
                        graphics_BlitPartBitmap(sc->frame[0].image, &transfs, cp, &pf->charSize);
                    }

                    c++;
                    transfs.trans.x += (float)pf->charSize.x + (float)pf->spacing.x;
                }

            }
            else
            {
                if(s->gaugeType != GAUGE_NONE)
                {

                    IntPoint from = {0, 0};
                    IntPoint sz = {bw, bh};

                    switch(s->gaugeType)
                    {
                        case GAUGE_LR:
                            sz.x = (int)((float)bw * s->gaugeFill);
                            graphics_BlitPartBitmap(sc->frame[frame].image, &transfs, &from, &sz);
                        break;

                        case GAUGE_RL:
                            sz.x = (int)((float)bw * s->gaugeFill);
                            from.x = bw - sz.x;
                            transfs.trans.x += (float)from.x;
                            graphics_BlitPartBitmap(sc->frame[frame].image, &transfs, &from, &sz);
                        break;

                        case GAUGE_UD:
                            sz.y = (int)((float)bh * s->gaugeFill);
                            graphics_BlitPartBitmap(sc->frame[frame].image, &transfs, &from, &sz);
                        break;

                        case GAUGE_DU:
                            sz.y = (int)((float)bh * s->gaugeFill);
                            from.y = bh - sz.y;
                            transfs.trans.y += (float)from.y;
                            graphics_BlitPartBitmap(sc->frame[frame].image, &transfs, &from, &sz);
                        break;
                    }
                }
                else
                    graphics_BlitBitmap(sc->frame[frame].image, &transfs);
            }
        }

        oldlayer = s->layer;
    }


}





