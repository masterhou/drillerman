#include "menu.h"

#include <time.h>
#include <stdlib.h>
#include <math.h>

#include <SDL/SDL.h>


#include "input.h"
#include "snge.h"
#include "defs.h"
#include "message.h"
#include "mainloop.h"

#define STAR_COUNT 30

#define M_SPD 1000.0
#define R_SPD (M_PI * 1.5)

static enum
{
    ME_ARCADE,
    ME_SURVIVAL,
    ME_HOF,
    ME_EXIT,
    ME_LAST
} menuEntry;

static Sprite *menuEntries[ME_LAST];
static Sprite *stars[STAR_COUNT];

static int starw, starh;

static struct
{
    float spx;
    float spy;
    float spr;
} starp[STAR_COUNT];

static void procStars(float lag)
{
    int i;

    for(i = 0; i < STAR_COUNT; ++i)
    {
        stars[i]->x += starp[i].spx * lag;
        stars[i]->y += starp[i].spy * lag;
        stars[i]->angle += starp[i].spr * lag;

        if(stars[i]->x >= _SCREEN_WIDTH || stars[i]->y >= _SCREEN_HEIGHT || (stars[i]->x + starw) < 0 || (stars[i]->y + starh) < 0)
        {
            stars[i]->x = rand() % _SCREEN_WIDTH;
            stars[i]->y = -starh;
            starp[i].spx = (float)(rand() % 200) - 100.0;
            starp[i].spy = (float)(rand() % 50) + 50;
            starp[i].spr = (float)(rand() % 720) - 360.0;

        }
    }
}

static void procMenu(float lag)
{
        menuEntries[ME_EXIT]->angle += lag * 50.0;
}

void menu_Init(void *data)
{

    sprites_LoadFromCfg("sprites/menu.spr", "");
    sprites_LoadFontsFromCfg("fonts/fonts.desc");

    menuEntry = ME_EXIT;

    srand(time(NULL));

    graphics_SetBackground(FT_FLAT, color(0.3, 0.59, 1.0, 1), color(0, 0, 0, 0), GT_NONE);

    snge_AddSprite(sprites_GetIdByName("menu_logo"), point(100, 25), 5);

    SpriteClassId starc = sprites_GetIdByName("star");

    int i;

    for(i = 0; i < STAR_COUNT; ++i)
        stars[i] = snge_AddSprite(starc, point(-500, -500), -1);

    sprites_GetDimensions(starc, &starw, &starh);

    menuEntries[ME_ARCADE] = snge_AddFontSprite(sprites_GetIdByName("font:base"), point(0, 0), 2, "ARCADE!!arcade");
    menuEntries[ME_SURVIVAL] = snge_AddFontSprite(sprites_GetIdByName("font:base"), point(0, 70), 2, "survival");
    menuEntries[ME_HOF] = snge_AddFontSprite(sprites_GetIdByName("font:base"), point(0, 140), 2, "hall of fame");
    menuEntries[ME_EXIT] = snge_AddFontSprite(sprites_GetIdByName("font:base"), point(0, 210), 2, "exit");
}

int menu_Frame(float lag)
{
    procStars(lag);
    procMenu(lag);

    return 0;
}

void menu_Cleanup()
{
    snge_FreeSprites();
    sprites_FreeAll();
}

