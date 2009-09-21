#include "level.h"

#include <math.h>
#include <stdlib.h>

#include "defs.h"
#include "common.h"
#include "sprites.h"
#include "brickshape.h"
#include "stack.h"

typedef struct
{
    int x;
    int y;
} IntPoint;

typedef struct
{
    Sprite *sprite;
    Particle *particle;
    FieldType type;
    FieldState state;
    float timer;
    Point shift;
    bool supported;
    bool checked;
    bool justhit;
    int shakedir;
    unsigned char crateState;
} MapField;

typedef struct
{
    bool hit;
    bool destroyed;
    Sprite *sprite;
    Particle *particle;
    float timer;
} InterField;

typedef struct
{
    SpriteClassId air;
    SpriteClassId crate;
    SpriteClassId crateHit[4];
    SpriteClassId interBrick;
    SpriteClassId bricks[VF_BRICK_COUNT][_BRICK_TYPE_COUNT];
    SpriteClassId destroyAnim[VF_LAST];
    SpriteClassId destroyParticle[_EXPLODE_PARTICLE_TYPE_COUNT];
} LevelSpriteClasses;

static void generateMaps();
static void loadScids();
static void addExplodeParticles(float x, float y);
static void updateBrickShape(MapField **cMap, const LevelSpriteClasses *pScids);
static void allocSprites(MapField **cMap, LevelSpriteClasses *pScids, float vertShift);
static float getShakeShift(int y);
static void pushAdjacentSameBricks(Stack *pstack, IntPoint bp, FieldType ftype);
static void updateFallingUponShakingBricks();
static void findFallingBricks();
static void unsetShake();
static int bodySize(int x, int y);
static void destroyFallen();
static void setBrickDestroying(MapField *field, int x, int y);
static void interAreaAllocate(float vShift);
static void interAreaCleanup();
static void interAreaFrame(float lag);
static void interAreaHit(int x, int y);

static MapField **cMap;
static MapField ***maps;
static int mapHeight;
static int mapWidth = _MAP_WIDTH;

static InterField **interArea = NULL;

static LevelSpriteClasses *cScids;
static LevelSpriteClasses **scids;

static Stack bodyStack;
static Stack mainStack;
static Stack timerStack;
static Stack shiftStack;

static float vShift;

static bool needShapeUpdate;


float level_GetOffset(int levelNum)
{
    /* -1 because first row of every map is empty so the player can start there */
    return (float)(_BRICK_HEIGHT * (_INTER_ROW_COUNT + mapHeight - 1) * levelNum);
}

void level_Advance(int hitx, int nextLevel)
{
    interAreaHit(hitx, 0);

    int x, y;

    int visibleCutoff = mapHeight - _VISIBLE_ROWS;

    for(x = 0; x < mapWidth; ++x)
        for(y = 0; y < visibleCutoff; ++y)
            if(cMap[x][y].sprite != NULL || cMap[x][y].type != VF_NONE)
            {
                cMap[x][y].sprite->destroy = true;
                cMap[x][y].type = VF_NONE;
                cMap[x][y].sprite = NULL;
            }


    for(x = 0; x < mapWidth; ++x)
        for(y = visibleCutoff; y < mapHeight; ++y)
        {
            MapField *f = &cMap[x][y];

            if(f->sprite != NULL || f->type != VF_NONE)
            {
                if(f->type < VF_BRICK_COUNT)
                {
                    f->sprite->sclass = cScids->bricks[f->type][_BRICK_SINGLE_TYPE_NR];
                    f->sprite->frame = 0;
                }

                Particle *p;

                if(f->state == FS_BLINK && f->particle != NULL)
                {
                    p = f->particle;
                    p->flags = 0;
                }
                else
                    p = particles_Add(f->sprite);

                const Point vector = {
                                        (float)x - ((float)mapWidth / 2.0),
                                        (float)y - (float)mapHeight - 15.0
                                        };

                particles_SetFading(p, _LEVEL_ADVANCE_FADE_SPEED, true);
                particles_SetVelocityFromNormalizedVector(p, vector, _LEVEL_ADVANCE_VELOCITY);

                f->type = VF_NONE;
                f->sprite = NULL;
            }
        }

    float vertShift = level_GetOffset(nextLevel);

    allocSprites(maps[nextLevel], scids[nextLevel], vertShift);

    cMap = maps[nextLevel];
    cScids = scids[nextLevel];
    vShift = vertShift;


}

static void interAreaAllocate(float vertShift)
{
    interArea = (InterField**)common_Alloc2DTable(_MAP_WIDTH, _INTER_ROW_COUNT, sizeof(InterField));

    vertShift += (float)mapHeight * (float)_BRICK_HEIGHT;

    int x, y;

    for(x = 0; x < _MAP_WIDTH; ++x)
    {
        float xpos = (float)_BRICK_WIDTH * (float)x;
        float ypos = vertShift;

        for(y = 0; y < _INTER_ROW_COUNT; ++y)
        {
            interArea[x][y].hit = false;
            interArea[x][y].sprite = snge_AddSprite(cScids->interBrick, point(xpos, ypos), _INTER_LAYER);
            interArea[x][y].timer = 0.0;
            interArea[x][y].destroyed = false;
            ypos += (float)_BRICK_HEIGHT;
        }
    }
}

static inline void interAreaHit(int x, int y)
{
    if(x < 0 || y < 0 || x >= _MAP_WIDTH || y >= _INTER_ROW_COUNT || interArea == NULL)
        return;

    InterField *f = &interArea[x][y];

    if(f->destroyed || f->hit)
        return;

    f->hit = true;

    f->particle = particles_Add(f->sprite);
    particles_SetFading(f->particle, _INTER_FADE_SPEED, true);

    addExplodeParticles(f->sprite->x + (float)_BRICK_WIDTH / 2.0,
                        f->sprite->y + (float)_BRICK_HEIGHT / 2.0);
    f->sprite = NULL;
}


static void interAreaFrame(float lag)
{
    if(interArea == NULL)
        return;

    int x, y;
    bool done = true;

    for(x = 0; x < _MAP_WIDTH; ++x)
        for(y = 0; y < _INTER_ROW_COUNT; ++y)
        {
            InterField *f = &interArea[x][y];

            if(!f->destroyed)
                done = false;

            if(!f->hit || f->destroyed)
                continue;

            done = false;

            f->timer += lag;

            if(f->timer >= _INTER_BLOW_DELAY)
            {
                f->destroyed = true;
                interAreaHit(x - 1, y);
                interAreaHit(x + 1, y);
                interAreaHit(x, y + 1);
            }
        }

    /* Automatic cleaning up.
       If every brick is either hit or destroyed
       there is nothing left to do... */
    if(done)
        interAreaCleanup();
}

static void interAreaCleanup()
{
    common_Free2DTable((void**)interArea, _MAP_WIDTH);
    interArea = NULL;
}

static void generateMaps()
{
    int i;

    maps = malloc(sizeof(MapField**) * _LEVEL_COUNT);

    for(i = 0; i < _LEVEL_COUNT; ++i)
    {
        MapField **tMap = (MapField**)common_Alloc2DTable(mapWidth, mapHeight, sizeof(MapField));

        maps[i] = tMap;

        FieldType **tmp;
        generator_AllocMap(&tmp, mapHeight, i);

        int x, y;

        for(x = 0; x < mapWidth; ++x)
            for(y = 0; y < mapHeight; ++y)
            {
                tMap[x][y].type = tmp[x][y];

                if(y == 0)
                {
                    tMap[x][y].type = VF_NONE;
                }

                tMap[x][y].shift = point(0, 0);
                tMap[x][y].state = FS_NORM;
                tMap[x][y].particle = NULL;
                tMap[x][y].justhit = false;
                tMap[x][y].crateState = 0;
            }

        generator_FreeMap(&tmp);

        if(i == 0)
        {
            int j;
            for(j = 0; j < mapWidth; ++j)
            {
                tMap[j][0].type = VF_NONE;
                tMap[j][1].type = VF_CRATE;

                int k;

                for(k = 0; k < VF_BRICK_COUNT; ++k)
                    if(abs((mapWidth / 2) - j) <= k)
                        tMap[j][k + 1].type = k;
                else
                    tMap[j][k + 1].type = VF_CRATE;

            }
        }
    }

    cMap = maps[0];

}

static void loadScids()
{
    int i, j;

    scids = malloc(sizeof(LevelSpriteClasses*) * _LEVEL_COUNT);

    for(i = 0; i < _LEVEL_COUNT; ++i)
    {
        scids[i] = malloc(sizeof(LevelSpriteClasses));
        LevelSpriteClasses *pscids = scids[i];

        /* get ids of sprites' classes */

        pscids->air = sprites_GetIdByName("level_common:air");
        pscids->crate = sprites_GetIdByName("level_common:crate");
        pscids->interBrick = sprites_GetIdByName("level_common:brick-inter");

        for(j = 0; j < 4; ++j)
            pscids->crateHit[j] = sprites_GetIdByNameF("level_common:crate_hit_%d", j + 1);

        for(j = 0; j < _EXPLODE_PARTICLE_TYPE_COUNT; ++j)
            pscids->destroyParticle[j] = sprites_GetIdByNameF("level_common:dparticle-%d", j + 1);

        pscids->destroyAnim[VF_CRATE] = sprites_GetIdByName("level_common:crate-destroy");

        pscids->destroyAnim[VF_BRICK_RED] = sprites_GetIdByNameF("level_%d:brick-destroy-red", i);
        pscids->destroyAnim[VF_BRICK_GREEN] = sprites_GetIdByNameF("level_%d:brick-destroy-green", i);
        pscids->destroyAnim[VF_BRICK_BLUE] = sprites_GetIdByNameF("level_%d:brick-destroy-blue", i);
        pscids->destroyAnim[VF_BRICK_YELLOW] = sprites_GetIdByNameF("level_%d:brick-destroy-yellow", i);

        for(j = 0; j < _BRICK_TYPE_COUNT; ++j)
        {
            pscids->bricks[VF_BRICK_RED][j] = sprites_GetIdByNameF("level_%d:brick%02d-red", i, j + 1);
            pscids->bricks[VF_BRICK_GREEN][j] = sprites_GetIdByNameF("level_%d:brick%02d-green", i, j + 1);
            pscids->bricks[VF_BRICK_BLUE][j] = sprites_GetIdByNameF("level_%d:brick%02d-blue", i, j + 1);
            pscids->bricks[VF_BRICK_YELLOW][j] = sprites_GetIdByNameF("level_%d:brick%02d-yellow", i, j + 1);
        }
    }

    cScids = scids[0];
}

static void addExplodeParticles(float x, float y)
{
    Point p = {x: x, y: y};
    int i;

    for(i = 0; i < _EXPLODE_PARTICLE_COUNT; ++i)
    {
        Sprite *s = snge_AddSprite(cScids->destroyParticle[rand() % _EXPLODE_PARTICLE_TYPE_COUNT], p, _EXPLODE_PARTICLES_LAYER);
        Particle *p = particles_Add(s);
        particles_SetVelocityDegrees(p, rand() % 360, _EXPLODE_PARTICLE_MAX_SPEED * common_RandD(), true);
        particles_SetFading(p, _EXPLODE_PARTICLE_FADE_SPEED, true);
        p->rotateSpeed = _EXPLODE_PARTICLE_MAX_ROT_SPEED * common_RandD();
    }

}

static void updateBrickShape(MapField **tMap, const LevelSpriteClasses *pScids)
{
    int x, y;
    MapField *f;
    int k = 0;
    unsigned char fits;
    unsigned char test;
    FieldType vt;

    int i, j;
    int xi, yj;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
        {
            f = &tMap[x][y];

            if(f->type < VF_BRICK_COUNT)
            {

                k = 0;

                do
                {
                    fits = 1;
                    j = -1;

                    while(j != 2 && fits)
                    {
                        i = -1;

                        while(i != 2 && fits)
                        {
                            vt = VF_NONE;

                            test = brickShape[k][j + 1][i + 1];

                            xi = x + i;
                            yj = y + j;

                            if(!(xi == -1 || yj == -1 || xi == mapWidth || yj == mapHeight))
                                vt = tMap[xi][yj].type;

                            if(test != 2 && (vt == f->type) != test)
                                fits = 0;

                            ++i;
                        }

                        ++j;
                    }


                    ++k;

                } while(!fits && k < _BRICK_TYPE_COUNT);

                f->sprite->sclass = pScids->bricks[f->type][k - 1];

            }
        }
}

static void allocSprites(MapField **tMap, LevelSpriteClasses *pScids, float vertShift)
{
    int x, y;
    Point s;
    FieldType vf;

    for(x = 0; x < mapWidth; ++x)
        for(y = 0; y < mapHeight; ++y)
        {
            vf = tMap[x][y].type;

            s.x = _BRICK_WIDTH * x;
            s.y = (_BRICK_HEIGHT * y) + vertShift;

            tMap[x][y].sprite = NULL;

            if(vf < VF_BRICK_COUNT)
                tMap[x][y].sprite = snge_AddSprite(pScids->bricks[vf][0], s, _BRICKS_LAYER);
            else if(vf == VF_CRATE)
                tMap[x][y].sprite = snge_AddSprite(pScids->crate, s, _BRICKS_LAYER);
            else if(vf == VF_AIR)
                tMap[x][y].sprite = snge_AddSprite(pScids->air, s, _BRICKS_LAYER);
        }

    updateBrickShape(tMap, pScids);
}

inline static float getShakeShift(int y)
{
    float pos = (float)y / (float)_VISIBLE_ROWS;
    return _SHAKE_MAX_SHIFT * sinf(pos * 4.0 * M_PI);
}

inline static void pushAdjacentSameBricks(Stack *pstack, IntPoint bp, FieldType ftype)
{
    IntPoint tmp;

    if((bp.x + 1) < mapWidth)
        if(cMap[bp.x + 1][bp.y].type == ftype)
        {
            tmp = bp;
            tmp.x++;
            stackPush(pstack, &tmp);
        }

    if((bp.y + 1) < mapHeight)
        if(cMap[bp.x][bp.y + 1].type == ftype)
        {
            tmp = bp;
            tmp.y++;
            stackPush(pstack, &tmp);
        }

    if(bp.x > 0)
        if(cMap[bp.x - 1][bp.y].type == ftype)
        {
            tmp = bp;
            tmp.x--;
            stackPush(pstack, &tmp);
        }

    if(bp.y > 0)
        if(cMap[bp.x][bp.y - 1].type == ftype)
        {
            tmp = bp;
            tmp.y--;
            stackPush(pstack, &tmp);
        }

}

static void updateFallingUponShakingBricks()
{
    int x, y;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
        {
            IntPoint p = {x: x, y: y};

            if(cMap[x][y].type != VF_NONE && cMap[x][y].state == FS_SHAKE)
            {
                stackPush(&mainStack, &p);
                stackPush(&timerStack, &cMap[x][y].timer);
                stackPush(&shiftStack, &cMap[x][y].shift.x);
            }

            cMap[x][y].checked = false;
        }

    while(stackNotEmpty(&mainStack))
    {
        IntPoint sp = *((IntPoint*)stackPop(&mainStack));
        float timer = *((float*)stackPop(&timerStack));
        float shift = *((float*)stackPop(&shiftStack));

        stackPush(&bodyStack, &sp);

        while(stackNotEmpty(&bodyStack))
        {
            IntPoint bp = *((IntPoint*)stackPop(&bodyStack));
            MapField *field = &cMap[bp.x][bp.y];

            if(field->checked)
                /* already checked */
                continue;

            field->checked = true;
            field->shift.y = 0;
            field->timer = timer;
            field->state = FS_SHAKE;
            field->shift.x = shift;

            IntPoint upper = bp;
            upper.y--;

            if(bp.y > 0)
                if(cMap[upper.x][upper.y].type != VF_NONE)
                    if(VF_DOES_MERGE(field->type) ? cMap[upper.x][upper.y].type != field->type : 1)
                        if(cMap[upper.x][upper.y].state == FS_FALL && !cMap[upper.x][upper.y].supported)
                        {
                            stackPush(&mainStack, &upper);
                            stackPush(&timerStack, &timer);

                            float upshift = getShakeShift(upper.y);

                            stackPush(&shiftStack, &upshift);
                        }

            if(!VF_DOES_MERGE(field->type))
                continue;

            pushAdjacentSameBricks(&bodyStack, bp, field->type);

        }
    }
}

static void findFallingBricks()
{
    int x, y;
    IntPoint p;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
        {
            cMap[x][y].supported = false;
            cMap[x][y].checked = false;
        }

    p.y = mapHeight - 1;

    /* Bottom fields have support by definition. */
    for(x = 0; x < mapWidth; ++x)
    {
        p.x = x;
        if(cMap[p.x][p.y].type != VF_NONE)
            stackPush(&mainStack, &p);
    }


    while(stackNotEmpty(&mainStack))
    {
        IntPoint sp = *((IntPoint*)stackPop(&mainStack));

        /*
            Follow the body shape because whole of it
            has support.
        */
        stackPush(&bodyStack, &sp);

        while(stackNotEmpty(&bodyStack))
        {
            IntPoint bp = *((IntPoint*)stackPop(&bodyStack));
            MapField *field = &cMap[bp.x][bp.y];

            if(field->checked)
                /* already checked */
                continue;

            field->checked = true;
            field->supported = true;

            IntPoint upper = bp;
            upper.y--;

            /*
                Push upper field to mainStack because it
                certainly has support by lying on top
                of a supported field.
            */
            if(bp.y > 0)
                if(cMap[upper.x][upper.y].type != VF_NONE)
                    if(VF_DOES_MERGE(field->type) ? cMap[upper.x][upper.y].type != field->type : 1)
                        stackPush(&mainStack, &upper);

            /*
                If it isn't a brick it doesn't merge so skip
                the recursion;
            */
            if(!VF_DOES_MERGE(field->type))
                continue;

            /* Body following recursion. */

            pushAdjacentSameBricks(&bodyStack, bp, field->type);
        }
    }

    /* set not supported bricks shaking */

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
            cMap[x][y].checked = false;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
            if(!cMap[x][y].supported && cMap[x][y].state == FS_NORM)
            {
                IntPoint pp = {x: x, y: y};

                stackPush(&bodyStack, &pp);

                float hshift = getShakeShift(y);

                while(stackNotEmpty(&bodyStack))
                {
                    IntPoint bp = *((IntPoint*)stackPop(&bodyStack));
                    MapField *field = &cMap[bp.x][bp.y];

                    if(field->checked)
                        continue;

                    field->checked = true;
                    field->state = FS_SHAKE;
                    field->shakedir = -1;
                    field->timer = 0;
                    field->shift.x = hshift;

                    if(!VF_DOES_MERGE(field->type))
                        continue;

                    pushAdjacentSameBricks(&bodyStack, bp, field->type);

                }
            }

}

static void unsetShake()
{
    int x, y;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
            if(cMap[x][y].supported && cMap[x][y].state == FS_SHAKE)
            {
                cMap[x][y].state = FS_NORM;
                cMap[x][y].shift.x = 0;
            }

}

static int bodySize(int x, int y)
{
    IntPoint p = {x: x, y: y};

    int size = 0;
    stackPush(&bodyStack, &p);

    while(stackNotEmpty(&bodyStack))
    {
        IntPoint bp = *((IntPoint*)stackPop(&bodyStack));
        MapField *field = &cMap[bp.x][bp.y];

        if(!VF_DOES_MERGE(field->type) || field->checked)
            continue;

        field->checked = true;
        size++;
        pushAdjacentSameBricks(&bodyStack, bp, field->type);
    }

    return size;

}

static void destroyFallen()
{
    int x, y;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
            cMap[x][y].checked = false;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
        {
            MapField *f = &cMap[x][y];

            if(f->justhit && VF_DOES_MERGE(f->type) && f->state == FS_NORM && !f->checked)
                if(bodySize(x, y) >= _DESTROY_SIZE_THRESHOLD)
                    level_DestroyBrick(x, y, true);
        }
}


static void setBrickDestroying(MapField *field, int x, int y)
{
    if(field->particle == NULL)
        field->particle = particles_Add(field->sprite);

    field->sprite->sclass = cScids->destroyAnim[field->type];
    field->sprite->frame = 0;
    particles_DestroyOnAnimationEnd(field->particle);
    addExplodeParticles(field->sprite->x + (float)_BRICK_WIDTH / 2.0,
                        field->sprite->y + (float)_BRICK_HEIGHT / 2.0);
    field->sprite = NULL;
    field->state = FS_VANISH;
    field->type = VF_NONE;
}

bool level_HitCrate(int x, int y)
{
    MapField *field = &cMap[x][y];

    if(field->type != VF_CRATE)
        return false;

    field->crateState++;

    if(field->crateState == 5)
    {
        setBrickDestroying(field, x, y);
        return true;
    }

    field->sprite->sclass = cScids->crateHit[field->crateState - 1];
    field->sprite->frame = 0;

    return false;
}

inline void level_DeleteField(int x, int y)
{
    MapField *f = &cMap[x][y];

    if(f->sprite != NULL)
        f->sprite->destroy = true;

    f->type = VF_NONE;
    f->sprite = NULL;
    f->state = FS_NORM;
}

bool level_IsAirGetAir(int x, int y, Sprite **psprite)
{
    if(cMap[x][y].type != VF_AIR)
        return false;

    *psprite = cMap[x][y].sprite;

    cMap[x][y].sprite = NULL;
    cMap[x][y].type = VF_NONE;
    cMap[x][y].supported = 0;
    cMap[x][y].justhit = 0;
    cMap[x][y].state = FS_NORM;

    return true;

}

void level_DestroyBrick(int x, int y, bool blink)
{
    IntPoint p = {x: x, y: y};

    stackPush(&bodyStack, &p);

    while(stackNotEmpty(&bodyStack))
    {
        IntPoint bp = *((IntPoint*)stackPop(&bodyStack));
        MapField *field = &cMap[bp.x][bp.y];
        FieldType type = field->type;

        if(blink ? field->state == FS_BLINK : field->state == FS_VANISH)
            continue;

        if(field->sprite != NULL)
        {
            field->particle = particles_Add(field->sprite);

            if(blink)
            {
                particles_SetBlinking(field->particle, _DESTROY_BLINK_FREQUENCY);
                field->timer = 0;
                field->state = FS_BLINK;
            }
            else
                setBrickDestroying(field, bp.x, bp.y);
        }

        if(!VF_DOES_MERGE(type))
            continue;

        pushAdjacentSameBricks(&bodyStack, bp, type);
    }
}

int level_IsSolid(int x, int y)
{
    if(x >= mapWidth || x < 0 || y >= mapHeight || y < 0)
        return 1;

    FieldType f = cMap[x][y].type;

    if(f < VF_BRICK_COUNT || f == VF_CRATE)
        return 1;

    return 0;
}

FieldType level_GetFieldType(int x, int y)
{
    if(!INBOUND(x, y, mapWidth, mapHeight))
        return VF_VOID;

    return cMap[x][y].type;
}

void level_Frame(float lag)
{
    int x, y;
    float fallDelta = _SHAKE_SPEED * lag;

    needShapeUpdate = false;

    findFallingBricks();
    unsetShake();
    updateFallingUponShakingBricks();
    destroyFallen();

    interAreaFrame(lag);

    for(y = mapHeight - 1; y >= 0; --y)
        for(x = 0; x < mapWidth; ++x)
        {

            MapField *f = &cMap[x][y];

            f->justhit = false;

            if(f->type == VF_NONE)
                continue;

            if(f->sprite == NULL)
                continue;

            if(f->state == FS_BLINK)
            {
                f->timer += lag;
                if(f->timer >= _BLINK_TIME_BEFORE_DESTROY)
                {
                    particles_UnsetFlag(f->particle, PF_BLINKING);
                    setBrickDestroying(f, x, y);
                }
            }


            if(f->state == FS_SHAKE)
            {
                f->timer += lag;
                f->shift.x += (float)f->shakedir * fallDelta;

                if(f->shift.x < -_SHAKE_MAX_SHIFT)
                {
                    f->shakedir = 1;
                    f->shift.x = -(2.0 * _SHAKE_MAX_SHIFT + f->shift.x);
                }

                if(f->shift.x > _SHAKE_MAX_SHIFT)
                {
                    f->shakedir = -1;
                    f->shift.x = 2.0 * _SHAKE_MAX_SHIFT - f->shift.x;
                }

                if(f->timer >= _SHAKE_TIME)
                {
                    f->shift.x = 0;
                    f->state = FS_FALL;
                }

            }
            /* a brick falls on a shaking brick */
            /* TODO: what if brick shakes and suddenly merges with falling bricks causing it to gain support */

            if(f->type != VF_NONE && f->state == FS_FALL)
            {
                if(f->supported)
                {
                    f->shift.y = 0;
                    f->state = FS_NORM;
                    needShapeUpdate = true;
                    f->justhit = true;
                }
                else
                {
                    f->shift.y += lag * _BRICK_FALL_SPEED;

                    if(f->shift.y >= _BRICK_HEIGHT)
                    {
                        cMap[x][y + 1] = cMap[x][y];
                        cMap[x][y + 1].shift.y = 0;

                        cMap[x][y].sprite = NULL;
                        cMap[x][y].type = VF_NONE;
                        needShapeUpdate = true;
                    }
                }
            }


        }

    for(y = 0; y < mapHeight; ++y)
    {
        float ypos = y * _BRICK_HEIGHT + vShift;

        for(x = 0; x < mapWidth; ++x)
        {
            float xpos = _BRICK_WIDTH * x;
            MapField *f = &cMap[x][y];

            if(f->sprite != NULL)
            {
                f->sprite->x = xpos + f->shift.x;
                f->sprite->y = ypos + f->shift.y;
            }
        }
    }

    if(needShapeUpdate)
        updateBrickShape(cMap, cScids);

}

void level_Init(int levelHeight)
{
    bodyStack = stackAlloc(sizeof(IntPoint), 5);
    mainStack = stackAlloc(sizeof(IntPoint), 5);
    timerStack = stackAlloc(sizeof(float), 5);
    shiftStack = stackAlloc(sizeof(float), 5);

    needShapeUpdate = false;
    mapHeight = levelHeight;

    loadScids();
    generateMaps();

    allocSprites(cMap, cScids, 0.0);
    interAreaAllocate(0.0);

    vShift = 0.0;
}

void level_Cleanup()
{
    int i;

    for(i = 0; i < _LEVEL_COUNT; ++i)
        free(scids[i]);

    free(scids);

    for(i = 0; i < _LEVEL_COUNT; ++i)
        common_Free2DTable((void**)maps[i], mapWidth);

    free(maps);

    stackFree(bodyStack);
    stackFree(mainStack);
    stackFree(timerStack);
    stackFree(shiftStack);
}

