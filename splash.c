#include "splash.h"

#include <SDL/SDL.h>

#include "snge.h"
#include "mainloop.h"

static Sprite *logo;

static float dx, dy, sx, sy;
static int go;
static int finished;

void splash_Init(void *data)
{

    int w, h;
    go = 1;

    SpriteClassId logoc = sprites_GetIdByName("logobig");
    sprites_GetDimensions(logoc, &w, &h);
    sx = -w;
    sy = -h;

    snge_AddSprite(sprites_GetIdByName("splash"), point(0, 0), 0);
    logo = snge_AddSprite(logoc, point(sx, sy), 1);

    dx = (_SCREEN_WIDTH - w) / 2;
    dy = (_SCREEN_HEIGHT- h) / 2;

    logo->sx = 0.1;
    logo->sy = 0.1;
    finished = 0;
}	


int splash_Frame(float lag)
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
            mainloop_ChangeScrWithFade(SCR_MENU, NULL, 0.5);
            finished = 1;
        }
    }

    return 0;
}

void splash_Cleanup()
{
    snge_FreeSprites();
}
