#ifndef COMMON_H
#define COMMON_H

#define INBOUND(_X, _Y, _maxX, _maxY) \
(((_X) >= 0) && ((_Y) >= 0) && \
 ((_X) < (_maxX)) && ((_Y) < (_maxY)))


#define NO_COLLISION(X1, Y1, W1, H1, X2, Y2, W2, H2) \
        (((X1) + (W1)) <= (X2) || \
        ((Y1) + (H1)) <= (Y2) || \
        (X1) >= ((X2) + (W2)) || \
        (Y1) >= ((Y2) + (H2)))

#define DEG_TO_RAD(D) ((D) / 57.2957795)
#define SQR(X) ((X) * (X))


#define SGN(X) \
        ((X > 0) ? 1 : ((X == 0) ? 0 : -1))

typedef unsigned char bool;

#define true 1
#define false 0

typedef struct
{
    int char_width;
    int char_height;
    char char_string[256];
} Font;

typedef unsigned int BitmapId ;

typedef struct
{
    float x;
    float y;
} Point;

typedef struct
{
    float r;
    float g;
    float b;
    float a;
} Color;

typedef struct
{
    Point trans;
    Point size;
    Point scale;
    int angle;
    float opacity;
    char vflip;
    char hflip;
} Transformations;

typedef enum
{
    DF_EASY,
    DF_MEDIUM,
    DF_HARD,
    DF_HARDCORE,
    DF_LAST
} Difficulty;

Point point(float x, float y);
Color color(float r, float g, float b, float a);

void commonGetBasePath(const char *input, char *output);
double commonRandD();
int commonRandI();

void commonFree2DTable(void **table, int w);
void **commonAlloc2DTable(int w, int h, unsigned int elsz);

#endif

