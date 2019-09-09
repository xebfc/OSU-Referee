#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <windows.h>

// 检索性能计数器的频率。性能计数器的频率在系统引导时是固定的，并且在所有处理器中都是一致的。
// 因此，只需在应用程序初始化时查询频率，就可以缓存结果。
#define Stopwatch_New(p) QueryPerformanceFrequency(&(p)->Frequency)

typedef struct
{
    LARGE_INTEGER Frequency;
    LARGE_INTEGER StartingTime;
    LARGE_INTEGER EndingTime;
    double ElapsedSeconds;
    double ElapsedMilliseconds;
    BOOL IsRunning;
} stopwatch_t;

void Stopwatch_StartNew(stopwatch_t* p);
void Stopwatch_Reset(stopwatch_t* p);
void Stopwatch_Restart(stopwatch_t* p);
void Stopwatch_Start(stopwatch_t* p);
void Stopwatch_Stop(stopwatch_t* p);

stopwatch_t* Stopwatch_Elapsed(stopwatch_t* p);
int Stopwatch_ElapsedSeconds(stopwatch_t* p);
int Stopwatch_ElapsedMilliseconds(stopwatch_t* p);

#endif