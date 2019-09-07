#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <windows.h>

typedef struct
{
    LARGE_INTEGER Frequency;
    LARGE_INTEGER StartingTime;
    LARGE_INTEGER EndingTime;
    double ElapsedSeconds;
    double ElapsedMilliseconds;
    BOOL IsRunning;
} stopwatch_t;

void Stopwatch_StartNew(stopwatch_t t);
void Stopwatch_Reset(stopwatch_t t);
void Stopwatch_Restart(stopwatch_t t);
void Stopwatch_Start(stopwatch_t t);
void Stopwatch_Stop(stopwatch_t t);

#endif
