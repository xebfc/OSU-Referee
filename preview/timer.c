#include <windows.h>
#include <stdlib.h>

#include "timer.h"
#include "stopwatch.h"

static stopwatch_t default_stopwatch;

typedef struct
{
    UINT            uDelay;
    UINT            uResolution;
    LPTIMECALLBACK  lpTimeProc;
    DWORD_PTR       dwUser;
    UINT            fuEvent;

    DWORD           lpThreadId; // 线程标识符
    DWORD           hThread;    // 线程的句柄
    BOOL            __close;
    HANDLE          __cLock;
} timer_t;

// ThreadProc
DWORD WINAPI lpStartAddress(LPVOID lpParameter)
{
    timer_t* timer = (timer_t*)lpParameter;

    stopwatch_t s;
    Stopwatch_StartNew(&s);

    // 固定时间框架
    UINT next = 0;
    while (TRUE)
    {
        next += timer->uDelay;
        while ((UINT)Stopwatch_ElapsedMilliseconds(&s) < next)
            Sleep(next - (UINT)s.ElapsedMilliseconds); // 自旋 + 阻塞

        WaitForSingleObject(timer->__cLock, 0);
        if (timer->lpTimeProc != NULL)
            timer->lpTimeProc(timer->lpThreadId, 0, timer->dwUser, 0, 0); // 触发任务
        ReleaseMutex(timer->__cLock);

        if (timer->__close)
            break; // 强行终止会导致资源无法被正确释放
    }

    return 0;
}

//-------------------------------------------------------------------------------

MMRESULT TIMER_timeSetEvent(
    UINT            uDelay,
    UINT            uResolution,
    LPTIMECALLBACK  lpTimeProc,
    DWORD_PTR       dwUser,
    UINT            fuEvent
)
{
    timer_t* timer = (timer_t*)malloc(sizeof(timer_t));

    if (timer == NULL)
        return NULL;

    timer->uDelay = uDelay;
    timer->uResolution = uResolution == 0 ? 1 : timer->uResolution; // 最低支持1ms分辨率，否则将超出范围
    timer->lpTimeProc = lpTimeProc;
    timer->dwUser = dwUser;
    timer->fuEvent = fuEvent;

    timer->hThread = CreateThread(NULL, 0, lpStartAddress, timer, CREATE_SUSPENDED, &timer->lpThreadId);

    if (timer->hThread == NULL)
        goto err;

    timer->__close = (timer->fuEvent & TIME_PERIODIC) != TIME_PERIODIC; // TIME_ONESHOT
    timer->__cLock = CreateMutex(NULL, FALSE, NULL);

    if (!SetThreadPriority(timer->hThread, THREAD_PRIORITY_HIGHEST)
        || timeBeginPeriod(timer->uResolution) == TIMERR_NOCANDO
        || ResumeThread(timer->hThread) == -1)
        goto err; // 任何一项失败对高精度来说都是致命的

    return timer;

err:
    free(timer);
    return NULL;
}

MMRESULT TIMER_timeKillEvent(UINT uTimerID)
{
    timer_t* timer = (timer_t*)uTimerID;

    if (timer == NULL || timer->hThread == NULL)
        return MMSYSERR_INVALPARAM;

    MWFMO(timer->__cLock, INFINITE);
    timer->__close = TRUE; // TerminateThread
    if ((timer->fuEvent & TIME_KILL_SYNCHRONOUS) == TIME_KILL_SYNCHRONOUS)
        timer->lpTimeProc = NULL;
    ReleaseMutex(timer->__cLock);

    CloseHandle(timer->hThread);
    timeEndPeriod(timer->uResolution);
    free(timer);
    return TIMERR_NOERROR;
}