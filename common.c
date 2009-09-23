#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

void common_GetBasePath(const char *input, char *output)
{

#ifndef WIN32
    char *e = strrchr(input, '/');
#else
    char *e = strrchr(input, '\\');
#endif
    int l = e - input + 1;

    memmove(output, input, (size_t)l);
    output[l] = 0;

}

double common_RandD()
{
    return (double)random() / (double)(RAND_MAX);
}

int common_RandI()
{
    return random();
}

void **common_Alloc2DTable(int w, int h, unsigned int elsz)
{
    int i;
    void **t;

    t = malloc(sizeof(void*) * w);

    for(i = 0; i < w; ++i)
        t[i] = malloc(elsz * h);

    return t;
}

void common_Free2DTable(void **table, int w)
{
    int i;

    for(i = 0; i < w; ++i)
        free(table[i]);

    free(table);
}

inline Point point(float x, float y)
{
    Point p = {x: x, y: y};
    return p;
}

inline Color color(float r, float g, float b, float a)
{
    Color c = {r: r, g: g, b: b, a: a};
    return c;
}

void common_Init()
{
    srandom(time(NULL));
}

