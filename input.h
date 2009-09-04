#ifndef INPUT_H
#define INPUT_H

#include <SDL/SDL_keysym.h>

extern unsigned char inputKeyState[SDLK_LAST + 1];
extern unsigned char inputQuit;

void inputPumpEvents();
void inputInit();

#endif
