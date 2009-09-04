#include "gamemech.h"

#include <math.h>

#include "input.h"
#include "snge.h"
#include "sprites.h"
#include "gamemap.h"
#include "timer.h"
#include "gamebcg.h"
#include "particles.h"

static char directionName[4][50] = {"left", "right", "up", "down"};
static int directionKey[4] = {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN};
static int directionDelta[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
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
static int playerDirection;

static Sprite *player;

static bool goingUp;
static bool goingUpTrigger;
static float goingUpShift;
static int goingUpDirection;
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

static inline void hitField(int x, int y)
{
    if(!INBOUND(x, y, mapWidth, mapHeight))
        return;

    FieldType ft = gameMapGetFieldType(x, y);

    inputKeyState[drillKey] = 0;

    if(ft < VF_BRICK_COUNT)
        gameMapDestroyBrick(x, y);
}

static void updatePlayerPosition(float lag)
{
    int vx, vy;
    int animation_set = 0;
    int i;

    physToVirtPos(playerPos.x, playerPos.y, &vx, &vy);

    for(i = 0; i < 4; ++i)
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

    int has_support = gameMapIsSolid(vx, vy + 1);

    if(((float)_BRICK_WIDTH - hoff) < _PLAYER_WIDTH)
        has_support = has_support ||  gameMapIsSolid(vx + 1, vy + 1);

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
        animation_set = 1;
        changePlayerAnimation(scidPlayerDrill[playerDirection]);

        float hx, hy;
        bool hit = false;

        if(voff < _HIT_DISTANCE_THRESHOLD)
        {
            if(playerDirection > 1)
            {
                int bx, by;
                physToVirtPos(playerPos.x + (_PLAYER_WIDTH / 2), playerPos.y, &bx, &by);

                by += directionDelta[playerDirection][1];

                hitField(bx, by);

                hit = true;
                hx = playerPos.x + (_PLAYER_WIDTH / 2);
                hy = vy * _BRICK_HEIGHT;

                if(playerDirection == 3)
                    hy += _BRICK_HEIGHT;
            }

            if(playerDirection == 0 && hoff < _HIT_DISTANCE_THRESHOLD)
            {
                hitField(vx - 1, vy);

                hit = true;
                hx = vx * _BRICK_WIDTH;
                hy = vy * _BRICK_HEIGHT + (_BRICK_HEIGHT / 2);
            }

            if(playerDirection == 1 && (_BRICK_WIDTH - _PLAYER_WIDTH - hoff) < _HIT_DISTANCE_THRESHOLD)
            {
                hitField(vx + 1, vy);

                hit = true;
                hx = (vx + 1) * _BRICK_WIDTH;
                hy = vy * _BRICK_HEIGHT + (_BRICK_HEIGHT / 2);
            }

            if(hit)
            {
                int k;

                for(k = 0; k < _HIT_PARTICLE_COUNT; ++k)
                {
                    Sprite *sp = sngeAddSprite(scidParticle, point(0, 0), 100);
                    sp->x = hx;
                    sp->y = hy;
                    Particle *p = particlesAdd(sp);
                    particlesSetVelocity(p,
                                         -(float)directionDelta[playerDirection][0] * _HIT_PARTICLE_FALL_SPEED * commonRandD() +
                                         (float)directionDelta[playerDirection][1] * _HIT_PARTICLE_FALL_SPEED * (commonRandD() - 0.5),
                                         -(float)directionDelta[playerDirection][1] * _HIT_PARTICLE_FALL_SPEED * commonRandD() +
                                         (float)directionDelta[playerDirection][0] * _HIT_PARTICLE_FALL_SPEED * (commonRandD() - 0.5), true);

                    particlesSetFading(p, _HIT_PARTICLE_FADE_SPEED, true);
                }
            }
        }
    }
    else
        for(i = 0; i < 2; ++i)
            if(inputKeyState[directionKey[i]])
            {
                animation_set = 1;
                changePlayerAnimation(scidPlayerWalk[i]);

                float dx = (float)directionDelta[i][0] * lag * _PLAYER_SPEED;

                int cvx = vx + directionDelta[i][0];
                float cpx = playerPos.x + dx;

                if(gameMapIsSolid(cvx, vy))
                    if(!NO_COLLISION(cpx, 0, _PLAYER_WIDTH, 1,  cvx * _BRICK_WIDTH, 0, _BRICK_WIDTH, 1))
                    {
                        if(!gameMapIsSolid(cvx, vy - 1) && !gameMapIsSolid(vx, vy - 1) && !goingUp && !goingUpTrigger)
                        {
                                goingUp = false;
                                goingUpTrigger = true;
                                goingUpShift = 0;
                                goingUpDirection = i;
                                goingUpTimer = timerAddTimer(_DELAY_BEFORE_JUMP, 1);
                        }

                        animation_set = 0;
                        break;
                    }

                playerPos.x += dx;
            }

    if(!animation_set)
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
    playerPos = point((_MAP_WIDTH / 2) * _BRICK_WIDTH + ((_BRICK_WIDTH - _PLAYER_WIDTH) / 2), 0);
    viewportPos.y = - _MAP_OFFSET_Y + playerPos.y - goingUpShift;

    goingUp = false;
    goingUpShift = false;
    goingUpTrigger = false;
}

void gameMechFrame(float lag)
{
    updatePlayerPosition(lag);

    float oldy = viewportPos.y;

    viewportPos.x = - _MAP_OFFSET_X;
    viewportPos.y = - _MAP_OFFSET_Y + playerPos.y - goingUpShift;

    gameBcgMove(oldy - viewportPos.y);

    sngeMoveViewport(viewportPos);

    player->x = playerPos.x - _PLAYER_SPRITE_PADDING_X + _MAP_OFFSET_X;
    player->y = - _PLAYER_SPRITE_PADDING_Y - (_PLAYER_HEIGHT - _BRICK_HEIGHT) + _MAP_OFFSET_Y;
}

void gameMechCleanup()
{
    gameMapCleanup();
    gameBcgCleanup();
}
