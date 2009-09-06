#include "gamemech.h"

#include <math.h>

#include "input.h"
#include "snge.h"
#include "sprites.h"
#include "gamemap.h"
#include "timer.h"
#include "gamebcg.h"
#include "particles.h"

typedef enum {DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_COUNT, DIR_NONE} Direction;

static char directionName[DIR_COUNT][50] = {"left", "right", "up", "down"};
static int directionKey[DIR_COUNT] = {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN};
static int directionDelta[DIR_COUNT][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
static int drillKey = SDLK_SPACE;

static SpriteClassId scidPlayer;
static SpriteClassId scidPlayerWait;
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

static void physToVirtPos(float px, float py, int *vx, int *vy)
{
    *vx = px / (float)_BRICK_WIDTH;
    *vy = py / (float)_BRICK_HEIGHT;
}

static void virtToPhysPos(int vx, int vy, float *px, float *py)
{
    *px = (float)vx * (float)_BRICK_WIDTH;
    *py = (float)vy * (float)_BRICK_HEIGHT;
}

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

    FieldType ft = gameMapGetFieldType(x, y);

    inputKeyState[drillKey] = 0;

    if(ft < VF_BRICK_COUNT)
    {
        gameMapDestroyBrick(x, y);
        return true;
    }

    return false;
}

static void addHitParticles(int x, int y, Direction hitDirection)
{
    int k;

    for(k = 0; k < _HIT_PARTICLE_COUNT; ++k)
    {
        Sprite *sp = sngeAddSprite(scidParticle, point(0, 0), 100);
        sp->x = x;
        sp->y = y;
        Particle *p = particlesAdd(sp);
        particlesSetVelocity(p,
                             -(float)directionDelta[hitDirection][0] * _HIT_PARTICLE_FALL_SPEED * commonRandD() +
                             (float)directionDelta[hitDirection][1] * _HIT_PARTICLE_FALL_SPEED * (commonRandD() - 0.5),
                             -(float)directionDelta[hitDirection][1] * _HIT_PARTICLE_FALL_SPEED * commonRandD() +
                             (float)directionDelta[hitDirection][0] * _HIT_PARTICLE_FALL_SPEED * (commonRandD() - 0.5), true);

        particlesSetFading(p, _HIT_PARTICLE_FADE_SPEED, true);
    }
}

static void updatePlayerPosition(float lag)
{
    int vx, vy;
    bool animationSet = false;
    bool additionalSupport = false;
    int i;

    physToVirtPos(playerPos.x, playerPos.y, &vx, &vy);

    for(i = 0; i < DIR_COUNT; ++i)
        if(inputKeyState[directionKey[i]])
            playerDirection = i;

    /* climbing bricks */

    if(goingUpTrigger && timerFired(goingUpTimer))
    {
        goingUpTrigger = false;
        goingUp = true;
    }

    if(!inputKeyState[directionKey[goingUpDirection]])
        goingUp = false;

    if(goingUp)
    {
        int cvx = vx + directionDelta[goingUpDirection][0];

        if(gameMapIsSolid(cvx, vy - 1) || gameMapIsSolid(vx, vy - 1) || !gameMapIsSolid(cvx, vy))
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

    bool has_support = gameMapIsSolid(vx, vy + 1);

    if(((float)_BRICK_WIDTH - hoff) < (float)_PLAYER_WIDTH2)
    {
        has_support = has_support ||  gameMapIsSolid(vx + 1, vy + 1);
        additionalSupport = true;
    }

    if(hoff < (float)_PLAYER_WIDTH2)
    {
        has_support = has_support ||  gameMapIsSolid(vx - 1, vy + 1);
        additionalSupport = true;
    }


    if(!has_support)
    {
        playerPos.y += (float)_PLAYER_FALL_SPEED * lag;
        changePlayerAnimation(scidPlayerFall);
        return;
    }
    else if(voff > 0)
    {
        playerPos.y -= voff;
        voff = 0;
    }

    /* check keystates and update position/animation accordingly */

    if(inputKeyState[drillKey])
    {
        animationSet = true;
        changePlayerAnimation(scidPlayerDrill[playerDirection]);

        float hx, hy;
        bool hit = false;

        if(voff < _HIT_DISTANCE_THRESHOLD)
        {
            if(playerDirection > DIR_RIGHT)
            {
                int bx, by;

                physToVirtPos(playerPos.x, playerPos.y, &bx, &by);

                by += directionDelta[playerDirection][1];

                hit = hitField(bx, by);

                hit = true;
                hx = playerPos.x;
                hy = vy * _BRICK_HEIGHT;

                if(playerDirection == DIR_DOWN)
                    hy += _BRICK_HEIGHT;
            }

            if(playerDirection == DIR_LEFT && hoff < (_HIT_DISTANCE_THRESHOLD + _PLAYER_WIDTH2))
            {
                hit = hitField(vx - 1, vy);

                hx = vx * _BRICK_WIDTH;
                hy = vy * _BRICK_HEIGHT + _BRICK_HEIGHT / 2;
            }

            if(playerDirection == DIR_RIGHT && (_BRICK_WIDTH - _PLAYER_WIDTH2 - hoff) < _HIT_DISTANCE_THRESHOLD)
            {
                hit = hitField(vx + 1, vy);

                hx = (vx + 1) * _BRICK_WIDTH;
                hy = vy * _BRICK_HEIGHT + _BRICK_HEIGHT / 2;
            }

            if(hit)
            {
                addHitParticles(hx, hy, playerDirection);
            }

        }
    }
    else
        for(i = 0; i < DIR_UP; ++i)
            if(inputKeyState[directionKey[i]])
            {
                animationSet = true;
                changePlayerAnimation(scidPlayerWalk[i]);

                float dx = (float)directionDelta[i][0] * lag * _PLAYER_SPEED;

                int cvx = vx + directionDelta[i][0];
                float cpx = playerPos.x + dx;

                if(gameMapIsSolid(cvx, vy))
                    if(!NO_COLLISION(cpx - _PLAYER_WIDTH2, 0, _PLAYER_WIDTH, 1,  cvx * _BRICK_WIDTH, 0, _BRICK_WIDTH, 1))
                    {
                        if(!gameMapIsSolid(cvx, vy - 1) && !gameMapIsSolid(vx, vy - 1) && !goingUp && !goingUpTrigger)
                        {
                                goingUp = false;
                                goingUpTrigger = true;
                                goingUpShift = 0;
                                goingUpDirection = i;
                                goingUpTimer = timerAddTimer(_DELAY_BEFORE_CLIMB, 1);
                        }

                        animationSet = false;
                        break;
                    }

                playerPos.x += dx;
            }

    if(!animationSet)
        changePlayerAnimation(scidPlayerStand[playerDirection]);



}

void gameMechInit(int mapheight, Difficulty difficulty)
{
    mapHeight = mapheight;

    gameBcgInit();
    gameMapInit(mapHeight, difficulty);
    gameMapAllocSprites();

    int i;

    scidPlayer = spritesGetIdByName("dman-stand");
    scidPlayerFall = spritesGetIdByName("dman-fall");

    for(i = 0; i < 4; ++i)
    {
        scidPlayerDrill[i] = spritesGetIdByNameF("dman-drill%s", directionName[i]);
        scidPlayerStand[i] = spritesGetIdByNameF("dman-stand%s", directionName[i]);
    }

    scidPlayerWalk[0] = spritesGetIdByName("dman-walkleft");
    scidPlayerWalk[1] = spritesGetIdByName("dman-walkright");

    scidParticle = spritesGetIdByName("particle-1");

    playerDirection = 3;
    player = sngeAddSprite(scidPlayerStand[playerDirection], point(0, 0), 6);
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

}

void gameMechFrame(float lag)
{
    updatePlayerPosition(lag);
    collectItems();

    float oldy = viewportPos.y;

    viewportPos.x = - _MAP_OFFSET_X;
    viewportPos.y = - _MAP_OFFSET_Y + playerPos.y - goingUpShift;

    gameBcgMove(oldy - viewportPos.y);

    sngeMoveViewport(viewportPos);

    player->x = playerPos.x - _PLAYER_WIDTH2 - _PLAYER_SPRITE_PADDING_X + _MAP_OFFSET_X;
    player->y = - _PLAYER_SPRITE_PADDING_Y - (_PLAYER_HEIGHT - _BRICK_HEIGHT) + _MAP_OFFSET_Y;
}

void gameMechCleanup()
{
    gameMapCleanup();
    gameBcgCleanup();
}
