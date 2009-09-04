#include "input.h"

#include <SDL/SDL.h>

unsigned char inputKeyState[SDLK_LAST + 1];
unsigned char inputQuit;

void inputInit()
{

    int i;

    for(i = 0; i <= SDLK_LAST; ++i)
        inputKeyState[i] = 0;

    inputQuit = 0;

}


void inputPumpEvents()
{

    SDL_PumpEvents();
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {

        if((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP))
        {
            unsigned char val;

            if(event.type == SDL_KEYDOWN) val = 1;
            else val = 0;

            inputKeyState[event.key.keysym.sym] = val;
        }

        if(event.type == SDL_QUIT)
            inputQuit = 1;

    }

}



