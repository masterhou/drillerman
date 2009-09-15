#include "main.h"

#include <stdlib.h>
#include <stdio.h>

#include <SDL/SDL.h>

#include "defaults.h"
#include "mainloop.h"
#include "message.h"
#include "input.h"
#include "snge.h"
#include "config.h"
#include "particles.h"

char dataPath[256] = _DATA_PATH;
int screenWidth = _SCREEN_WIDTH;
int screenHeight = _SCREEN_HEIGHT;

int enableFullscreen = 0;
int filteringType = 0;
float xScrRatio;
float yScrRatio;

static void readConfiguration()
{
    char path[256];
    sprintf(path, "%sdefault.cfg", dataPath);

    char resolution[256];

    cfg_Open(path);

    cfg_SeekSection("graphics");
    cfg_GetIntValue("fullscreen", &enableFullscreen);
    cfg_GetIntValue("filtering", &filteringType);
    cfg_GetStringValue("resolution", resolution);


    sscanf(resolution, "%dx%d", &screenWidth, &screenHeight);

    cfg_Close();
}

int main(int argc, char **argv)
{
    message_OutEx("Data path is set to '%s'\n", _DATA_PATH);

    xScrRatio = (float)screenWidth / (float)_SCREEN_WIDTH;
    yScrRatio = (float)screenHeight / (float)_SCREEN_HEIGHT;

    readConfiguration();

    SDL_Surface* screen;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        message_CriticalError("Could not initialize SDL video.\n");

    atexit(SDL_Quit);

    if(!(screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_OPENGL | (enableFullscreen ? SDL_FULLSCREEN : 0))))
        message_CriticalError("Could not set desired video mode.\n");

    SDL_WM_SetCaption("D-Man", NULL);

    graphicsInitSubsytem(_SCREEN_WIDTH, _SCREEN_HEIGHT);
    input_Init();
    sprites_Init();
    snge_Init();
    particles_Init();

    mainloop_Go();

    particles_Cleanup();
    sprites_FreeAll();
    snge_FreeSprites();

    message_Out("Goodbye.\n");

    return EXIT_SUCCESS;
}
