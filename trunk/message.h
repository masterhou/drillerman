#ifndef MESSAGE_H
#define MESSAGE_H

void messageCriticalError(const char *msg);
void messageCriticalErrorEx(const char *fmt, ...);

void messageWarning(const char *msg);
void messageWarningEx(const char *fmt, ...);

void messageOut(const char *msg);
void messageOutEx(const char *fmt, ...);

void messageOutEmf(const char *msg);
void messageOutEmfEx(const char *fmt, ...);


#endif
