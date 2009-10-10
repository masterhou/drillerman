#include "player.h"

#include <math.h>

#include "input.h"
#include "snge.h"
#include "sprites.h"
#include "level.h"
#include "bcg.h"
#include "particles.h"
#include "hud.h"

static int depth;
static int points;
static float air;

static inline bool drillField(int x, int y);
static void addDrillParticles(int x, int y, Direction hitDirection);
static void advanceLevel(int hitx);
static bool checkSupport(int vx, int vy, Direction *leanOutDirection);
static void updatePlayerPosition(float lag);

static char directionName[DIR_COUNT][50] = {"left", "right", "up", "down"};
static int directionDelta[DIR_COUNT][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

static SpriteClassId scidPlayer;
static SpriteClassId scidPlayerStand[4];
static SpriteClassId scidPlayerDrill[4];
static SpriteClassId scidPlayerWalk[2];
static SpriteClassId scidPlayerFall;

static SpriteClassId scidDrillParticle;

static SpriteClassId scidLastDrill;

static Point viewportPos;

static int mapHeight;
static int mapWidth = _MAP_WIDTH;

static Direction playerDirection;

static Sprite *player;

static Direction slipDirection;
static bool slipping;
static bool goingUp;
static bool goingUpTrigger;
static float goingUpShift;
static Direction goingUpDirection;
static float goingUpTimer;
static bool levelAdvanceFalling;

static float drillDelayTimer;
static bool drillSched;

static Bcg bcg;
static Bcg bcgNext;

static int level;
static float hoff;
static float voff;
static int vx;
static int vy;

static SpriteClassId newAnimation;

static inline bool drillField(int x, int y)
{
    if(!INBOUND(x, y, mapWidth, mapHeight))
        return false;

    FieldType ft = level_GetFieldType(x, y);

    input_UnsetPressed(KEY_DRILL);

    if(ft < VF_BRICK_COUNT)
    {
        int size = level_DestroyBrick(x, y, false);
        points += size;
        level_AddPointsParticle(x, y, size);
        return true;
    }

    if(ft == VF_CRATE)
    {
        return level_HitCrate(x, y);
    }

    return false;
}

static void addDrillParticles(int x, int y, Direction drillDirection)
{
    int k;

    for(k = 0; k < _DRILL_PARTICLE_COUNT; ++k)
    {
        Sprite *sp = snge_AddSprite(scidDrillParticle, point(0, 0), _DRILL_PARTICLES_LAYER);
        sp->x = x;
        sp->y = y;
        Particle *p = particles_Add(sp);
        particles_SetVelocity(p,
                             -(float)directionDelta[drillDirection][0] * _DRILL_PARTICLE_FALL_SPEED * common_RandD() +
                             (float)directionDelta[drillDirection][1] * _DRILL_PARTICLE_FALL_SPEED * (common_RandD() - 0.5),
                             -(float)directionDelta[drillDirection][1] * _DRILL_PARTICLE_FALL_SPEED * common_RandD() +
                             (float)directionDelta[drillDirection][0] * _DRILL_PARTICLE_FALL_SPEED * (common_RandD() - 0.5), true);
        particles_SetFading(p, _DRILL_PARTICLE_FADE_SPEED, true);
    }
}

static void advanceLevel(int hitx)
{
    level_Advance(hitx, level + 1);
    input_UnsetPressed(KEY_DRILL);
    levelAdvanceFalling = true;
    voff = 0.0;
    newAnimation = scidPlayerFall;
}

static bool checkSupport(int x, int y, Direction *leanOutDirection)
{
    bool hasSupport = level_IsSolid(x, y + 1);

    *leanOutDirection = DIR_NONE;

    /*
       Check if the player is leaning out of the
       main brick he is standing on causing him to
       gain additional support.
    */

    if(((float)_BRICK_WIDTH - hoff) < (float)_PLAYER_WIDTH2)
        if(level_IsSolid(x + 1, y + 1))
        {
            hasSupport = true;
            *leanOutDirection = DIR_RIGHT;
        }

    if(hoff < (float)_PLAYER_WIDTH2)
        if(level_IsSolid(x - 1, y + 1))
        {
            hasSupport = true;
            *leanOutDirection = DIR_LEFT;
        }

    return hasSupport;
}

static void updatePlayerPosition(float lag)
{
    if(levelAdvanceFalling)
    {
        float interHeight = (float)(_BRICK_HEIGHT * (_INTER_ROW_COUNT));

        voff += _PLAYER_FALL_SPEED * lag;

        if(voff >= interHeight)
        {
            voff = 0.0;
            levelAdvanceFalling = false;
            vy = 0;
            level++;

            bcg_Cleanup(&bcg);
            bcg = bcgNext;
            bcg_Relativize(&bcg);

            if((level + 1) < _LEVEL_COUNT)
                bcgNext = bcg_Create(level_GetOffset(level + 1) - (float)_MAP_OFFSET_Y, level + 1);
        }

        return;
    }

    if(player->sclass != scidLastDrill || player->aended)
        newAnimation = scidPlayerStand[playerDirection];

    bool hasSupport;
    Direction leanOutDirection;
    int i;

    goingUpTimer += lag;

    for(i = 0; i < DIR_COUNT; ++i)
        if(input_IsDirPressed(i))
            playerDirection = i;

    /* check if player has support */

    hasSupport = checkSupport(vx, vy, &leanOutDirection);

    if(!hasSupport)
    {
        voff += (float)_PLAYER_FALL_SPEED * lag;

        if(voff >= (float)_BRICK_HEIGHT)
        {
            vy++;
            voff -= (float)_BRICK_HEIGHT;

            if(checkSupport(vx, vy + 1, &leanOutDirection))
                /* if gained support reset voff */
                voff = 0.0;
        }

        newAnimation = scidPlayerFall;

        /* cancel slipping move if player lost his support */
        slipping = false;

        return;
    }

    /* process player slipping move */

    if(slipping)
    {
        /* What if player suddenly gains support or his way
           is blocked by a falling brick? Simple - it can't
           happen because that would mean he was smashed. */

        hoff += lag * _PLAYER_SLIP_SPEED * (float)directionDelta[slipDirection][0];

        if(hoff >= (float)_BRICK_WIDTH)
        {
            hoff -= (float)_BRICK_WIDTH;
            vx++;
        }

        if(hoff < 0.0)
        {
            hoff += (float)_BRICK_WIDTH;
            vx--;
        }

        return;
    }

    /* climbing bricks */

    if(goingUpTrigger && goingUpTimer >= _DELAY_BEFORE_CLIMB)
    {
        goingUpTrigger = false;
        goingUp = true;
    }

    if(!input_IsDirPressed(goingUpDirection))
        goingUp = false;

    if(goingUp)
    {
        int cvx = vx + directionDelta[goingUpDirection][0];

        if(level_IsSolid(cvx, vy - 1) || level_IsSolid(vx, vy - 1) || !level_IsSolid(cvx, vy))
            goingUp = false;
    }

    if(goingUp)
    {
        voff -= _PLAYER_FALL_SPEED * lag;

        if(voff <= -(float)_BRICK_HEIGHT)
        {
            goingUp = false;

            voff = 0.0;
            vy--;

            hoff += (float)directionDelta[goingUpDirection][0] * _PLAYER_SPEED * lag;
        }
    }
    else if(voff < 0.0)
    {
        voff += _PLAYER_FALL_SPEED * lag;

        if(voff > 0.0)
            voff = 0.0;
    }

    /* check keystates and update position/animation accordingly */


    if(input_IsKeyPressed(KEY_DRILL))
    {
        newAnimation = scidPlayerDrill[playerDirection];
        scidLastDrill = newAnimation;

        if(!drillSched)
        {
            drillSched = true;
            drillDelayTimer = 0.0;
            return;
        }
    }

    if(drillSched)
        drillDelayTimer += lag;

    if(drillSched && drillDelayTimer >= _PLAYER_DRILL_DELAY)
    {
        drillSched = false;

        if(vy == (mapHeight - 1) && voff == 0.0 && playerDirection == DIR_DOWN)
        {
            advanceLevel(vx);
            return;
        }

        /* exact point where the brick was hit */
        float hx, hy;
        bool destroyed = false;

        if(voff < _HIT_DISTANCE_THRESHOLD)
        {
            if(playerDirection > DIR_RIGHT)
            {
                destroyed = drillField(vx, vy + directionDelta[playerDirection][1]);

                hx = (float)(vx * _BRICK_WIDTH) + hoff;
                hy = (float)(vy * _BRICK_HEIGHT);

                if(playerDirection == DIR_DOWN)
                    hy += (float)_BRICK_HEIGHT;
            }

            if(playerDirection == DIR_LEFT && hoff < (float)(_HIT_DISTANCE_THRESHOLD + _PLAYER_WIDTH2))
            {
                destroyed = drillField(vx - 1, vy);

                hx = (float)(vx * _BRICK_WIDTH);
                hy = (float)(vy * _BRICK_HEIGHT + _BRICK_HEIGHT / 2);
            }

            if(playerDirection == DIR_RIGHT && ((float)(_BRICK_WIDTH - _PLAYER_WIDTH2) - hoff) < (float)(_HIT_DISTANCE_THRESHOLD))
            {
                destroyed = drillField(vx + 1, vy);

                hx = (float)((vx + 1) * _BRICK_WIDTH);
                hy = (float)(vy * _BRICK_HEIGHT + _BRICK_HEIGHT / 2);
            }

            if(destroyed)
            {
                addDrillParticles(hx, hy, playerDirection);

                /*
                    Cause player to slip off the brick he is standing off
                    if he is leaning out towards newly created empty space.
                    This will prevent those moments of confusion when
                    player destroys a brick beneath and doesn't fall because
                    he has additional support.
                */
                if(playerDirection == DIR_DOWN && leanOutDirection != DIR_NONE)
                {
                    slipping = true;

                    if(leanOutDirection == DIR_LEFT)
                        slipDirection = DIR_RIGHT;
                    else
                        slipDirection = DIR_LEFT;
                }
            }

        }

        return;
    }

    bool goUp = false;

    if(playerDirection == DIR_LEFT && input_IsDirPressed(DIR_LEFT))
    {
        hoff -= lag * _PLAYER_SPEED;

        if(hoff < (float)_PLAYER_WIDTH2 && level_IsSolid(vx - 1, vy))
        {
            hoff = (float)_PLAYER_WIDTH2;

            if(!level_IsSolid(vx - 1, vy - 1))
                goUp = true;
        }
        else
            newAnimation = scidPlayerWalk[DIR_LEFT];
    }

    if(playerDirection == DIR_RIGHT && input_IsDirPressed(DIR_RIGHT))
    {
        hoff += lag * _PLAYER_SPEED;

        if(hoff > (float)(_BRICK_WIDTH - _PLAYER_WIDTH2) && level_IsSolid(vx + 1, vy))
        {
            hoff = (float)(_BRICK_WIDTH - _PLAYER_WIDTH2);

            if(!level_IsSolid(vx + 1, vy - 1))
                goUp = true;
        }
        else
            newAnimation = scidPlayerWalk[DIR_RIGHT];
    }

    if(hoff < 0.0)
    {
        hoff += (float)_BRICK_WIDTH;
        vx--;
    }

    if(hoff >= (float)_BRICK_WIDTH)
    {
        hoff -= (float)_BRICK_WIDTH;
        vx++;
    }

    if(goUp && !level_IsSolid(vx, vy - 1) && !goingUp && !goingUpTrigger)
    {
        goingUp = false;
        goingUpTrigger = true;
        goingUpShift = 0.0;
        goingUpDirection = playerDirection;
        goingUpTimer = 0.0;
    }
}

void player_Init(int levelHeight)
{
    points = 0;
    depth = 0;
    air = 100.0;

    mapHeight = levelHeight;
    level = 0;
    levelAdvanceFalling = false;

    level_Init(mapHeight);

    bcg = bcg_Create(0, 0);
    bcg_Relativize(&bcg);
    bcgNext = bcg_Create(level_GetOffset(1) - (float)_MAP_OFFSET_Y, 1);

    int i;

    scidPlayer = sprites_GetIdByName("level_common:dman-stand");
    scidPlayerFall = sprites_GetIdByName("level_common:dman-fall");

    for(i = 0; i < 4; ++i)
    {
        scidPlayerDrill[i] = sprites_GetIdByNameF("level_common:dman-drill%s", directionName[i]);
        scidPlayerStand[i] = sprites_GetIdByNameF("level_common:dman-stand%s", directionName[i]);
    }

    scidPlayerWalk[0] = sprites_GetIdByName("level_common:dman-walkleft");
    scidPlayerWalk[1] = sprites_GetIdByName("level_common:dman-walkright");

    scidDrillParticle = sprites_GetIdByName("level_common:particle-1");

    playerDirection = 3;
    player = snge_AddSprite(scidPlayerStand[playerDirection], point(0, 0), _PLAYER_LAYER);
    newAnimation = player->sclass;
    player->relative = true;
    player->y = - _PLAYER_SPRITE_PADDING_Y - (_PLAYER_HEIGHT - _BRICK_HEIGHT) + _MAP_OFFSET_Y;

    vx = _MAP_WIDTH / 2;
    vy = 0;
    voff = 0.0;
    hoff = (float)_BRICK_WIDTH / 2.0;
    viewportPos.y = - _MAP_OFFSET_Y;

    goingUp = false;
    goingUpShift = false;
    goingUpTrigger = false;

    scidLastDrill = scidPlayerDrill[DIR_UP];
    drillDelayTimer = 0.0;
    drillSched = false;
}

static void collectItems()
{
    Sprite *itemSprite;

    if(level_IsAirGetAir(vx, vy, &itemSprite))
    {
        points += _POINTS_FOR_AIR;
        air += _AIR_RESTORE;

        level_AddPointsParticle(vx, vy, _POINTS_FOR_AIR);
        snge_RelativizeSprite(itemSprite);
        Particle *p = particles_Add(itemSprite);
        particles_SetDestination(p, point(_AIR_DEST_X, _AIR_DEST_Y), _AIR_FLY_SPEED, true, false, true);
        particles_SetTrail(p, _AIR_TRAIL_SPACING, p->fadeSpeed);
        p->fadeSpeed /= 3.0;
        p->rotateSpeed = _AIR_ROT_SPEED;
        itemSprite->layer++;

    }
}

void player_Frame(float lag)
{
    updatePlayerPosition(lag);
    collectItems();

    if(player->sclass != newAnimation)
        snge_SwitchAnim(player, newAnimation);

    float oldy = viewportPos.y;

    float gY = (float)((_BRICK_HEIGHT * (mapHeight + _INTER_ROW_COUNT - 1) * level) + (_BRICK_HEIGHT * vy)) + voff;
    float gX = (float)(_BRICK_WIDTH * vx) + hoff;

    viewportPos.x = - _MAP_OFFSET_X;
    viewportPos.y = - _MAP_OFFSET_Y + gY;

    bcg_Move(&bcg, oldy - viewportPos.y);

    snge_MoveViewport(viewportPos);

    player->x = gX - _PLAYER_WIDTH2 - _PLAYER_SPRITE_PADDING_X + _MAP_OFFSET_X;

    air -= _AIR_DECREASE_SPEED * lag;
    if(air < 0.0)
        air = 0.0;
    depth = (mapHeight * level) + vy;
    hud_SetDepth(depth);
    hud_SetPoints(points);
    hud_SetAir(air);
}

void player_Cleanup()
{
    bcg_Cleanup(&bcg);
    level_Cleanup();
}
