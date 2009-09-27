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

#define FSGN(X) \
        ((X > 0.0) ? 1.0 : ((X == 0.0) ? 0.0 : -1.0))


typedef unsigned char bool;

#define true 1
#define false 0

typedef unsigned int BitmapId ;

typedef struct
{
    float x;
    float y;
} Point;

typedef struct
{
    int x;
    int y;
} IntPoint;

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
    IntPoint size;
    Point scale;
    float angle;
    float opacity;
    char vflip;
    char hflip;
} Transformations;

typedef enum {DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_COUNT, DIR_NONE} Direction;

Point point(float x, float y);
Color color(float r, float g, float b, float a);

void common_GetBasePath(const char *input, char *output);
double common_RandD();
int common_RandI();

void common_Free2DTable(void **table, int w);
void **common_Alloc2DTable(int w, int h, unsigned int elsz);

void common_Init();

#endif

