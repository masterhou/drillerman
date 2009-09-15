#ifndef MESSAGE_H
#define MESSAGE_H

void message_CriticalError(const char *msg);
void message_CriticalErrorEx(const char *fmt, ...);

void message_Warning(const char *msg);
void message_WarningEx(const char *fmt, ...);

void message_Out(const char *msg);
void message_OutEx(const char *fmt, ...);

void message_OutEmf(const char *msg);
void message_OutEmfEx(const char *fmt, ...);


#endif
