#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

void commonGetBasePath(const char *input, char *output)
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

double commonRandD()
{
    return (double)rand() / (double)(RAND_MAX);
}

int commonRandI()
{
    return (double)rand() / (double)(RAND_MAX) * 100.0;
}

void **commonAlloc2DTable(int w, int h, unsigned int elsz)
{
    int i;
    void **t;

    t = malloc(sizeof(void*) * w);

    for(i = 0; i < w; ++i)
        t[i] = malloc(elsz * h);

    return t;
}

void commonFree2DTable(void **table, int w)
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

