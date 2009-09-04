#ifndef GAMEMECH_H
#define GAMEMECH_H

#include "generator.h"

void gameMechInit(int mapheight, Difficulty difficulty);
void gameMechFrame(float lag);
void gameMechCleanup();

#endif
