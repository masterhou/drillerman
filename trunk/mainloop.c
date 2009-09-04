#include "mainloop.h"

#include <SDL/SDL.h>

#include "input.h"
#include "message.h"
#include "snge.h"
#include "splash.h"
#include "menu.h"
#include "exitscr.h"
#include "game.h"
#include "timer.h"
#include "particles.h"

typedef void (*ScreenInitFunc)(void *);
typedef void (*ScreenCleanupFunc)(void);
typedef int (*ScreenFrameFunc)(float);

#define _FPS_ITEM_COUNT 10
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



static ScreenId scr = SCR_NONE;
static ScreenFuncs scrFuncs[SCR_LAST] =	{
    {init: &exitInit, cleanup: &exitCleanup, frame: &exitFrame},
    {init: &splashInit, cleanup: &splashCleanup, frame: &splashFrame},
    {init: &menuInit, cleanup: &menuCleanup, frame: &menuFrame},
    {init: &gameInit, cleanup: &gameCleanup, frame: &gameFrame}
};

static float getFps(float lag)
{
    fps[fpsItem] = 1.0 / lag;
    fpsItem++;
    fpsItem %= _FPS_ITEM_COUNT;

    int i;
    float cfps = 0.0f;

    for(i = 0; i < _FPS_ITEM_COUNT; ++i)
        cfps += fps[i] / (float)_FPS_ITEM_COUNT;

    if(fpsItem == 0)
        messageOutEx("FPS: %f\n", cfps);

    return cfps;
}

void mainloopChangeScr(ScreenId newscr, void *data)
{
    if(scr != SCR_NONE)
        scrFuncs[scr].cleanup();
    scr = newscr;
    scrFuncs[scr].init(data);
}

void mainloopChangeScrWithFade(ScreenId newscr, void *data, float duration)
{
    fadeIntoScr = newscr;
    fadeIntoScrData = data;
    fadeDuration = duration;

    fadeStatus = FS_FADEOUT;
    fadeAmount = 0.0;
}

static void procFade(float lag)
{
    if(fadeStatus == FS_FADEOUT)
    {
        fadeAmount += lag / fadeDuration;

        if(fadeAmount > 1.0)
        {
            mainloopChangeScr(fadeIntoScr, fadeIntoScrData);
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
            graphicsSetFadeColor(color(0, 0, 0, fadeAmount));
        }
    }


    if(fadeStatus)
        graphicsSetFadeColor(color(0, 255, 0, fadeAmount));

}

void mainloopGo()
{

    int done = 0;
    double lag = 0;
    Uint32 oldticks = 0;

    mainloopChangeScr(SCR_GAME, NULL);

    fadeDuration = 1.5;
    fadeAmount = 1.0;
    fadeStatus = FS_FADEIN;

    while(!done)
    {
        lag = (double)(SDL_GetTicks() - oldticks) / 1000.0;
        oldticks = SDL_GetTicks();

        getFps(lag);

        graphicsClearBuffer();

        inputPumpEvents();
        sngeUpdateAnim(lag);

        timerProcessTimers(lag);

        if(scrFuncs[scr].frame(lag))
            done = 1;

        particlesFrame(lag);
        procFade(lag);

        sngeDraw();

        if(inputKeyState[SDLK_ESCAPE] || inputQuit) done = 1;

        graphicsBlitBuffer();

        timerCleanTimers();
        SDL_Delay(10);
    }

}
