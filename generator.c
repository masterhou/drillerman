#include "generator.h"


#include <stdlib.h>
#include <time.h>

typedef struct 
{
    int airPeriodMin;
    int airPeriodMax;
    int airRowMax;
    int cratePeriodMin;
    int cratePeriodMax;
    int crateRowMax;
    int cratePassCount;
    int maxLength;
    float airCratedProbability[3][3];
    float sameColorProbability;
    float colorProbability[VF_BRICK_COUNT];
} GenerationParams;


static GenerationParams gps[_LEVEL_COUNT] = {
    {
        airPeriodMin: 6,
        airPeriodMax: 10,
        airRowMax: 2,
        cratePeriodMin: 3,
        cratePeriodMax: 10,
        crateRowMax: 4,
        cratePassCount: 3,
        airCratedProbability: {
                {0.1, 0.2, 0.1},
                {0.3, 0.0, 0.3},
                {0.1, 0.3, 0.1}
            },
        maxLength: 1
    },
    {
        airPeriodMin: 6,
        airPeriodMax: 10,
        airRowMax: 2,
        cratePeriodMin: 2,
        cratePeriodMax: 10,
        crateRowMax: 4,
        cratePassCount: 2,
        airCratedProbability: {
                {0.1, 0.2, 0.1},
                {0.3, 0.0, 0.3},
                {0.1, 0.3, 0.1}
            },
        maxLength: 1
    },
};

static void generateBrickVPeriods(FieldType ft, int vPrMin, int vPrMax, int rowMax, int width, int height, FieldType **map)
{
    int x, y, i;

    y = 0;
    y += (common_RandI() % (vPrMax - vPrMin + 1)) + vPrMin;

    while(y < height)
    {
        int hCount = common_RandI() % rowMax + 1;

        for(i = 0; i < hCount; ++i)
        {
            do { x = common_RandI() % width; } while(map[x][y] == ft);
            map[x][y] = ft;
        }
        y += (common_RandI() % (vPrMax - vPrMin + 1)) + vPrMin;
    }
}


void generator_AllocMap(FieldType ***pmap, int height, int level)
{
    int i, j, x, y;

    FieldType **map;
    bool **checked;
    GenerationParams gp = gps[level];
    int width = _MAP_WIDTH;

    srand(time(NULL));

    map = (FieldType**)common_Alloc2DTable(width, height, sizeof(FieldType));
    checked = (bool**)common_Alloc2DTable(width, height, sizeof(bool));


    for(y = 0; y < height; ++y)
    {
        for(x = 0; x < width; ++x)
        {
            map[x][y] = VF_NONE;
            checked[x][y] = false;
        }
    }


    int count = 0;
    int total = width * height;

    while(count < total)
    {
        do
        {
            x = common_RandI() % width;
            y = common_RandI() % height;
        }
        while(map[x][y] != VF_NONE);

        FieldType ft = common_RandI() % VF_BRICK_COUNT;
        Direction dir = common_RandI() % DIR_COUNT;
        int maxLen = common_RandI() % gp.maxLength + 1;
        int len = 0;
        int tx = x;
        int ty = y;

        do
        {
            map[tx][ty] = ft;
            len++;

            if(tx > 0 && dir == DIR_LEFT)
                if(map[tx - 1][ty] == VF_NONE)
                {
                    tx--;
                    continue;
                }

            if(ty > 0 && dir == DIR_UP)
                if(map[tx][ty - 1] == VF_NONE)
                {
                    ty--;
                    continue;
                }

            if(tx < (width - 1) && dir == DIR_RIGHT)
                if(map[tx + 1][ty] == VF_NONE)
                {
                    tx++;
                    continue;
                }

            if(ty < (height - 1) && dir == DIR_DOWN)
                if(map[tx][ty + 1] == VF_NONE)
                {
                    ty++;
                    continue;
                }

            break;

        } while(len < maxLen);

        count += len;
    }

    generateBrickVPeriods(VF_AIR, gp.airPeriodMin, gp.airPeriodMax, gp.airRowMax, width, height, map);

    for(i = 0; i < gp.cratePassCount; ++ i)
        generateBrickVPeriods(VF_CRATE, gp.cratePeriodMin, gp.cratePeriodMax, gp.airRowMax, width, height, map);

    for(y = 0; y < height; ++y)
        for(x = 0; x < width; ++x)
            if(map[x][y] == VF_AIR)
                for(j = -1; j <= 1; ++j)
                    for(i = -1; i <= 1; ++i)
                        if(INBOUND(x + i, y + j, width, height) &&
                           common_RandD() < gp.airCratedProbability[j + 1][i + 1])
                            map[x + i][y + j] = VF_CRATE;

    *pmap = map;

    common_Free2DTable((void**)checked, _MAP_WIDTH);
}

void generator_FreeMap(FieldType ***map)
{
    common_Free2DTable((void**)*map, _MAP_WIDTH);
    *map = NULL;
}



