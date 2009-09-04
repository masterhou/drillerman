#include "message.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "defaults.h"

#define colorRed(s) fprintf(s, "\033[1;31m")
#define colorBlue(s) fprintf(s, "\033[1;34m")
#define colorGreen(s) fprintf(s, "\033[1;32m")
#define colorEnd(s) fprintf(s, "\033[0m")

void messageCriticalError(const char *msg)
{
#ifdef __BASH_COLORS
    colorRed(stderr);
#endif
    fprintf(stderr, "E: ");
    fprintf(stderr, "critical error: %s", msg);
    exit(1);
#ifdef __BASH_COLORS
    colorEnd(stderr);
#endif
}

void messageWarning(const char *msg)
{
#ifdef __BASH_COLORS
    colorBlue(stderr);
#endif
    fprintf(stderr, "W: %s", msg);

#ifdef __BASH_COLORS
    colorEnd(stderr);
#endif
}

void messageCriticalErrorEx(const char *fmt, ...)
{
#ifdef __BASH_COLORS
    colorRed(stderr);
#endif
    fprintf(stderr, "E: ");
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
#ifdef __BASH_COLORS
    colorEnd(stderr);
#endif
}

void messageWarningEx(const char *fmt, ...)
{
#ifdef __BASH_COLORS
    colorBlue(stderr);
#endif
    fprintf(stderr, "W: ");
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
#ifdef __BASH_COLORS
    colorEnd(stderr);
#endif
}

void messageOut(const char *msg)
{
    printf("M: ");
    printf(msg);
}

void messageOutEx(const char *fmt, ...)
{
    printf("M: ");
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void messageOutEmf(const char *msg)
{
#ifdef __BASH_COLORS
    colorGreen(stdout);
#endif
    printf("!M: ");
    printf(msg);
#ifdef __BASH_COLORS
    colorEnd(stdout);
#endif
}

void messageOutEmfEx(const char *fmt, ...)
{
#ifdef __BASH_COLORS
    colorGreen(stdout);
#endif
    printf("!M: ");
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
#ifdef __BASH_COLORS
    colorEnd(stdout);
#endif
}
