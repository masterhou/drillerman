#include "sprites.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "graphics.h"
#include "message.h"
#include "config.h"
#include "common.h"


static int count;
static SpriteClass *classes;

static void incrementClassCount()
{
    count++;
    classes = realloc(classes, sizeof(SpriteClass) * count);
}

void sprites_GetDimensions(SpriteClassId id, int *width, int *height)
{
    *width = classes[id].frame[0].w;
    *height = classes[id].frame[0].h;
}

void sprites_Init()
{
    classes = NULL;
    count = 0;
}

void sprites_LoadFromCfg(const char *cfgpathrel, const char *namePrefix)
{

    extern char dataPath[];

    char path[_STR_BUFLEN] = {'\0'};

    strcat(path, dataPath);
    strcat(path, cfgpathrel);

    cfg_Open(path);

    unsigned int totalsz = 0;
    int totalnum = 0;

    char name[_STR_BUFLEN];
    char sprpath[_STR_BUFLEN];

    common_GetBasePath(path, sprpath);

    while(cfg_NextSection(name))
    {
        incrementClassCount();
        int s = count - 1;

        message_OutEmfEx("Loading sprite class '%s' id '%d'...\n", name, s);

        SpriteClass *sc = &classes[s];

        sc->name = malloc(strlen(name) + strlen(namePrefix) + 1);
        sprintf(sc->name, "%s%s", namePrefix, name);

        message_OutEmfEx("Internal sprite class name: %s\n", sc->name);

        sc->areverse = cfg_GetBool("reverse");
        sc->arepeat = cfg_GetBool("repeat");

        cfg_GetIntValue("frames", &sc->fcount);

        sc->ssc = SSC_STILL;

        if(sc->fcount > 1)
        {
            cfg_GetDoubleValue("speed", &sc->fps);
            message_OutEx("\tHas %d frames.\n", sc->fcount);
            message_OutEx("\tRuns at %2.2lf fps.\n", sc->fps);
            sc->ssc = SSC_ANIM;
        }

        sc->frame = malloc(sizeof(SpriteFrame) * sc->fcount);

        int i;

        for(i = 0; i < sc->fcount; ++i)
        {
            char texfile[_STR_BUFLEN];
            char framestr[_STR_BUFLEN];
            char texpath[_STR_BUFLEN];

            SpriteFrame *frame = &sc->frame[i];

            sprintf(framestr, "frame(%d)", i + 1);

            cfg_GetStringValue(framestr, texfile);

            sprintf(texpath, "%s%s", sprpath, texfile);

            frame->image = graphicsLoadBitmap(texpath, &frame->w, &frame->h);

            message_OutEx("\tLoading frame %2d : '%s' (%dx%d) ...\n", i + 1, texpath, frame->w, frame->h);

            if(!frame->image)
                message_CriticalErrorEx("Could not load frame %d - '%s'.\n", i, texpath);



            totalsz += frame->w * frame->h * 4;
            totalnum++;

        }

    }

    message_OutEx("Loaded %d bitmaps of approximate size of %.3lf MiB\n",
                 totalnum, (double)totalsz / (1024.0 * 1024.0));

    cfg_Close();
}

inline SpriteClass* sprites_GetClass(SpriteClassId id)
{
    return &classes[id];
}

void sprites_LoadFromCfgF(const char *cfgpathrelfmt, const char *namePrefix, ...)
{
    va_list args;
    char buf[_STR_BUFLEN];

    va_start(args, namePrefix);
    vsnprintf(buf, sizeof(buf), cfgpathrelfmt, args);
    va_end(args);

    sprites_LoadFromCfg(buf, namePrefix);
}

void sprites_LoadFontsFromCfg(char *cfgpathrel)
{
    extern char dataPath[];

    char path[_STR_BUFLEN] = {'\0'};

    strcat(path, dataPath);
    strcat(path, cfgpathrel);

    cfg_Open(path);

    char name[_STR_BUFLEN];
    char imgpath[_STR_BUFLEN];
    unsigned int totalsz = 0;
    int totalnum = 0;

    common_GetBasePath(path, imgpath);

    while(cfg_NextSection(name))
    {
        message_OutEx("Loading sprite:font class '%s'...\n", name);

        incrementClassCount();

        SpriteClass *sc = &classes[count - 1];

        sc->name = malloc(strlen(name) + 1);

        sprintf(sc->name, "%s", name);

        sc->fcount = 1;
        sc->frame = malloc(sizeof(SpriteFrame));
        sc->ssc = SSC_BFNT;

        char texfile[_STR_BUFLEN];
        char texpath[_STR_BUFLEN];

        cfg_GetIntValue("char_width", &sc->font.char_width);
        cfg_GetIntValue("char_height", &sc->font.char_height);
        cfg_GetStringValue("char_string", sc->font.char_string);

        message_OutEx("\tCharacter string: '%s'\n", sc->font.char_string);
        message_OutEx("\tCharacter size: %dx%d\n", sc->font.char_width, sc->font.char_height);

        cfg_GetStringValue("image", texfile);

        sprintf(texpath, "%s%s", imgpath, texfile);

        sc->frame->image = graphicsLoadBitmap(texpath, &sc->frame->w, &sc->frame->h);

        message_OutEx("\tLoading bmp font image: '%s' (%dx%d) ...\n", texpath, sc->frame->w, sc->frame->h);

        if(!sc->frame->image)
            message_CriticalErrorEx("Could not load font image '%s'.\n", texpath);

        totalsz += sc->frame->w * sc->frame->h * 4;
        totalnum++;

    }

    message_OutEx("Loaded %d bitmaps of approximate size of %.3lf MiB\n",
                 totalnum, (double)totalsz / (1024.0 * 1024.0));

    cfg_Close();
}

SpriteClassId sprites_GetIdByName(const char *name)
{
    int i;

    for(i = 0; i < count; ++i)
        if(!strcmp(classes[i].name, name))
        {
            return i;
        }

    message_CriticalErrorEx("Sprite class '%s' not found!\n", name);
    return -1;

}

SpriteClassId sprites_GetIdByNameF(const char *namefmt, ...)
{
    va_list args;
    char buf[_STR_BUFLEN];

    va_start(args, namefmt);
    vsnprintf(buf, sizeof(buf), namefmt, args);
    va_end(args);

    return sprites_GetIdByName(buf);
}

void sprites_FreeAll()
{
    int i, j;

    for(i = 0; i < count; ++i)
    {
        for(j = 0; j < classes[i].fcount; ++j)
            glDeleteTextures(1, &classes[i].frame[j].image);

        free(classes[i].frame);
        free(classes[i].name);
    }

    if(classes)
        free(classes);

    count = 0;
    classes = NULL;

}


