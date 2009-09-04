#include "sprites.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "graphics.h"
#include "message.h"
#include "config.h"
#include "common.h"


int spritesCCount;
SpriteClass *spritesClasses;

static void incrementClassCount()
{
    spritesCCount++;
    spritesClasses = realloc(spritesClasses, sizeof(SpriteClass) * spritesCCount);
}

void spritesGetDimensions(SpriteClassId id, int *width, int *height)
{
    *width = spritesClasses[id].frame[0].w;
    *height = spritesClasses[id].frame[0].h;
}

void spritesInit()
{
    spritesClasses = NULL;
    spritesCCount = 0;
}

void spritesLoadFromCfg(const char *cfgpathrel)
{

    extern char dataPath[];

    char path[256] = {'\0'};

    strcat(path, dataPath);
    strcat(path, cfgpathrel);

    cfgOpen(path);

    unsigned int totalsz = 0;
    int totalnum = 0;

    char name[256];
    char sprpath[256];

    commonGetBasePath(path, sprpath);

    while(cfgNextSection(name))
    {
        messageOutEmfEx("Loading sprite class '%s'...\n", name);

        incrementClassCount();

        int s = spritesCCount - 1;

        SpriteClass *sc = &spritesClasses[s];

        sc->name = malloc(strlen(name) + 1);
        sprintf(sc->name, "%s", name);

        sc->areverse = cfgGetTag("reverse");
        sc->arepeat = cfgGetTag("repeat");

        cfgGetIntValue("frames", &sc->fcount);

        sc->ssc = SSC_STILL;

        if(sc->fcount > 1)
        {
            cfgGetDoubleValue("speed", &sc->fps);
            messageOutEx("\tHas %d frames.\n", sc->fcount);
            messageOutEx("\tRuns at %2.2lf fps.\n", sc->fps);
            sc->ssc = SSC_ANIM;
        }


        sc->frame = malloc(sizeof(SpriteFrame) * sc->fcount);


        int i;

        for(i = 0; i < sc->fcount; ++i)
        {
            char texfile[256];
            char framestr[256];
            char texpath[256];

            SpriteFrame *frame = &sc->frame[i];

            sprintf(framestr, "frame(%d)", i + 1);

            cfgGetStringValue(framestr, texfile);

            sprintf(texpath, "%s%s", sprpath, texfile);

            frame->image = graphicsLoadBitmap(texpath, &frame->w, &frame->h);

            messageOutEx("\tLoading frame %2d : '%s' (%dx%d) ...\n", i + 1, texpath, frame->w, frame->h);

            if(!frame->image)
                messageCriticalErrorEx("Could not load frame %d - '%s'.\n", i, texpath);



            totalsz += frame->w * frame->h * 4;
            totalnum++;

        }

    }

    messageOutEx("Loaded %d bitmaps of approximate size of %.3lf MiB\n",
                 totalnum, (double)totalsz / (1024.0 * 1024.0));

    cfgClose();
}

void spritesLoadFromCfgF(const char *cfgpathrelfmt, ...)
{
    va_list args;
    char buf[1024];

    va_start(args, cfgpathrelfmt);
    vsnprintf(buf, sizeof(buf), cfgpathrelfmt, args);
    va_end(args);

    spritesLoadFromCfg(buf);
}

void spritesLoadFontsFromCfg(char *cfgpathrel)
{
    extern char dataPath[];

    char path[256] = {'\0'};

    strcat(path, dataPath);
    strcat(path, cfgpathrel);

    cfgOpen(path);

    char name[256];
    char imgpath[256];
    unsigned int totalsz = 0;
    int totalnum = 0;

    commonGetBasePath(path, imgpath);

    while(cfgNextSection(name))
    {
        messageOutEx("Loading sprite:font class '%s'...\n", name);

        incrementClassCount();

        int s = spritesCCount - 1;

        SpriteClass *sc = &spritesClasses[s];

        sc->name = malloc(strlen(name) + 1);

        sprintf(sc->name, "%s", name);

        sc->fcount = 1;
        sc->frame = malloc(sizeof(SpriteFrame));
        sc->ssc = SSC_BFNT;

        char texfile[256];
        char texpath[256];

        cfgGetIntValue("char_width", &sc->font.char_width);
        cfgGetIntValue("char_height", &sc->font.char_height);
        cfgGetStringValue("char_string", sc->font.char_string);

        messageOutEx("\tCharacter string: '%s'\n", sc->font.char_string);
        messageOutEx("\tCharacter size: %dx%d\n", sc->font.char_width, sc->font.char_height);

        cfgGetStringValue("image", texfile);

        sprintf(texpath, "%s%s", imgpath, texfile);

        sc->frame->image = graphicsLoadBitmap(texpath, &sc->frame->w, &sc->frame->h);

        messageOutEx("\tLoading bmp font image: '%s' (%dx%d) ...\n", texpath, sc->frame->w, sc->frame->h);

        if(!sc->frame->image)
            messageCriticalErrorEx("Could not load font image '%s'.\n", texpath);

        totalsz += sc->frame->w * sc->frame->h * 4;
        totalnum++;

    }

    messageOutEx("Loaded %d bitmaps of approximate size of %.3lf MiB\n",
                 totalnum, (double)totalsz / (1024.0 * 1024.0));

    cfgClose();
}

SpriteClassId spritesGetIdByName(const char *name)
{
    int i;

    for(i = 0; i < spritesCCount; ++i)
        if(!strcmp(spritesClasses[i].name, name))
            return i;

    messageCriticalErrorEx("Sprite class '%s' not found!\n", name);
    return -1;

}

SpriteClassId spritesGetIdByNameF(const char *namefmt, ...)
{
    va_list args;
    char buf[1024];

    va_start(args, namefmt);
    vsnprintf(buf, sizeof(buf), namefmt, args);
    va_end(args);

    return spritesGetIdByName(buf);
}

void spritesFreeAll()
{
    int i, j;

    for(i = 0; i < spritesCCount; ++i)
    {
        for(j = 0; j < spritesClasses[i].fcount; ++j)
            glDeleteTextures(1, &spritesClasses[i].frame[j].image);

        free(spritesClasses[i].frame);
        free(spritesClasses[i].name);
    }

    if(spritesClasses)
        free(spritesClasses);

    spritesCCount = 0;
    spritesClasses = NULL;

}


