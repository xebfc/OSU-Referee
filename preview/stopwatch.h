#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <windows.h>

// 检索性能计数器的频率。性能计数器的频率在系统引导时是固定的，并且在所有处理器中都是一致的。
// 因此，只需在应用程序初始化时查询频率，就可以缓存结果。
#define Stopwatch_New(t) QueryPerformanceFrequency(&t.Frequency);

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

stopwatch_t Stopwatch_Elapsed(stopwatch_t t);
UINT Stopwatch_ElapsedSeconds(stopwatch_t t);
UINT Stopwatch_ElapsedMilliseconds(stopwatch_t t);

#endif