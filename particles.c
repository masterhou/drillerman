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

static inline Particle *allocParticle();
static inline void pushParticle(Particle *particle);

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


void particles_Init()
{
    count = 0;
    capacity = overhead;
    particles = malloc(sizeof(Particle*) * capacity);
}

void particles_Cleanup()
{
    int i;
    for(i = 0; i < count; ++i)
        free(particles[i]);

    free(particles);
}

void particles_RemoveAll()
{
    count = 0;
}

Particle *particles_Add(Sprite *sprite)
{
    if(sprite == NULL)
        return NULL;

    Particle *p = allocParticle();

    p->sprite = sprite;
    p->oldPos.x = sprite->x;
    p->oldPos.y = sprite->y;

    pushParticle(p);

    return p;
}

void particles_SetFading(Particle *particle, float fadeSpeed, bool destroyOnFadeOut)
{
    particle->fadeSpeed = fadeSpeed;

    if(destroyOnFadeOut)
        particles_SetFlag(particle, PF_DESTROY_ON_FADEOUT);
}

void particles_SetVelocity(Particle *particle, float vx, float vy, bool affectedGravity)
{
    particle->vx = vx;
    particle->vy = vy;

    if(affectedGravity)
       particles_SetFlag(particle, PF_GRAVITY_AFFECTED);
}

inline void particles_SetVelocityDegrees(Particle *particle, float angleDeg, float speed, bool affectedGravity)
{
    float rad = DEG_TO_RAD(angleDeg);
    particles_SetVelocity(particle, cosf(rad) * speed, sinf(rad) * speed, affectedGravity);
}

void particles_SetDestroyDistance(Particle *particle, float destroyDistance)
{
    particles_SetFlag(particle,  PF_DESTROY_ON_DISTANCE);
    particle->distanceLimit = destroyDistance;
}

void particles_SetTrail(Particle *particle, float trailSpacing, float trailFadeSpeed)
{
    particles_SetFlag(particle,  PF_LEAVE_TRAIL);
    particle->oldTrailDistance = particle->distance;
    particle->trailSpacing = trailSpacing;
    particle->trailFadeSpeed = trailFadeSpeed;
}

void particles_SetTimeout(Particle *particle, float timeout)
{
    particles_SetFlag(particle, PF_DESTROY_ON_TIMEOUT);
    particle->timer = 0;
}

void particles_SetBlinking(Particle *particle, float frequency)
{
    particles_SetFlag(particle, PF_BLINKING);
    particle->fadeSpeed = frequency;
}

void particles_SetDestination(Particle *particle, Point destination, float speed, bool fadeOut, bool destroyOnArrival)
{
    particle->distance = 0.0;

    float dx = destination.x - particle->sprite->x;
    float dy = destination.y - particle->sprite->y;

    particle->distanceLimit = sqrtf(SQR(dx) + SQR(dy));

    particle->vx = (dx / particle->distanceLimit) * speed;
    particle->vy = (dy / particle->distanceLimit) * speed;

    if(destroyOnArrival)
        particles_SetFlag(particle, PF_DESTROY_ON_DISTANCE);
    else
        particles_SetFlag(particle, PF_STOP_ON_DISTANCE);
    
    if(fadeOut)
    {
        float fadeSpeed = particle->sprite->opacity / (particle->distanceLimit / speed);
        particles_SetFading(particle, fadeSpeed, true);
    }

}

inline void particles_DestroyOnAnimationEnd(Particle *particle)
{
    particles_SetFlag(particle, PF_DESTROY_ON_ANIM_END);
}

Particle *particles_Clone(Particle *particle)
{
    Point p;

    Sprite *ns = snge_AddSprite(0, p, 0);

    memcpy(ns, particle->sprite, sizeof(Sprite));

    Particle *np = malloc(sizeof(Particle));
    memcpy(np, particle, sizeof(Particle));

    np->sprite = ns;

    return np;
}

void particles_Frame(float lag)
{
    int i;

    int newcount = 0;
    Particle **ptmp = malloc(sizeof(Particle*) * capacity);

    for(i = 0; i < count; ++i)
    {
        Particle *p = particles[i];

        /* If something outside the particle
           engine wants it destroyed. We have
           to do it here because the sprite
           can be already freed. */
        if(p->destroyScheduled)
        {
            p->sprite->destroy = true;
            free(p);
            continue;
        }

        if(p->flags & PF_DESTROY_ON_ANIM_END && p->sprite->aended)
        {
            p->destroyScheduled = true;
        }

        if(p->rotateSpeed != 0.0)
            p->sprite->angle += lag * p->rotateSpeed;

        if(p->fadeSpeed != 0.0)
        {
            p->sprite->opacity -= p->fadeSpeed * lag;

            if(p->flags & PF_BLINKING)
            {
                if(p->sprite->opacity <= 0)
                {
                    p->sprite->opacity = -p->sprite->opacity;
                    p->fadeSpeed = -p->fadeSpeed;
                }

                if(p->sprite->opacity >= 1.0)
                {
                    p->sprite->opacity = 2.0 - p->sprite->opacity;
                    p->fadeSpeed = -p->fadeSpeed;
                }
            }
        }

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


            if(p->flags & PF_DESTROY_ON_DISTANCE || p->flags & PF_LEAVE_TRAIL || p->flags & PF_STOP_ON_DISTANCE)
            {
                dx = p->sprite->x - p->oldPos.x;
                dy = p->sprite->y - p->oldPos.y;

                p->oldPos.x = p->sprite->x;
                p->oldPos.y = p->sprite->y;

                p->distance += sqrtf(SQR(dx) + SQR(dy));
            }

            if(p->distance >= p->distanceLimit)
            {
                if(p->flags & PF_STOP_ON_DISTANCE)
                {
                    p->vx = 0.0;
                    p->vy = 0.0;
                    p->rotateSpeed = 0.0;
                }

                if(p->flags & PF_DESTROY_ON_DISTANCE)
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
            if(p->sprite->opacity <= 0.0)
            {
                p->sprite->opacity = 0.0;
                p->destroyScheduled = true;
            }
        }

        if(p->destroyScheduled)
        {
            p->sprite->destroy = true;
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
                Particle *np = particles_Clone(p);

                p->oldTrailDistance = p->distance - dd + p->trailSpacing;

                np->vx = 0.0;
                np->vy = 0.0;
                np->sprite->animdir = 0;
                np->rotateSpeed = 0.0;
                np->fadeSpeed = p->trailFadeSpeed;
                particles_UnsetFlag(np, PF_LEAVE_TRAIL);
                particles_UnsetFlag(np, PF_GRAVITY_AFFECTED);
                particles_SetFlag(np, PF_DESTROY_ON_FADEOUT);

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
