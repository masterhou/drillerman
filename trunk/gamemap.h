#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "particles.h"
#include "generator.h"
#include "snge.h"

typedef enum {FS_NORM, FS_SHAKE, FS_FALL, FS_VANISH, FS_BLINK} FieldState;

typedef struct
{
    Sprite *sprite;
    Particle *particle;
    FieldType type;
    FieldState state;
    float timer;
    Point shift;
    bool supported;
    bool checked;
    bool justhit;
    int shakedir;
} MapField;

void gameMapFrame(float lag);
void gameMapUpdateBrickShape();
void gameMapAllocSprites();
void gameMapInit(int height, Difficulty difficulty);
int gameMapIsSolid(int x, int y);
void gameMapCleanup();
void gameMapDestroyBrick(int x, int y, bool blink);
FieldType gameMapGetFieldType(int x, int y);
bool gameMapIsAirGetAir(int x, int y, Sprite **psprite);

#endif
