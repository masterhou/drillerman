#include "settings.h"

#include <math.h>

#include "sprites.h"
#include "snge.h"
#include "mainloop.h"
#include "defs.h"
#include "common.h"
#include "particles.h"
#include "input.h"

#define _H_PADD 70.0
#define _V_PADD 40.0
#define _V_SPAC 20.0
#define _POINTER_SPEED 200.0
#define _POINTER_V_PADD 5.0

typedef enum {S_FULLSCREEN, S_RESOLUTION, S_SFX, S_MUSIC, S_KEYS, S_EXIT, S_LAST} Settings;

static int selected;
static Sprite *labels[S_LAST];
static Sprite *values[S_LAST];
static Sprite *pointer;

static SpriteClassId scidParticle;

static Point ptPos;
static Point ptShift;
static int ptDir;

static float partTimer;

static float tH;

void settings_Init(void *data)
{
    sprites_LoadFontsFromCfg("fonts/fonts.desc");
    sprites_LoadFromCfg("sprites/settings/settings.spr", "");

    scidParticle = sprites_GetIdByName("particle");

    SpriteClassId baseFNT = sprites_GetIdByName("font:base");
    SpriteClassId baseFNTalt = sprites_GetIdByName("font:base-alt");

    labels[S_FULLSCREEN] = snge_AddFontSprite(baseFNT, point(0, 0), 10, "Fullscreen");
    labels[S_RESOLUTION] = snge_AddFontSprite(baseFNT, point(0, 0), 10, "Resolution");
    labels[S_KEYS] = snge_AddFontSprite(baseFNT, point(0, 0), 10, "Controls");
    labels[S_SFX] = snge_AddFontSprite(baseFNT, point(0, 0), 10, "Play SFX");
    labels[S_MUSIC] = snge_AddFontSprite(baseFNT, point(0, 0), 10, "Play Music");
    labels[S_EXIT] = snge_AddFontSprite(baseFNT, point(0, 0), 10, "Back");

    values[S_FULLSCREEN] = snge_AddFontSprite(baseFNTalt, point(0, 0), 10, "Yes");
    values[S_RESOLUTION] = snge_AddFontSprite(baseFNTalt, point(0, 0), 10, "1024x768");
    values[S_KEYS] = snge_AddFontSprite(baseFNTalt, point(0, 0), 10, "");
    values[S_SFX] = snge_AddFontSprite(baseFNTalt, point(0, 0), 10, "Yes");
    values[S_MUSIC] = snge_AddFontSprite(baseFNTalt, point(0, 0), 10, "Yes");
    values[S_EXIT] = snge_AddFontSprite(baseFNTalt, point(0, 0), 10, "");

    pointer = snge_AddSprite(sprites_GetIdByName("pointer"), point(0, 0), 11);

    ptPos = point(20.0, _V_PADD + _POINTER_V_PADD);
    ptShift = point(0, 0);
    ptDir = 0;

    partTimer = 0.0;

    selected = 0;

    int i;

    IntPoint cS = snge_GetTextSize(values[0]);
    tH = (float)cS.y + _V_SPAC;

    for(i = 0; i < S_LAST; ++i)
    {
        cS = snge_GetTextSize(values[i]);

        float y = _V_PADD + tH * (float)i;

        labels[i]->x = _H_PADD;
        labels[i]->y = y;
        labels[i]->relative = true;

        values[i]->x = (float)_SCREEN_WIDTH - _H_PADD - (float)cS.x;
        values[i]->y = y;
        values[i]->relative = true;
    }
}

void emitParticles(float lag)
{

    partTimer += lag;
    if(partTimer >= 0.1)
    {
        partTimer -= 0.1;

        Point p = point(pointer->x + 35.0, pointer->y + 14.0);
        int i;
        for(i = 0; i < 10; ++i)
        {
            Sprite *s = snge_AddSprite(scidParticle, p, 5);
            Particle *p = particles_Add(s);
            particles_SetVelocityDegrees(p, 130.0 + common_RandD() * 80.0, 50.0, true);
            //particles_SetTrail(p, 1.0, 2.0);
            particles_SetFading(p, 0.5, true);
        }

    }
}

int settings_Frame(float lag)
{
    if(input_IsDirPressed(DIR_DOWN) && ptDir == 0 && selected < (S_LAST - 1))
    {
        ptDir = 1;
    }

    if(input_IsDirPressed(DIR_UP) && ptDir == 0 && selected > 0)
    {
        ptDir = -1;
    }

    ptShift.y += (float)ptDir * _POINTER_SPEED * lag;

    if(fabs(ptShift.y) >= tH)
    {
        selected += ptDir;
        ptShift.y = 0;
        ptPos.y = _V_PADD + _POINTER_V_PADD + tH * (float)selected;
        ptDir = 0;
    }

    pointer->x = ptPos.x + ptShift.x;
    pointer->y = ptPos.y + ptShift.y;

    emitParticles(lag);

    return 0;
}

void settings_Cleanup()
{
    snge_FreeSprites();
    sprites_FreeAll();
}
