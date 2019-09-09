#include <windows.h>

#include "stopwatch.h"

void Stopwatch_StartNew(stopwatch_t* p)
{
    Stopwatch_New(p);
    Stopwatch_Start(p);
}

void Stopwatch_Reset(stopwatch_t* p)
{
    p->StartingTime.QuadPart = 0;
    p->EndingTime.QuadPart = 0;
    p->ElapsedSeconds = 0;
    p->ElapsedMilliseconds = 0;
    p->IsRunning = FALSE;
}

void Stopwatch_Restart(stopwatch_t* p)
{
    Stopwatch_Reset(p);
    Stopwatch_Start(p);
}

void Stopwatch_Start(stopwatch_t* p)
{
    QueryPerformanceCounter(&p->StartingTime);
    p->IsRunning = TRUE;
}

void Stopwatch_Stop(stopwatch_t* p)
{
    Stopwatch_Elapsed(p);
    p->IsRunning = FALSE;
}

stopwatch_t* Stopwatch_Elapsed(stopwatch_t* p)
{
    if (p->IsRunning)
    {
        QueryPerformanceCounter(&p->EndingTime);
        p->ElapsedSeconds = (p->EndingTime.QuadPart - p->StartingTime.QuadPart) * 1.0 / p->Frequency.QuadPart;
        p->ElapsedMilliseconds = p->ElapsedSeconds * 1000;
    }
    return p;
}

int Stopwatch_ElapsedSeconds(stopwatch_t* p)
{
    return Stopwatch_Elapsed(p)->ElapsedSeconds;
}

int Stopwatch_ElapsedMilliseconds(stopwatch_t* p)
{
    return Stopwatch_Elapsed(p)->ElapsedMilliseconds;
}