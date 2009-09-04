#include "generator.h"

#include <stdlib.h>
#include <time.h>

typedef struct 
{
    int airPeriodMin;
    int airPeriodMax;
    float airCratedProbability[3][3];
    float sameColorFactor;
    float colorProbability[VF_BRICK_COUNT];
} GenerationParams;


static GenerationParams gps[DF_LAST] = {
    {
        airPeriodMin: 6,
        airPeriodMax: 10,
        airCratedProbability: {
                {0.1, 0.2, 0.1},
                {0.3, 0.0, 0.3},
                {0.1, 0.3, 0.1}
            },
        sameColorFactor: 1.0,
        colorProbability: {0.4, 0.3, 0.2, 0.1}
    }
};


void generatorAllocMap(FieldType ***pmap, int height, Difficulty difficulty)
{
    int i, j, x, y;

    FieldType **map;
    GenerationParams gp = gps[difficulty];
    int width = _MAP_WIDTH;

    srand(time(NULL));

    map = (FieldType **)commonAlloc2DTable(width, height, sizeof(FieldType));

    for(y = 0; y < height; ++y)
        for(x = 0; x < width; ++x)
            map[x][y] = VF_NONE;

    for(y = 0; y < height; ++y)
        for(x = 0; x < width; ++x)
            do
            {
        FieldType vf = commonRandI() % VF_BRICK_COUNT;

        int is_there = 0;

        if(y > 0)
            if(map[x][y - 1] == vf)
                is_there = 1;
        if(x > 0)
            if(map[x - 1][y] == vf)
                is_there = 1;

        map[x][y] = vf;

        if(is_there && commonRandD() >= gp.sameColorFactor)
            map[x][y] = VF_NONE;
        if(commonRandD() >= gp.colorProbability[vf])
            map[x][y] = VF_NONE;

    } while(map[x][y] == VF_NONE);

    y = 0;

    while(y < height)
    {
        x = commonRandI() % width;
        map[x][y] = VF_AIR;

        for(j = -1; j <= 1; ++j)
            for(i = -1; i <= 1; ++i)
                if(INBOUND(x + i, y + j, width, height) &&
                   commonRandD() < gp.airCratedProbability[j + 1][i + 1])
                    map[x + i][y + j] = VF_CRATE;
        y += (commonRandI() % (gp.airPeriodMax - gp.airPeriodMin + 1)) + gp.airPeriodMin;
    }

    *pmap = map;


}

void generatorFreeMap(FieldType ***map, int height)
{
    commonFree2DTable((void**)*map, height);
    map = NULL;
}



