#ifndef MAINLOOP_H
#define MAINLOOP_H

typedef enum 
{
    SCR_EXIT,
    SCR_SPLASH,
    SCR_MENU,
    SCR_GAME,
    SCR_LAST,
    SCR_NONE
} ScreenId;

void mainloopChangeScr(ScreenId newscr, void *data);
void mainloopChangeScrWithFade(ScreenId newscr, void *data, float duration);
void mainloopGo();

#endif
