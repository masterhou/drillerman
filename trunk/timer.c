#include "timer.h"

#include <stdlib.h>
#include <assert.h>

#include "common.h"

/*
    This module implements non-callback
    non-threaded timers tied to the frame
    loop. A timer firing can be missed so
    it should be checked each frame where
    applicable.
*/

typedef struct
{
    int oneshot;
    TimerHandle handle;
    float time;
    float interval;
} Timer;

static int timerCount = 0;
static int maxHandle = 0;
Timer **timers = NULL;

void timer_ProcessTimers(float lag)
{
    int i;

    for(i = 0; i < timerCount; ++i)
        timers[i]->time += lag;
}

int timer_Fired(TimerHandle handle)
{
    int a, b, c, d = 0;

    if(timerCount == 0)
        return 0;

    a = 0;
    b = timerCount - 1;
    c = timerCount / 2;

    while(a != b &&
          timers[a]->handle != handle &&
          timers[b]->handle != handle &&
          timers[c]->handle != handle)
    {
        c = a + ((b - a) / 2);

        if(timers[c]->handle > handle)
            b = c;
        else
            a = c;
    }

    if(timers[a]->handle == handle)
        d = a;
    else if(timers[b]->handle == handle)
        d = b;
    else if(timers[c]->handle == handle)
        d = c;
    else
        return 0;

    if(timers[d]->time >= timers[d]->interval)
        return 1;

    return 0;
}

TimerHandle timer_AddTimer(float interval, int oneshot)
{
    Timer *t = malloc(sizeof(Timer));

    t->handle = maxHandle;
    t->interval = interval;
    t->oneshot = oneshot;
    t->time = 0;

    maxHandle++;

    timers = realloc(timers, sizeof(Timer*) * (timerCount + 1));
    timers[timerCount] = t;

    timerCount++;
    return t-> handle;
}

void timer_CleanTimers()
{
    if(timerCount == 0)
        return;

    Timer **tmptimers = NULL;
    int tmptimerCount = 0;

    int i;

    for(i = 0; i < timerCount; ++i)
    {
        Timer *t = timers[i];

        if(t->time < t->interval || !t->oneshot)
        {
            tmptimers = realloc(tmptimers, sizeof(Timer*) * (tmptimerCount + 1));
            tmptimers[tmptimerCount] = t;
            tmptimerCount++;
        }

        if(t->time >= t->interval)
        {
            if(!t->oneshot)
                t->time -= t->interval;
            else
                free(t);
        }

    }

    if(tmptimerCount == 0)
    {
        free(timers);
        timers = NULL;
        timerCount = 0;
        return;
    }

    free(timers);
    timers = tmptimers;
    timerCount = tmptimerCount;
}
