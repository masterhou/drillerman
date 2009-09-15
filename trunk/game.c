#include "game.h"

#include <stdio.h>

#include "defaults.h"
#include "message.h"
#include "mainloop.h"
#include "level.h"
#include "player.h"
#include "sprites.h"
#include "snge.h"
#include "particles.h"

static int mapHeight = 150;

static void loadLevelSpriteClasses(int levelNum);

static void loadLevelSpriteClasses(int levelNum)
{
    char prefix[_STR_BUFLEN];
    snprintf(prefix, _STR_BUFLEN, "level_%d:", levelNum);

    sprites_LoadFromCfgF("sprites/levelset_%d/red/red.spr", prefix, levelNum + 1);
    sprites_LoadFromCfgF("sprites/levelset_%d/blue/blue.spr", prefix, levelNum + 1);
    sprites_LoadFromCfgF("sprites/levelset_%d/green/green.spr", prefix, levelNum + 1);
    sprites_LoadFromCfgF("sprites/levelset_%d/yellow/yellow.spr", prefix, levelNum + 1);
    sprites_LoadFromCfgF("sprites/levelset_%d/bcg/bcg.spr", prefix, levelNum + 1);
}

void game_Init(void *data)
{
    sprites_LoadFromCfg("sprites/level_common/level_common.spr", "level_common:");

    int i;

    for(i = 0; i < _LEVEL_COUNT; ++i)
        loadLevelSpriteClasses(i);

    player_Init(mapHeight);
}

int game_Frame(float lag)
{
    player_Frame(lag);
    level_Frame(lag);
    return 0;
}

void game_Cleanup()
{
    particles_Cleanup();
    snge_FreeSprites();
    sprites_FreeAll();
}


