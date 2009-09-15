#ifndef LEVEL_H
#define LEVEL_H

#include "particles.h"
#include "generator.h"
#include "snge.h"

typedef enum {FS_NORM, FS_SHAKE, FS_FALL, FS_VANISH, FS_BLINK} FieldState;

void level_Frame(float lag);
void level_Init(int levelHeight);
int level_IsSolid(int x, int y);
void level_Cleanup();
void level_DestroyBrick(int x, int y, bool blink);
FieldType level_GetFieldType(int x, int y);
bool level_IsAirGetAir(int x, int y, Sprite **psprite);
bool level_HitCrate(int x, int y);

#endif
