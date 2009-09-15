#ifndef TIMER_H
#define TIMER_H

typedef unsigned short int TimerHandle;

void timer_ProcessTimers(float lag);
void timer_CleanTimers();
TimerHandle timer_AddTimer(float interval, int oneshot);
int timer_Fired(TimerHandle handle);

#endif
