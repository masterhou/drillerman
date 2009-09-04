#include "game.h"

#include "defaults.h"
#include "message.h"
#include "mainloop.h"
#include "gamemap.h"
#include "gamemech.h"
#include "sprites.h"
#include "snge.h"
#include "particles.h"

static int mapHeight = 150;

static void loadSpriteClasses(char *levelsetName)
{
    spritesLoadFromCfgF("sprites/levelset_%s/single/single.spr", levelsetName);
    spritesLoadFromCfgF("sprites/levelset_%s/red/red.spr", levelsetName);
    spritesLoadFromCfgF("sprites/levelset_%s/blue/blue.spr", levelsetName);
    spritesLoadFromCfgF("sprites/levelset_%s/green/green.spr", levelsetName);
    spritesLoadFromCfgF("sprites/levelset_%s/yellow/yellow.spr", levelsetName);
}

void gameInit(void *data)
{
    loadSpriteClasses("default");
    gameMechInit(mapHeight, DF_EASY);
}

int gameFrame(float lag)
{
    gameMechFrame(lag);
    gameMapFrame(lag);
    return 0;
}

void gameCleanup()
{
    particlesCleanup();
    sngeFreeSprites();
    spritesFreeAll();
}


