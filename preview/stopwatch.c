#include <windows.h>

#include "stopwatch.h"

void Stopwatch_StartNew(stopwatch_t t)
{
    Stopwatch_New(t);
    Stopwatch_Start(t);
}

void Stopwatch_Reset(stopwatch_t t)
{
    t.StartingTime.QuadPart = 0;
    t.EndingTime.QuadPart = 0;
    t.ElapsedSeconds = 0;
    t.ElapsedMilliseconds = 0;
    t.IsRunning = FALSE;
}

void Stopwatch_Restart(stopwatch_t t)
{
    Stopwatch_Reset(t);
    Stopwatch_Start(t);
}

void Stopwatch_Start(stopwatch_t t)
{
    QueryPerformanceCounter(&t.StartingTime);
    t.IsRunning = TRUE;
}

void Stopwatch_Stop(stopwatch_t t)
{
    Stopwatch_Elapsed(t);
    t.IsRunning = FALSE;
}

stopwatch_t Stopwatch_Elapsed(stopwatch_t t)
{
    if (t.IsRunning)
    {
        QueryPerformanceCounter(&t.EndingTime);
        t.ElapsedSeconds = (t.EndingTime.QuadPart - t.StartingTime.QuadPart) * 1.0 / t.Frequency.QuadPart;
        t.ElapsedMilliseconds = t.ElapsedSeconds * 1000;
    }
    return t;
}

UINT Stopwatch_ElapsedSeconds(stopwatch_t t)
{
    return Stopwatch_Elapsed(t).ElapsedSeconds;
}

UINT Stopwatch_ElapsedMilliseconds(stopwatch_t t)
{
    return Stopwatch_Elapsed(t).ElapsedMilliseconds;
}