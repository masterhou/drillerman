#include "mainloop.h"

#include <SDL/SDL.h>

#include "input.h"
#include "message.h"
#include "snge.h"
#include "splash.h"
#include "menu.h"
#include "exitscr.h"
#include "settings.h"
#include "game.h"
#include "timer.h"
#include "particles.h"

typedef void (*ScreenInitFunc)(void *);
typedef void (*ScreenCleanupFunc)(void);
typedef int (*ScreenFrameFunc)(float);

#define _FPS_ITEM_COUNT 20
float fps[_FPS_ITEM_COUNT];
int fpsItem = 0;

typedef struct
{
    ScreenInitFunc init;
    ScreenCleanupFunc cleanup;
    ScreenFrameFunc frame;
} ScreenFuncs;

static enum {FS_NOFADE, FS_FADEIN, FS_FADEOUT} fadeStatus = FS_NOFADE;
static float fadeAmount;
static float fadeDuration;
static ScreenId fadeIntoScr;
static void *fadeIntoScrData;

static int lastDelay;

static ScreenId scr = SCR_NONE;
static ScreenFuncs scrFuncs[SCR_LAST] =	{
    {init: &exit_Init, cleanup: &exit_Cleanup, frame: &exit_Frame},
    {init: &splash_Init, cleanup: &splash_Cleanup, frame: &splash_Frame},
    {init: &menu_Init, cleanup: &menu_Cleanup, frame: &menu_Frame},
    {init: &game_Init, cleanup: &game_Cleanup, frame: &game_Frame},
    {init: &settings_Init, cleanup: &settings_Cleanup, frame: &settings_Frame}
};

static void procFade(float lag)
{
    if(fadeStatus == FS_FADEOUT)
    {
        fadeAmount += lag / fadeDuration;

        if(fadeAmount > 1.0)
        {
            mainloop_ChangeScr(fadeIntoScr, fadeIntoScrData);
            fadeStatus = FS_FADEIN;
            fadeAmount = 1.0;
        }
    }

    if(fadeStatus == FS_FADEIN)
    {
        fadeAmount -= lag / fadeDuration;

        if(fadeAmount < 0.0)
        {
            fadeAmount = 0.0;
            fadeStatus = FS_NOFADE;
            graphics_SetFadeColor(color(0, 0, 0, fadeAmount));
        }
    }


    if(fadeStatus)
        graphics_SetFadeColor(color(0, 0, 0, fadeAmount));

}

static inline float getFPS(float lag)
{
    fps[fpsItem] = 1.0 / lag;
    fpsItem++;
    fpsItem %= _FPS_ITEM_COUNT;

    int i;
    float cfps = 0.0f;

    for(i = 0; i < _FPS_ITEM_COUNT; ++i)
        cfps += fps[i] / (float)_FPS_ITEM_COUNT;

    if(fpsItem == 0)
        message_OutEx("FPS: %f\n", cfps);

    return cfps;
}

static inline void fpsLimiterDelay(float lag)
{
    float diff = (1.0 / _FPS_LIMIT) - lag;

    int delay = (int)(diff * 1000.0) + lastDelay;
    lastDelay = delay;

    if(delay <= 0.0)
        return;

    SDL_Delay(delay);
}

void mainloop_ChangeScr(ScreenId newscr, void *data)
{
    if(scr != SCR_NONE)
        scrFuncs[scr].cleanup();
    scr = newscr;
    scrFuncs[scr].init(data);
}

void mainloop_ChangeScrWithFade(ScreenId newscr, void *data, float duration)
{
    fadeIntoScr = newscr;
    fadeIntoScrData = data;
    fadeDuration = duration;

    fadeStatus = FS_FADEOUT;
    fadeAmount = 0.0;
}


void mainloop_Go()
{

    bool done = 0;
    double lag = 0;
    Uint32 oldticks = 0;

    lastDelay = 0;

    mainloop_ChangeScr(SCR_MENU, NULL);

    fadeDuration = 1.5;
    fadeAmount = 1.0;
    fadeStatus = FS_FADEIN;

    while(!done)
    {
        lag = (double)(SDL_GetTicks() - oldticks) / 1000.0;
        oldticks = SDL_GetTicks();

        graphics_ClearBuffer();

        input_PumpEvents();
        snge_UpdateAnim(lag);

        timer_ProcessTimers(lag);

        if(scrFuncs[scr].frame(lag))
            done = 1;

        particles_Frame(lag);
        procFade(lag);

        snge_CleanupSprites();
        snge_Draw();

        if(input_IsKeyPressed(KEY_EXIT) || input_WantQuit())
            done = true;

        graphics_BlitBuffer();

        timer_CleanTimers();
        fpsLimiterDelay(lag);
    }

}
