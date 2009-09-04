#include "particles.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "snge.h"
#include "defaults.h"
#include "graphics.h"

static Particle **particles;
static int count;
static int capacity;
static const int overhead = _PARTICLE_ENGINE_CAPACITY_OVERHEAD;

void particlesInit()
{
    count = 0;
    capacity = overhead;
    particles = malloc(sizeof(Particle*) * capacity);
}

void particlesCleanup()
{
    int i;
    for(i = 0; i < count; ++i)
        free(particles[i]);

    free(particles);
}

void particlesRemoveAll()
{
    count = 0;
}

static inline void pushParticle(Particle *particle)
{
    if(count == capacity)
    {
        capacity += overhead;
        particles = realloc(particles, sizeof(Particle*) * capacity);
    }

    particles[count] = particle;
    count++;
}

static inline Particle *allocParticle()
{
    Particle *p = malloc(sizeof(Particle));
    memset(p, 0, sizeof(Particle));
    return p;
}

Particle *particlesAdd(Sprite *sprite)
{
    Particle *p = allocParticle();

    p->sprite = sprite;
    p->oldPos.x = sprite->x;
    p->oldPos.y = sprite->y;

    pushParticle(p);

    return p;
}

void particlesSetFading(Particle *particle, float fadeSpeed, bool destroyOnFadeOut)
{
    particle->fadeSpeed = fadeSpeed;

    if(destroyOnFadeOut)
        particle->flags |= PF_DESTROY_ON_FADEOUT;
}

void particlesSetVelocity(Particle *particle, float vx, float vy, bool affectedGravity)
{
    particle->vx = vx;
    particle->vy = vy;

    if(affectedGravity)
        particle->flags |= PF_GRAVITY_AFFECTED;
}

inline void particlesSetVelocityDegrees(Particle *particle, float angleDeg, float speed, bool affectedGravity)
{
    float rad = DEG_TO_RAD(angleDeg);
    particlesSetVelocity(particle, cosf(rad) * speed, sinf(rad) * speed, affectedGravity);
}

void particlesSetDestroyDistance(Particle *particle, float destroyDistance)
{
    particle->flags |= PF_DESTROY_ON_DISTANCE;
    particle->distanceLimit = destroyDistance;
}

void particlesSetTrail(Particle *particle, float trailSpacing, float trailFadeSpeed)
{
    particle->flags |= PF_LEAVE_TRAIL;
    particle->oldTrailDistance = particle->distance;
    particle->trailSpacing = trailSpacing;
    particle->trailFadeSpeed = trailFadeSpeed;
}

void particlesSetTimeout(Particle *particle, float timeout)
{
    particle->flags |= PF_DESTROY_ON_TIMEOUT;
    particle->timer = 0;
}

void particleSetDestination(Particle *particle, Point destination, float speed)
{
    particle->distance = 0.0;

    float dx = particle->sprite->x - destination.x;
    float dy = particle->sprite->y - destination.y;

    particle->distanceLimit = sqrtf(SQR(dx) + SQR(dy));

    particle->vx = (dx / particle->distanceLimit) * speed;
    particle->vy = (dy / particle->distanceLimit) * speed;

    particle->flags |= PF_DESTROY_ON_DISTANCE;
}

Particle *particlesClone(Particle *particle)
{
    Point p;

    Sprite *ns = sngeAddSprite(0, p, 0);

    SpriteId sid = ns->sid;
    memcpy(ns, particle->sprite, sizeof(Sprite));
    ns->sid = sid;

    Particle *np = malloc(sizeof(Particle));
    memcpy(np, particle, sizeof(Particle));

    np->sprite = ns;

    return np;
}

void particlesFrame(float lag)
{
    int i;

    int newcount = 0;
    Particle **ptmp = malloc(sizeof(Particle*) * capacity);

    for(i = 0; i < count; ++i)
    {
        Particle *p = particles[i];

        if(p->flags & PF_GRAVITY_AFFECTED)
        {
            p->vy += _PARTICLE_ENGINE_GRAVITY * lag;
        }

        if(p->vx != 0.0 || p->vy != 0.0)
        {
            float dx = p->vx * lag;
            float dy = p->vy * lag;

            p->sprite->x += dx;
            p->sprite->y += dy;


            if(p->flags & PF_DESTROY_ON_DISTANCE || p->flags & PF_LEAVE_TRAIL)
            {
                dx = p->sprite->x - p->oldPos.x;
                dy = p->sprite->x - p->oldPos.y;

                p->oldPos.x = p->sprite->x;
                p->oldPos.y = p->sprite->y;

                p->distance += sqrtf(SQR(dx) + SQR(dy));
            }

            if(p->flags & PF_DESTROY_ON_DISTANCE)
            {
                if(p->distance >= p->distanceLimit)
                    p->destroyScheduled = true;
            }
        }

        if(p->flags & PF_DESTROY_ON_TIMEOUT)
        {
            p->timer += lag;
            if(p->timer >= p->timeout)
                p->destroyScheduled = true;
        }

        if(p->flags & PF_DESTROY_ON_FADEOUT)
        {
            p->sprite->opacity -= p->fadeSpeed * lag;
            if(p->sprite->opacity <= 0.0)
            {
                p->sprite->opacity = 0.0;
                p->destroyScheduled = true;
            }
        }

        if(p->destroyScheduled)
        {
            sngeRemSprite(p->sprite->sid);
            free(p);
        }
        else
        {
            ptmp[newcount] = p;
            newcount++;
        }
    }

    for(i = 0; i < count; ++i)
    {
        Particle *p = particles[i];

        if(p->flags & PF_LEAVE_TRAIL && !p->destroyScheduled)
        {
            float dd = p->distance - p->oldTrailDistance;

            if(dd >= p->trailSpacing)
            {
                Particle *np = particlesClone(p);

                p->oldTrailDistance = p->distance - dd + p->trailSpacing;

                np->vx = 0;
                np->vy = 0;
                np->sprite->animdir = 0;
                np->fadeSpeed = p->trailFadeSpeed;
                np->flags &= ~PF_LEAVE_TRAIL;
                np->flags &= ~PF_GRAVITY_AFFECTED;
                np->flags |= PF_DESTROY_ON_FADEOUT;

                if(newcount == capacity)
                {
                    capacity += overhead ;
                    ptmp = realloc(ptmp, sizeof(Particle*) * capacity);
                }

                ptmp[newcount] = np;
                newcount++;
            }
        }

    }

    free(particles);

    count = newcount;
    particles = ptmp;
}
