#ifndef PARTICLES_H
#define PARTICLES_H

#include "common.h"
#include "snge.h"

#define PF_DESTROY_ON_DISTANCE  2
#define PF_DESTROY_ON_FADEOUT   4
#define PF_DESTROY_ON_TIMEOUT   8
#define PF_GRAVITY_AFFECTED     16
#define PF_LEAVE_TRAIL          32


typedef struct
{
    Sprite *sprite;

    unsigned char flags;
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
void particlesInit();
void particlesCleanup();

Particle *particlesAdd(Sprite *sprite);

void particlesSetFading(Particle *particle, float fadeSpeed, bool destroyOnFadeOut);
void particlesSetVelocity(Particle *particle, float vx, float vy, bool affectedGravity);
inline void particlesSetVelocityDegrees(Particle *particle, float angleDeg, float speed, bool affectedGravity);
void particlesSetDestroyDistance(Particle *particle, float destroyDistance);
void particlesSetTrail(Particle *particle, float trailSpacing, float trailFadeSpeed);
Particle *particlesClone(Particle *particle);
void particlesSetTimeout(Particle *particle, float timeout);
void particlesFrame(float lag);

#endif
