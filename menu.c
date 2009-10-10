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
    ME_500M,
    ME_1000M,
    ME_HOF,
    ME_SETTINGS,
    ME_EXIT,
    ME_CREDITS,
    ME_LAST
} menuEntry;

#define _ICON_WIDTH 64.0
#define _ICON_HEIGHT 64.0
#define _MENU_RADIUS 150.0
#define _MENU_X (400.0 - _ICON_WIDTH / 2.0)
#define _MENU_Y (260.0 - _ICON_HEIGHT / 2.0)
#define _ARROW_MAX_SHIFT 20.0
#define _ARROW_Y (_MENU_Y - _MENU_RADIUS - _ARROW_MAX_SHIFT - _ICON_HEIGHT)
#define _ARROW_SPEED 60.0
#define _ARROW_SFACTOR 1.5
#define _ARROW_RISE_SPEED 300.0
#define _ROT_SPEED 100.0


#define _LABEL_FONT "font:base"

#define _LABEL_FADE_SPEED 2.0

static Sprite *menuEntries[ME_LAST];
static Sprite *menuLabels[ME_LAST];
static Sprite *stars[STAR_COUNT];
static Sprite *arrow;

static int entry = ME_1000M;
static int nextEntry = ME_1000M;
static float angle;
static float angleShift;
static float arrowShift;
static float arrowSpeed;
static float angleSpacing;
static int rotDir;
static bool arrowFalling;

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
            starp[i].spy = 50.0;
            starp[i].spr = (float)(rand() % 720) - 360.0;

        }
    }
}

static void procMenu(float lag)
{
    arrowShift += lag * arrowSpeed;

    if(arrowShift < 0.0 && !arrowFalling)
    {
        arrowShift = -arrowShift;
        arrowSpeed = _ARROW_SFACTOR * _ARROW_SPEED;
    }

    if(arrowShift >= _ARROW_MAX_SHIFT)
    {
        arrowShift = _ARROW_MAX_SHIFT;
        arrowSpeed = -_ARROW_SPEED;
        arrowFalling = false;
    }

    if(arrowShift < -100.0 && arrowFalling)
    {
        arrowSpeed = _ARROW_RISE_SPEED;
        arrowShift = -100.0;
    }


    arrow->y = (float)_ARROW_Y + arrowShift;

    if(input_IsDirPressed(DIR_LEFT) && rotDir == 0)
    {
        rotDir = -1;
        nextEntry = entry + 1;
        if(nextEntry == ME_LAST)
            nextEntry = 0;

        arrowSpeed = -_ARROW_RISE_SPEED;
        arrowFalling = true;
    }

    if(input_IsDirPressed(DIR_RIGHT) && rotDir == 0)
    {
        rotDir = 1;
        nextEntry = entry - 1;
        if(nextEntry == -1)
            nextEntry = ME_LAST - 1;

        arrowSpeed = -_ARROW_RISE_SPEED;
        arrowFalling = true;
    }

    angleShift += lag * _ROT_SPEED * (float)rotDir;

    if((rotDir == 1 && angleShift >= angleSpacing) || (rotDir == -1 && angleShift <= -angleSpacing))
    {
        menuLabels[entry]->opacity = 0.0;
        menuLabels[nextEntry]->opacity = 1.0;
        angleShift = 0.0;
        rotDir = 0;
        entry = nextEntry;
    }

    if(rotDir != 0)
    {
        menuLabels[nextEntry]->opacity += _LABEL_FADE_SPEED * lag;
        menuLabels[entry]->opacity -= _LABEL_FADE_SPEED * lag;

        if(menuLabels[entry]->opacity < 0.0)
            menuLabels[entry]->opacity = 0.0;

        if(menuLabels[nextEntry]->opacity > 1.0)
            menuLabels[nextEntry]->opacity = 1.0;
    }

    angle = -(float)entry * angleSpacing + angleShift - 90.0;

    int i;

    for(i = 0; i < ME_LAST; ++i)
    {
        float fangle = angle + (float)i * angleSpacing;
        menuEntries[i]->x = cosf(DEG_TO_RAD(fangle)) * _MENU_RADIUS + _MENU_X;
        menuEntries[i]->y = sinf(DEG_TO_RAD(fangle)) * _MENU_RADIUS + _MENU_Y;
    }


}

static void center(Sprite *s)
{
    IntPoint sz = snge_GetTextSize(s);
    s->x = (float)(_SCREEN_WIDTH - sz.x) / 2.0;
    s->y = (float)(_SCREEN_HEIGHT - sz.y) / 2.0;
}

void menu_Init(void *data)
{

    sprites_LoadFromCfg("sprites/menu.spr", "");
    sprites_LoadFontsFromCfg("fonts/fonts.desc");

    menuEntry = ME_EXIT;

    srand(time(NULL));

    //graphics_SetBackground(FT_FLAT, color(0.3, 0.59, 1.0, 1), color(0, 0, 0, 0), GT_NONE);

//    snge_AddSprite(sprites_GetIdByName("menu_logo"), point(100, 25), 5);

    SpriteClassId starc = sprites_GetIdByName("star");

    int i;

    //for(i = 0; i < STAR_COUNT; ++i)
      //  stars[i] = snge_AddSprite(starc, point(-500, -500), -1);

    sprites_GetDimensions(starc, &starw, &starh);

    menuEntries[ME_500M] = snge_AddSprite(sprites_GetIdByName("start1"), point(0,0), 10);
    menuEntries[ME_1000M] = snge_AddSprite(sprites_GetIdByName("start2"), point(0,0), 10);
    menuEntries[ME_SETTINGS] = snge_AddSprite(sprites_GetIdByName("settings"), point(0,0), 10);
    menuEntries[ME_HOF] = snge_AddSprite(sprites_GetIdByName("highscores"), point(0,0), 10);
    menuEntries[ME_EXIT] = snge_AddSprite(sprites_GetIdByName("exit"), point(0,0), 10);
    menuEntries[ME_CREDITS] = snge_AddSprite(sprites_GetIdByName("credits"), point(0,0), 10);

    menuLabels[ME_500M] = snge_AddFontSprite(sprites_GetIdByName(_LABEL_FONT), point(0,0), 9, "500M");
    menuLabels[ME_1000M] = snge_AddFontSprite(sprites_GetIdByName(_LABEL_FONT), point(0,0), 9, "1000M");
    menuLabels[ME_SETTINGS] = snge_AddFontSprite(sprites_GetIdByName(_LABEL_FONT), point(0,0), 9, "SETTINGS");
    menuLabels[ME_HOF] = snge_AddFontSprite(sprites_GetIdByName(_LABEL_FONT), point(0,0), 9, "HALL OF FAME");
    menuLabels[ME_EXIT] = snge_AddFontSprite(sprites_GetIdByName(_LABEL_FONT), point(0,0), 9, "EXIT");
    menuLabels[ME_CREDITS] = snge_AddFontSprite(sprites_GetIdByName(_LABEL_FONT), point(0,0), 9, "CREDITS!");

    for(i = 0; i < ME_LAST; ++i)
    {
        center(menuLabels[i]);
        menuLabels[i]->opacity = 0.0;
    }

    arrow = snge_AddSprite(sprites_GetIdByName("arrow"), point(_MENU_X, _ARROW_Y), 10);
    angleSpacing = 360.0 / (float)ME_LAST;
    angle = 0.0;
    angleShift = 0.0;
    arrowShift = 0.0;
    arrowSpeed = _ARROW_SFACTOR * _ARROW_SPEED;
    rotDir = 0;
    arrowFalling = false;

    menuLabels[entry]->opacity = 1.0;
}

int menu_Frame(float lag)
{
    //procStars(lag);
    procMenu(lag);

    if(input_IsKeyPressed(KEY_DRILL))
    {
        switch(entry)
        {
            case ME_1000M:
                mainloop_ChangeScrWithFade(SCR_GAME, NULL, 1.0);
            break;
        }
    }


    return 0;
}

void menu_Cleanup()
{
    snge_FreeSprites();
    sprites_FreeAll();
}

