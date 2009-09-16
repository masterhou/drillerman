#ifndef PARTICLES_H
#define PARTICLES_H

#include <inttypes.h>
#include "common.h"
#include "snge.h"

#define PF_DESTROY_ON_DISTANCE  2
#define PF_DESTROY_ON_FADEOUT   4
#define PF_DESTROY_ON_TIMEOUT   8
#define PF_GRAVITY_AFFECTED     16
#define PF_LEAVE_TRAIL          32
#define PF_STOP_ON_DISTANCE     64
#define PF_BLINKING             128
#define PF_DESTROY_ON_ANIM_END  256

#define particles_SetFlag(__pparticle, __flag) __pparticle->flags |= __flag
#define particles_UnsetFlag(__pparticle, __flag) __pparticle->flags &= ~__flag

typedef struct
{
    Sprite *sprite;

    uint16_t flags;
    float distance;
    float fadeSpeed;
    float trailFadeSpeed;
    float rotateSpeed;
    float distanceLimit;
    float trailSpacing;
    float oldTrailDistance;
    float timer;
    float timeout;
    float vx;
    float vy;
    bool destroyScheduled;

    Point oldPos;

    Color color;
} Particle;

/*
    Cleaning up particle engine does not remove any sprites!
*/
void particles_Init();
void particles_Cleanup();

Particle *particles_Add(Sprite *sprite);

void particles_SetFading(Particle *particle, float fadeSpeed, bool destroyOnFadeOut);
void particles_SetVelocity(Particle *particle, float vx, float vy, bool affectedGravity);
inline void particles_SetVelocityDegrees(Particle *particle, float angleDeg, float speed, bool affectedGravity);
void particles_SetDestroyDistance(Particle *particle, float destroyDistance);
void particles_SetTrail(Particle *particle, float trailSpacing, float trailFadeSpeed);
Particle *particles_Clone(Particle *particle);
void particles_SetTimeout(Particle *particle, float timeout);
void particles_SetDestination(Particle *particle, Point destination, float speed, bool fadeOut, bool destroyOnArrival, bool stopOnArrival);
void particles_Frame(float lag);
void particles_SetBlinking(Particle *particle, float frequency);
void particles_DestroyOnAnimationEnd(Particle *particle);
void particles_SetVelocityFromNormalizedVector(Particle *particle, Point unnormVector, float speed);

#endif
