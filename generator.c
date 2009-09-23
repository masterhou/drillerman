#include "generator.h"


#include <stdlib.h>
#include <time.h>

typedef struct 
{
    int airPeriodMin;
    int airPeriodMax;
    float airCratedProbability[3][3];
    float sameColorProbability;
    float colorProbability[VF_BRICK_COUNT];
} GenerationParams;


static GenerationParams gps[_LEVEL_COUNT] = {
    {
        airPeriodMin: 6,
        airPeriodMax: 10,
        airCratedProbability: {
                {0.1, 0.2, 0.1},
                {0.3, 0.0, 0.3},
                {0.1, 0.3, 0.1}
            },
        sameColorProbability: 0.00,
        colorProbability: {0.3, 0.3, 0.3, 0.3}
    },
    {
        airPeriodMin: 6,
        airPeriodMax: 10,
        airCratedProbability: {
                {0.1, 0.2, 0.1},
                {0.3, 0.0, 0.3},
                {0.1, 0.3, 0.1}
            },
        sameColorProbability: 0.00,
        colorProbability: {0.3, 0.3, 0.3, 0.3}
    }
};


void generator_AllocMap(FieldType ***pmap, int height, int level)
{
    int i, j, x, y;

    FieldType **map;
    GenerationParams gp = gps[level];
    int width = _MAP_WIDTH;

    srand(time(NULL));

    map = (FieldType **)common_Alloc2DTable(width, height, sizeof(FieldType));

    for(y = 0; y < height; ++y)
        for(x = 0; x < width; ++x)
            map[x][y] = VF_NONE;

    for(y = 0; y < height; ++y)
    {
        for(x = 0; x < width; ++x)
        {
            while(map[x][y] == VF_NONE)
            {
                if(y > 0)
                {
                    FieldType vf = map[x][y - 1];

                    if(common_RandD() < (gp.sameColorProbability * gp.colorProbability[vf]))
                    {
                        map[x][y] = vf;
                        continue;
                    }
                }

                if(x > 0)
                {
                    FieldType vf = map[x - 1][y];

                    if(common_RandD() < (gp.sameColorProbability * gp.colorProbability[vf]))
                    {
                        map[x][y] = vf;
                        continue;
                    }
                }


                map[x][y] = common_RandI() % VF_BRICK_COUNT;

                if(common_RandD() >= gp.colorProbability[map[x][y]])
                    map[x][y] = VF_NONE;
            }

        }
    }

    y = 0;

    while(y < height)
    {
        x = common_RandI() % width;
        map[x][y] = VF_AIR;

        for(j = -1; j <= 1; ++j)
            for(i = -1; i <= 1; ++i)
                if(INBOUND(x + i, y + j, width, height) &&
                   common_RandD() < gp.airCratedProbability[j + 1][i + 1])
                    map[x + i][y + j] = VF_CRATE;
        y += (common_RandI() % (gp.airPeriodMax - gp.airPeriodMin + 1)) + gp.airPeriodMin;
    }

    *pmap = map;


}

void generator_FreeMap(FieldType ***map)
{
    common_Free2DTable((void**)*map, _MAP_WIDTH);
    *map = NULL;
}



