#include "player.h"

#include <math.h>

#include "input.h"
#include "snge.h"
#include "sprites.h"
#include "level.h"
#include "timer.h"
#include "bcg.h"
#include "particles.h"

static char directionName[DIR_COUNT][50] = {"left", "right", "up", "down"};
static int directionDelta[DIR_COUNT][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

static SpriteClassId scidPlayer;
static SpriteClassId scidPlayerStand[4];
static SpriteClassId scidPlayerDrill[4];
static SpriteClassId scidPlayerWalk[2];
static SpriteClassId scidPlayerFall;

static SpriteClassId scidParticle;

static Point viewportPos;

static int mapHeight;
static int mapWidth = _MAP_WIDTH;

static Point playerPos;
static Direction playerDirection;

static Sprite *player;

static Direction slipDirection;
static bool slipping;
static bool goingUp;
static bool goingUpTrigger;
static float goingUpShift;
static Direction goingUpDirection;
static TimerHandle goingUpTimer;

static Bcg bcg;
static Bcg bcgNext;

static int level;

static inline void changePlayerAnimation(SpriteClassId scid)
{
    if(player->sclass == scid)
        return;

    player->sclass = scid;
    player->frame = 0;
}

static inline bool hitField(int x, int y)
{
    if(!INBOUND(x, y, mapWidth, mapHeight))
        return false;

    FieldType ft = level_GetFieldType(x, y);

    input_UnsetPressed(KEY_DRILL);

    if(ft < VF_BRICK_COUNT)
    {
        level_DestroyBrick(x, y, false);
        return true;
    }

    if(ft == VF_CRATE)
    {
        return level_HitCrate(x, y);
    }

    return false;
}

static void addHitParticles(int x, int y, Direction hitDirection)
{
    int k;

    for(k = 0; k < _HIT_PARTICLE_COUNT; ++k)
    {
        Sprite *sp = snge_AddSprite(scidParticle, point(0, 0), 100);
        sp->x = x;
        sp->y = y;
        Particle *p = particles_Add(sp);
        particles_SetVelocity(p,
                             -(float)directionDelta[hitDirection][0] * _HIT_PARTICLE_FALL_SPEED * common_RandD() +
                             (float)directionDelta[hitDirection][1] * _HIT_PARTICLE_FALL_SPEED * (common_RandD() - 0.5),
                             -(float)directionDelta[hitDirection][1] * _HIT_PARTICLE_FALL_SPEED * common_RandD() +
                             (float)directionDelta[hitDirection][0] * _HIT_PARTICLE_FALL_SPEED * (common_RandD() - 0.5), true);
        particles_SetFading(p, _HIT_PARTICLE_FADE_SPEED, true);
    }
}

static void advanceLevel(int hitx)
{
    level++;
    level_Advance(hitx, level);
}

static void updatePlayerPosition(float lag)
{
    bool animationSet = false;
    bool additionalSupport = false;
    Direction leanOutDirection = DIR_NONE;
    int i;

    int vx = playerPos.x / (float)_BRICK_WIDTH;
    int vy = playerPos.y / (float)_BRICK_HEIGHT;

    for(i = 0; i < DIR_COUNT; ++i)
        if(input_IsDirPressed(i))
            playerDirection = i;

    /* climbing bricks */

    if(goingUpTrigger && timer_Fired(goingUpTimer))
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
        {
            goingUp = false;
        }
    }

    if(goingUp)
    {
        goingUpShift += _PLAYER_FALL_SPEED * lag;

        if(goingUpShift >= _BRICK_HEIGHT)
        {
            goingUp = false;
            goingUpShift = 0;

            playerPos.y -= _BRICK_HEIGHT;

            vy--;
            vx += directionDelta[goingUpDirection][0];

            playerPos.x += (float)directionDelta[goingUpDirection][0] * _PLAYER_SPEED * lag;
        }
    }
    else if(goingUpShift != 0)
    {
        goingUpShift -= _PLAYER_FALL_SPEED * lag;

        if(goingUpShift < 0)
            goingUpShift = 0;
    }

    /* get player horiz offset relative to current column */

    float hoff = fmodf(playerPos.x, (float)_BRICK_WIDTH);
    float voff = fmodf(playerPos.y, (float)_BRICK_HEIGHT);

    /* check if player has support */

    bool hasSupport = level_IsSolid(vx, vy + 1);

    /*
       Check if the player is leaning out of the
       main brick he is standing on causing him to
       gain additional support.
    */
    if(((float)_BRICK_WIDTH - hoff) < (float)_PLAYER_WIDTH2)
    {
        additionalSupport = level_IsSolid(vx + 1, vy + 1);
        hasSupport = hasSupport ||  additionalSupport;
        leanOutDirection = DIR_RIGHT;
    }

    if(hoff < (float)_PLAYER_WIDTH2)
    {
        additionalSupport = level_IsSolid(vx - 1, vy + 1);
        hasSupport = hasSupport ||  additionalSupport;
        leanOutDirection = DIR_LEFT;
    }

    if(!hasSupport)
    {
        playerPos.y += (float)_PLAYER_FALL_SPEED * lag;
        changePlayerAnimation(scidPlayerFall);

        /* cancel slipping move if player lost his support */
        slipping = false;
        return;
    }
    else if(voff > 0)
    {
        playerPos.y -= voff;
        voff = 0;
    }

    /* process player slipping move */

    if(slipping)
    {
        /* What if player suddenly gains support or his way
           is blocked by a falling brick? Simple - it can't
           happen because that would mean he was smashed. */

        playerPos.x += lag * _PLAYER_SLIP_SPEED * directionDelta[slipDirection][0];
    }


    /* check keystates and update position/animation accordingly */

    if(input_IsKeyPressed(KEY_DRILL))
    {
        if(vy == (mapHeight - 1) && voff == 0.0 && playerDirection == DIR_DOWN)
            advanceLevel(vx);

        animationSet = true;
        changePlayerAnimation(scidPlayerDrill[playerDirection]);

        /* exact point where the brick was hit */
        float hx, hy;
        bool destroyed = false;

        if(voff < _HIT_DISTANCE_THRESHOLD)
        {
            if(playerDirection > DIR_RIGHT)
            {
                destroyed = hitField(vx, vy + directionDelta[playerDirection][1]);

                hx = playerPos.x;
                hy = vy * _BRICK_HEIGHT;

                if(playerDirection == DIR_DOWN)
                    hy += _BRICK_HEIGHT;
            }

            if(playerDirection == DIR_LEFT && hoff < (_HIT_DISTANCE_THRESHOLD + _PLAYER_WIDTH2))
            {
                destroyed = hitField(vx - 1, vy);

                hx = vx * _BRICK_WIDTH;
                hy = vy * _BRICK_HEIGHT + _BRICK_HEIGHT / 2;
            }

            if(playerDirection == DIR_RIGHT && (_BRICK_WIDTH - _PLAYER_WIDTH2 - hoff) < _HIT_DISTANCE_THRESHOLD)
            {
                destroyed = hitField(vx + 1, vy);

                hx = (vx + 1) * _BRICK_WIDTH;
                hy = vy * _BRICK_HEIGHT + _BRICK_HEIGHT / 2;
            }

            if(destroyed)
            {
                addHitParticles(hx, hy, playerDirection);

                /*
                    Cause player to slip off the brick he is standing off
                    if he is leaning out towards newly created empty space.
                    This will prevent those moments of confusion when
                    player destroys a brick beneath and doesn't fall because
                    he has additional support.
                */
                if(playerDirection == DIR_DOWN && additionalSupport)
                {
                    slipping = true;

                    if(leanOutDirection == DIR_LEFT)
                        slipDirection = DIR_RIGHT;
                    else
                        slipDirection = DIR_LEFT;
                }
            }

        }
    }
    else
        for(i = 0; i < DIR_UP; ++i)
            if(input_IsDirPressed(i) && !slipping)
            {
                animationSet = true;
                changePlayerAnimation(scidPlayerWalk[i]);

                float dx = (float)directionDelta[i][0] * lag * _PLAYER_SPEED;

                int cvx = vx + directionDelta[i][0];
                float cpx = playerPos.x + dx;

                if(level_IsSolid(cvx, vy))
                    if(!NO_COLLISION(cpx - _PLAYER_WIDTH2, 0, _PLAYER_WIDTH, 1,  cvx * _BRICK_WIDTH, 0, _BRICK_WIDTH, 1))
                    {
                        if(!level_IsSolid(cvx, vy - 1) && !level_IsSolid(vx, vy - 1) && !goingUp && !goingUpTrigger)
                        {
                                goingUp = false;
                                goingUpTrigger = true;
                                goingUpShift = 0;
                                goingUpDirection = i;
                                goingUpTimer = timer_AddTimer(_DELAY_BEFORE_CLIMB, 1);
                        }

                        animationSet = false;
                        break;
                    }

                playerPos.x += dx;
            }

    if(!animationSet)
        changePlayerAnimation(scidPlayerStand[playerDirection]);



}

void player_Init(int levelHeight)
{
    mapHeight = levelHeight;
    level = 0;

    bcg = bcg_Create(0, 0);
    level_Init(mapHeight);

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

    scidParticle = sprites_GetIdByName("level_common:particle-1");

    playerDirection = 3;
    player = snge_AddSprite(scidPlayerStand[playerDirection], point(0, 0), 6);
    player->relative = true;
    playerPos = point((_MAP_WIDTH / 2) * _BRICK_WIDTH + (_BRICK_WIDTH / 2), 0);
    viewportPos.y = - _MAP_OFFSET_Y + playerPos.y - goingUpShift;

    goingUp = false;
    goingUpShift = false;
    goingUpTrigger = false;
}

static void collectItems()
{
    Sprite *itemSprite;

    int vx = playerPos.x / (float)_BRICK_WIDTH;
    int vy = playerPos.y / (float)_BRICK_HEIGHT;

    if(level_IsAirGetAir(vx, vy, &itemSprite))
    {
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

    float oldy = viewportPos.y;

    viewportPos.x = - _MAP_OFFSET_X;
    viewportPos.y = - _MAP_OFFSET_Y + playerPos.y - goingUpShift;

    bcg_Move(&bcg, oldy - viewportPos.y);

    snge_MoveViewport(viewportPos);

    player->x = playerPos.x - _PLAYER_WIDTH2 - _PLAYER_SPRITE_PADDING_X + _MAP_OFFSET_X;
    player->y = - _PLAYER_SPRITE_PADDING_Y - (_PLAYER_HEIGHT - _BRICK_HEIGHT) + _MAP_OFFSET_Y;
}

void player_Cleanup()
{
    bcg_Cleanup(&bcg);
    level_Cleanup();
}
