#include "gamemap.h"

#include <math.h>
#include <stdlib.h>

#include "defaults.h"
#include "common.h"
#include "sprites.h"
#include "brickshape.h"
#include "stack.h"

typedef struct
{
    int x;
    int y;
} PointI;

static MapField **map;
static int mapHeight;
static int mapWidth = _MAP_WIDTH;

static SpriteClassId scidAir;
static SpriteClassId scidCrate;
static SpriteClassId scidCrateHit[4];
static SpriteClassId scidBricks[VF_BRICK_COUNT][_BRICK_TYPE_COUNT];
static SpriteClassId scidDestroyAnim[VF_LAST];
static SpriteClassId scidDestroyParticle[_DESTROY_PARTICLE_TYPE_COUNT];

static bool needShapeUpdate;

static void addDestroyParticles(int x, int y)
{
    Point p = {x: x * _BRICK_WIDTH + _BRICK_WIDTH / 2,
               y: y * _BRICK_HEIGHT + _BRICK_HEIGHT / 2};
    int i;

    for(i = 0; i < _DESTROY_PARTICLE_COUNT; ++i)
    {
        Sprite *s = sngeAddSprite(scidDestroyParticle[rand() % _DESTROY_PARTICLE_TYPE_COUNT], p, 10);
        Particle *p = particlesAdd(s);
        particlesSetVelocityDegrees(p, rand() % 360, _DESTROY_PARTICLE_MAX_SPEED * commonRandD(), true);
        particlesSetFading(p, _DESTROY_PARTICLE_FADE_SPEED, true);
        p->rotateSpeed = _DESTROY_PARTICLE_MAX_ROT_SPEED * commonRandD();
    }

}

void gameMapUpdateBrickShape()
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
            f = &map[x][y];

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
                                vt = map[xi][yj].type;

                            if(test != 2 && (vt == f->type) != test)
                                fits = 0;

                            ++i;
                        }

                        ++j;
                    }


                    ++k;

                } while(!fits && k < _BRICK_TYPE_COUNT);

                f->sprite->sclass = scidBricks[f->type][k - 1];

            }
        }
}

void gameMapAllocSprites()
{
    int x, y;
    Point s;
    FieldType vf;

    for(x = 0; x < mapWidth; ++x)
        for(y = 0; y < mapHeight; ++y)
        {
            vf = map[x][y].type;
            s.x = _BRICK_WIDTH * x;
            s.y = _BRICK_HEIGHT * y;

            map[x][y].sprite = NULL;

            if(vf < VF_BRICK_COUNT)
                map[x][y].sprite = sngeAddSprite(scidBricks[vf][0], s, 5);
            else if(vf == VF_CRATE)
                map[x][y].sprite = sngeAddSprite(scidCrate, s, 5);
            else if(vf == VF_AIR)
                map[x][y].sprite = sngeAddSprite(scidAir, s, 5);
        }

    gameMapUpdateBrickShape();
}

inline void gameMapDeleteField(int x, int y)
{
    MapField *f = &map[x][y];

    if(f->sprite != NULL)
        f->sprite->destroy = true;

    f->type = VF_NONE;
    f->sprite = NULL;
    f->state = FS_NORM;
}

inline static float getShakeShift(int y)
{
    float pos = (float)y / (float)_VISIBLE_ROWS;
    return _SHAKE_MAX_SHIFT * sinf((pos - truncf(pos)) * 2.0 * M_PI);
}

inline static void pushAdjacentSameBricks(Stack *pstack, PointI bp, FieldType ftype)
{
    PointI tmp;

    if((bp.x + 1) < mapWidth)
        if(map[bp.x + 1][bp.y].type == ftype)
        {
            tmp = bp;
            tmp.x++;
            stackPush(pstack, &tmp);
        }

    if((bp.y + 1) < mapHeight)
        if(map[bp.x][bp.y + 1].type == ftype)
        {
            tmp = bp;
            tmp.y++;
            stackPush(pstack, &tmp);
        }

    if(bp.x > 0)
        if(map[bp.x - 1][bp.y].type == ftype)
        {
            tmp = bp;
            tmp.x--;
            stackPush(pstack, &tmp);
        }

    if(bp.y > 0)
        if(map[bp.x][bp.y - 1].type == ftype)
        {
            tmp = bp;
            tmp.y--;
            stackPush(pstack, &tmp);
        }

}

bool gameMapIsAirGetAir(int x, int y, Sprite **psprite)
{
    if(map[x][y].type != VF_AIR)
        return false;

    *psprite = map[x][y].sprite;

    map[x][y].sprite = NULL;
    map[x][y].type = VF_NONE;
    map[x][y].supported = 0;
    map[x][y].justhit = 0;
    map[x][y].state = FS_NORM;

    return true;

}

static void updateFallingUponShakingBricks()
{
    int x, y;

    Stack bodyStack = stackAlloc(sizeof(PointI), 5);
    Stack mainStack = stackAlloc(sizeof(PointI), 5);
    Stack timerStack = stackAlloc(sizeof(float), 5);
    Stack shiftStack = stackAlloc(sizeof(float), 5);

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
        {
            PointI p = {x: x, y: y};

            if(map[x][y].type != VF_NONE && map[x][y].state == FS_SHAKE)
            {
                stackPush(&mainStack, &p);
                stackPush(&timerStack, &map[x][y].timer);
                stackPush(&shiftStack, &map[x][y].shift.x);
            }

            map[x][y].checked = false;
        }

    while(stackNotEmpty(&mainStack))
    {
        PointI sp = *((PointI*)stackPop(&mainStack));
        float timer = *((float*)stackPop(&timerStack));
        float shift = *((float*)stackPop(&shiftStack));

        stackPush(&bodyStack, &sp);

        while(stackNotEmpty(&bodyStack))
        {
            PointI bp = *((PointI*)stackPop(&bodyStack));
            MapField *field = &map[bp.x][bp.y];

            if(field->checked)
                /* already checked */
                continue;

            field->checked = true;
            field->shift.y = 0;
            field->timer = timer;
            field->state = FS_SHAKE;
            field->shift.x = shift;

            PointI upper = bp;
            upper.y--;

            if(bp.y > 0)
                if(map[upper.x][upper.y].type != VF_NONE)
                    if(VF_DOES_MERGE(field->type) ? map[upper.x][upper.y].type != field->type : 1)
                        if(map[upper.x][upper.y].state == FS_FALL && !map[upper.x][upper.y].supported)
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

    stackFree(mainStack);
    stackFree(bodyStack);
    stackFree(timerStack);
    stackFree(shiftStack);
}

static void findFallingBricks()
{
    int x, y;
    PointI p;

    Stack mainStack = stackAlloc(sizeof(PointI), 5);
    Stack bodyStack = stackAlloc(sizeof(PointI), 5);

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
        {
            map[x][y].supported = false;
            map[x][y].checked = false;
        }

    p.y = mapHeight - 1;

    /* Bottom fields have support by definition. */
    for(x = 0; x < mapWidth; ++x)
    {
        p.x = x;
        if(map[p.x][p.y].type != VF_NONE)
            stackPush(&mainStack, &p);
    }


    while(stackNotEmpty(&mainStack))
    {
        PointI sp = *((PointI*)stackPop(&mainStack));

        /*
            Follow the body shape because whole of it
            has support.
        */
        stackClear(&bodyStack);
        stackPush(&bodyStack, &sp);

        while(stackNotEmpty(&bodyStack))
        {
            PointI bp = *((PointI*)stackPop(&bodyStack));
            MapField *field = &map[bp.x][bp.y];

            if(field->checked)
                /* already checked */
                continue;

            field->checked = true;
            field->supported = true;

            PointI upper = bp;
            upper.y--;

            /*
                Push upper field to mainStack because it
                certainly has support by lying on top
                of a supported field.
            */
            if(bp.y > 0)
                if(map[upper.x][upper.y].type != VF_NONE)
                    if(VF_DOES_MERGE(field->type) ? map[upper.x][upper.y].type != field->type : 1)
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

    stackClear(&bodyStack);

    /* set not supported bricks shaking */

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
            map[x][y].checked = false;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
            if(!map[x][y].supported && map[x][y].state == FS_NORM)
            {
                PointI pp = {x: x, y: y};

                stackPush(&bodyStack, &pp);

                float hshift = getShakeShift(y);

                while(stackNotEmpty(&bodyStack))
                {
                    PointI bp = *((PointI*)stackPop(&bodyStack));
                    MapField *field = &map[bp.x][bp.y];

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

    stackFree(bodyStack);
    stackFree(mainStack);

}

static void unsetShake()
{
    int x, y;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
            if(map[x][y].supported && map[x][y].state == FS_SHAKE)
            {
                map[x][y].state = FS_NORM;
                map[x][y].shift.x = 0;
            }

}

static int bodySize(int x, int y)
{
    Stack bodyStack = stackAlloc(sizeof(PointI), 5);
    PointI p = {x: x, y: y};

    int size = 0;
    stackPush(&bodyStack, &p);

    while(stackNotEmpty(&bodyStack))
    {
        PointI bp = *((PointI*)stackPop(&bodyStack));
        MapField *field = &map[bp.x][bp.y];

        if(!VF_DOES_MERGE(field->type) || field->checked)
            continue;

        field->checked = true;
        size++;
        pushAdjacentSameBricks(&bodyStack, bp, field->type);
    }

    return size;

    stackFree(bodyStack);
}

static void destroyFallen()
{
    int x, y;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
            map[x][y].checked = false;

    for(y = 0; y < mapHeight; ++y)
        for(x = 0; x < mapWidth; ++x)
        {
            MapField *f = &map[x][y];

            if(f->justhit && VF_DOES_MERGE(f->type) && f->state == FS_NORM && !f->checked)
                if(bodySize(x, y) >= _DESTROY_SIZE_THRESHOLD)
                    gameMapDestroyBrick(x, y, true);
        }
}
static void setBrickDestroying(MapField *field, int x, int y)
{
    if(field->particle == NULL)
        field->particle = particlesAdd(field->sprite);

    field->sprite->sclass = scidDestroyAnim[field->type];
    field->sprite->frame = 0;
    particlesDestroyOnAnimationEnd(field->particle);
    addDestroyParticles(x, y);
    field->sprite = NULL;
    field->state = FS_VANISH;
    field->type = VF_NONE;
}

void gameMapDestroyBrick(int x, int y, bool blink)
{
    Stack bodyStack = stackAlloc(sizeof(PointI), 5);
    PointI p = {x: x, y: y};

    stackPush(&bodyStack, &p);

    while(stackNotEmpty(&bodyStack))
    {
        PointI bp = *((PointI*)stackPop(&bodyStack));
        MapField *field = &map[bp.x][bp.y];
        FieldType type = field->type;

        if(blink ? field->state == FS_BLINK : field->state == FS_VANISH)
            continue;

        if(field->sprite != NULL)
        {
            field->particle = particlesAdd(field->sprite);

            if(blink)
            {
                particlesSetBlinking(field->particle, _DESTROY_BLINK_FREQUENCY);
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

    stackFree(bodyStack);
}

int gameMapIsSolid(int x, int y)
{
    if(x >= mapWidth || x < 0 || y >= mapHeight || y < 0)
        return 1;

    FieldType f = map[x][y].type;

    if(f < VF_BRICK_COUNT || f == VF_CRATE)
        return 1;

    return 0;
}

void gameMapInit(int height, Difficulty difficulty)
{
    int i;

    needShapeUpdate = false;

    /* get ids of sprites' classes */

    scidAir = spritesGetIdByName("air");
    scidCrate = spritesGetIdByName("crate");

    for(i = 0; i < 4; ++i)
        scidCrateHit[i] = spritesGetIdByNameF("crate_hit_%d", i + 1);
    scidDestroyAnim[VF_CRATE] = spritesGetIdByName("crate-destroy");

    scidDestroyAnim[VF_BRICK_RED] = spritesGetIdByNameF("brick-destroy-red");
    scidDestroyAnim[VF_BRICK_GREEN] = spritesGetIdByNameF("brick-destroy-green");
    scidDestroyAnim[VF_BRICK_BLUE] = spritesGetIdByNameF("brick-destroy-blue");
    scidDestroyAnim[VF_BRICK_YELLOW] = spritesGetIdByNameF("brick-destroy-yellow");

    for(i = 0; i < _BRICK_TYPE_COUNT; ++i)
    {
        scidBricks[VF_BRICK_RED][i] = spritesGetIdByNameF("brick%02d-red", i + 1);
        scidBricks[VF_BRICK_GREEN][i] = spritesGetIdByNameF("brick%02d-green", i + 1);
        scidBricks[VF_BRICK_BLUE][i] = spritesGetIdByNameF("brick%02d-blue", i + 1);
        scidBricks[VF_BRICK_YELLOW][i] = spritesGetIdByNameF("brick%02d-yellow", i + 1);
    }


    for(i = 0; i < _DESTROY_PARTICLE_TYPE_COUNT; ++i)
        scidDestroyParticle[i] = spritesGetIdByNameF("dparticle-%d", i + 1);


    /* generate map structure */

    mapHeight = height;

    map = (MapField**)commonAlloc2DTable(mapWidth, mapHeight, sizeof(MapField));

    FieldType **tmp;
    generatorAllocMap(&tmp, mapHeight, difficulty);

    int x, y;

    for(x = 0; x < mapWidth; ++x)
        for(y = 0; y < mapHeight; ++y)
        {
            map[x][y].type = tmp[x][y];
            map[x][y].shift = point(0, 0);
            map[x][y].state = FS_NORM;
            map[x][y].particle = NULL;
            map[x][y].justhit = false;
        }

    commonFree2DTable((void**)tmp, mapWidth);

    for(i = 0; i < mapWidth; ++i)
    {
        map[i][0].type = VF_NONE;
        map[i][1].type = VF_CRATE;

        int k;

        for(k = 0; k < VF_BRICK_COUNT; ++k)
            if(abs((mapWidth / 2) - i) <= k)
                map[i][k + 1].type = k;
            else
                map[i][k + 1].type = VF_CRATE;
    }


}

void gameMapCleanup()
{
    commonFree2DTable((void**)map, mapWidth);
}

void gameMapFrame(float lag)
{
    int x, y;
    float fallDelta = _SHAKE_SPEED * lag;

    needShapeUpdate = false;

    findFallingBricks();
    unsetShake();
    updateFallingUponShakingBricks();
    destroyFallen();


    for(y = mapHeight - 1; y >= 0; --y)
        for(x = 0; x < mapWidth; ++x)
        {

            MapField *f = &map[x][y];

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
                    particlesUnsetFlag(f->particle, PF_BLINKING);
                    setBrickDestroying(f, x, y);
                    f->particle = NULL;
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
                        map[x][y + 1] = map[x][y];
                        map[x][y + 1].shift.y = 0;

                        map[x][y].sprite = NULL;
                        map[x][y].type = VF_NONE;
                        needShapeUpdate = true;
                    }
                }
            }


        }

    for(y = 0; y < mapHeight; ++y)
    {
        float ypos = y * _BRICK_HEIGHT;

        for(x = 0; x < mapWidth; ++x)
        {
            float xpos = _BRICK_WIDTH * x;
            MapField *f = &map[x][y];

            if(f->sprite != NULL)
            {
                f->sprite->x = xpos + f->shift.x;
                f->sprite->y = ypos + f->shift.y;
            }
        }
    }

    if(needShapeUpdate)
        gameMapUpdateBrickShape();

}

FieldType gameMapGetFieldType(int x, int y)
{
    if(!INBOUND(x, y, mapWidth, mapHeight))
        return VF_VOID;

    return map[x][y].type;
}

