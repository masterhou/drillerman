#include "message.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "defs.h"

#define colorRed(s) fprintf(s, "\033[1;31m")
#define colorBlue(s) fprintf(s, "\033[1;34m")
#define colorGreen(s) fprintf(s, "\033[1;32m")
#define colorEnd(s) fprintf(s, "\033[0m")

void message_CriticalError(const char *msg)
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

void message_Warning(const char *msg)
{
#ifdef __BASH_COLORS
    colorBlue(stderr);
#endif
    fprintf(stderr, "W: %s", msg);

#ifdef __BASH_COLORS
    colorEnd(stderr);
#endif
}

void message_CriticalErrorEx(const char *fmt, ...)
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

void message_WarningEx(const char *fmt, ...)
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

void message_Out(const char *msg)
{
    printf("M: ");
    printf(msg);
}

void message_OutEx(const char *fmt, ...)
{
    printf("M: ");
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void message_OutEmf(const char *msg)
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

void message_OutEmfEx(const char *fmt, ...)
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
