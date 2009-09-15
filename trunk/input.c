#include "input.h"

#include <SDL/SDL.h>

static bool keyState[SDLK_LAST + 1];
static bool quit;

static int keyMap[KEY_COUNT] = {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN, SDLK_SPACE, SDLK_ESCAPE};
static int directionKeyMap[DIR_COUNT] = {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN};

void input_Init()
{

    int i;

    for(i = 0; i <= SDLK_LAST; ++i)
        keyState[i] = false;

    quit = false;
}

bool input_IsDirPressed(Direction dir)
{
    return keyState[keyMap[directionKeyMap[dir]]];
}

bool input_IsKeyPressed(Key key)
{
    return keyState[keyMap[key]];
}

void input_UnsetPressed(Key key)
{
    keyState[keyMap[key]] = false;
}

bool input_WantQuit()
{
    return quit;
}

void input_PumpEvents()
{

    SDL_PumpEvents();
    SDL_Event event;

    quit = false;

    while(SDL_PollEvent(&event))
    {
        if((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP))
        {
            bool val;

            if(event.type == SDL_KEYDOWN) val = true;
            else val = false;

            keyState[event.key.keysym.sym] = val;
        }

        if(event.type == SDL_QUIT)
            quit = true;
    }

}



