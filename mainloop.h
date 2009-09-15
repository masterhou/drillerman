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

void mainloop_ChangeScr(ScreenId newscr, void *data);
void mainloop_ChangeScrWithFade(ScreenId newscr, void *data, float duration);
void mainloop_Go();

#endif
