#include <windows.h>

#include "stopwatch.h"

void Stopwatch_StartNew(stopwatch_t t)
{
    // 检索性能计数器的频率。性能计数器的频率在系统引导时是固定的，并且在所有处理器中都是一致的。
    // 因此，只需在应用程序初始化时查询频率，就可以缓存结果。
    QueryPerformanceFrequency(&t.Frequency);
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
    QueryPerformanceCounter(&t.EndingTime);
    t.ElapsedSeconds = (t.EndingTime.QuadPart - t.StartingTime.QuadPart) * 1.0 / t.Frequency.QuadPart;
    t.ElapsedMilliseconds = t.ElapsedSeconds * 1000;
    t.IsRunning = FALSE;
}