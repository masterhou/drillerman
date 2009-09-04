#ifndef TIMER_H
#define TIMER_H

typedef unsigned short int TimerHandle;

void timerProcessTimers(float lag);
void timerCleanTimers();
TimerHandle timerAddTimer(float interval, int oneshot);
int timerFired(TimerHandle handle);

#endif
