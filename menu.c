#include "menu.h"

#include <time.h>
#include <stdlib.h>
#include <math.h>

#include <SDL/SDL.h>


#include "input.h"
#include "snge.h"
#include "defaults.h"
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
    if(inputKeyState[SDLK_LEFT])
        ;

    if(inputKeyState[SDLK_RIGHT])
        ;

        menuEntries[ME_EXIT]->angle += lag * 50.0;
}

void menuInit(void *data)
{

    spritesLoadFromCfg("sprites/menu.spr");
    spritesLoadFontsFromCfg("fonts/fonts.desc");

    menuEntry = ME_EXIT;

    srand(time(NULL));

    graphicsSetBackground(FT_FLAT, color(0.3, 0.59, 1.0, 1), color(0, 0, 0, 0), GT_NONE);

    sngeAddSprite(spritesGetIdByName("menu_logo"), point(100, 25), 5);

    SpriteClassId starc = spritesGetIdByName("star");

    int i;

    for(i = 0; i < STAR_COUNT; ++i)
        stars[i] = sngeAddSprite(starc, point(-500, -500), -1);

    starw = spritesClasses[starc].frame[0].w;
    starh = spritesClasses[starc].frame[0].h;

    menuEntries[ME_ARCADE] = sngeAddFontSprite(spritesGetIdByName("font:base"), point(0, 0), 2, "arcade");
    menuEntries[ME_SURVIVAL] = sngeAddFontSprite(spritesGetIdByName("font:base"), point(0, 70), 2, "survival");
    menuEntries[ME_HOF] = sngeAddFontSprite(spritesGetIdByName("font:base"), point(0, 140), 2, "hall of fame");
    menuEntries[ME_EXIT] = sngeAddFontSprite(spritesGetIdByName("font:base"), point(0, 210), 2, "exit");
}

int menuFrame(float lag)
{
    procStars(lag);
    procMenu(lag);

    if(inputKeyState[SDLK_SPACE])
    {
        inputKeyState[SDLK_SPACE] = 0;
        switch(menuEntry)
        {
            case ME_EXIT: return 1; break;
            default:  break;
        }
    }

    return 0;
}

void menuCleanup()
{
    sngeFreeSprites();
    spritesFreeAll();
}

