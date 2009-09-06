#ifndef GAMEMAP_H
#define GAMEMAP_H

#include "generator.h"
#include "snge.h"

typedef enum {FS_NORM, FS_SHAKE, FS_FALL, FS_VANISH} FieldState;

typedef struct
{
    Sprite *sprite;
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
void gameMapDestroyBrick(int x, int y);
FieldType gameMapGetFieldType(int x, int y);
bool gameMapIsAirGetAir(int x, int y, Sprite **psprite);

#endif
