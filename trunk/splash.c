#include "splash.h"

#include <SDL/SDL.h>

#include "snge.h"
#include "mainloop.h"

static Sprite *logo;

static float dx, dy, sx, sy;
static int go;
static int finished;

void splashInit(void *data)
{

    go = 1;

    SpriteClassId logoc = spritesGetIdByName("logobig");
    int w = spritesClasses[logoc].frame[0].w;
    int h = spritesClasses[logoc].frame[0].h;
    sx = -w;
    sy = -h;

    sngeAddSprite(spritesGetIdByName("splash"), point(0, 0), 0);
    logo = sngeAddSprite(logoc, point(sx, sy), 1);

    dx = (_SCREEN_WIDTH - w) / 2;
    dy = (_SCREEN_HEIGHT- h) / 2;

    logo->sx = 0.1;
    logo->sy = 0.1;
    finished = 0;
}	


int splashFrame(float lag)
{
    if(!finished)
    {
        if(go)
        {
            logo->sx += 0.35 * lag;
            logo->sy += 0.35 * lag;
            logo->angle += 700.0 * lag;
        }

        if(!go && logo->angle > (lag * 360.0)) logo->angle += 360.0 * lag;

        if(logo->x < dx) logo->x += (dx - sx) / 3.0 * lag;
        if(logo->y < dy) logo->y += (dy - sy) / 3.0 * lag;

        if(logo->x >= dx && logo->y >= dy) go = 0;

        if(!go && logo->angle < (lag * 360.0))
        {
            mainloopChangeScrWithFade(SCR_MENU, NULL, 0.5);
            finished = 1;
        }
    }

    return 0;
}

void splashCleanup()
{
    sngeFreeSprites();
}
