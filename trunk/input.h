#ifndef INPUT_H
#define INPUT_H

#include <SDL/SDL_keysym.h>

#include "common.h"

typedef enum {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_DRILL, KEY_EXIT, KEY_COUNT} Key;

void input_PumpEvents();
void input_Init();
bool input_IsDirPressed(Direction dir);
bool input_IsKeyPressed(Key key);
void input_UnsetPressed(Key key);
bool input_WantQuit();


#endif
