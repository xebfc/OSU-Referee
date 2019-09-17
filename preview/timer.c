#include <windows.h>
#include <stdlib.h>

#include "timer.h"
#include "stopwatch.h"

// 最低支持1ms分辨率，否则将超出范围
#define getResolution(timer) (timer->uResolution ? timer->uResolution : 1)

typedef struct
{
    UINT            uDelay;
    UINT            uResolution;
    LPTIMECALLBACK  lpTimeProc;
    DWORD_PTR       dwUser;
    UINT            fuEvent;

    DWORD           lpThreadId; // 线程标识符
    HANDLE          hThread;    // 线程句柄
    BOOL            __close;
    BOOL            __error;
} timer_t;

// ThreadProc
DWORD WINAPI lpStartAddress(LPVOID lpParameter)
{
    timer_t* timer = (timer_t*)lpParameter;

    if (timer->__error)
        return 1; // ExitThread

    stopwatch_t s;
    Stopwatch_StartNew(&s);

    // 固定时间框架
    UINT next = 0, delay;
    while (TRUE)
    {
        next += timer->uDelay;
        while ((UINT)Stopwatch_ElapsedMilliseconds(&s) < next)
        {
            if (!timer->uResolution)
                continue; // 自旋的精准度最高，尽可能准确地发生周期性事件

            // Sleep(1) 会在 1ms 与 2ms 两种状态间跳动，也就是可能会产生 +1ms 多的误差。如果需要修正这个误差，
            // 可以在阻塞 n 毫秒时，使用 Sleep(n-1)，并通过 Stopwatch 计时，剩余等待时间用 Sleep(0) 或自旋来补充。
            // Sleep(0) 在 CPU 高负载情况下非常不稳定，实测可能会阻塞高达 6ms 时间，所以可能会产生更多的误差。
            // 因此误差修正最好通过自旋方式实现。
            if ((delay = next - (UINT)s.ElapsedMilliseconds) > 1)
                Sleep(delay - 1);
        }

        timer->lpTimeProc(timer, 0, timer->dwUser, 0, 0); // 触发任务

        if (timer->__close)
            break; // 终止线程运行的最佳方法是让它的线程函数返回
    }

    timeEndPeriod(getResolution(timer));
    CloseHandle(timer->hThread); // 关闭线程句柄不会终止关联的线程或删除线程对象

    free(timer);
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
    timer->uResolution = uResolution;
    timer->lpTimeProc = lpTimeProc;
    timer->dwUser = dwUser;
    timer->fuEvent = fuEvent;

    timer->hThread = CreateThread(NULL, 0, lpStartAddress, timer, CREATE_SUSPENDED, &timer->lpThreadId);

    if (timer->hThread == NULL)
        goto err;

    timer->__close = (timer->fuEvent & TIME_PERIODIC) != TIME_PERIODIC; // TIME_ONESHOT

    // 将一个进程的优先级设置为 Realtime 是通知操作系统，我们绝不希望该进程将 CPU 时间出让给其它进程。
    // 如果你的程序误入一个无限循环，会发现甚至是操作系统也被锁住了，就只好去按电源按钮了o(>_<)o
    // 正是由于这一原因，High 通常是实时程序的最好选择。
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)
        || !SetThreadPriority(timer->hThread, THREAD_PRIORITY_HIGHEST)
        || timeBeginPeriod(getResolution(timer)) == TIMERR_NOCANDO)
    {
        timer->__error = TRUE;
        ResumeThread(timer->hThread);

        MWFMO(timer->hThread, INFINITE); // Wait Function
        CloseHandle(timer->hThread);
        goto err; // 对于高精度来说任何一项失败都是致命的
    }

    timer->__error = FALSE;
    ResumeThread(timer->hThread);
    return timer;

err:
    free(timer);
    return NULL;
}

MMRESULT TIMER_timeKillEvent(UINT uTimerID)
{
    timer_t* timer = (timer_t*)uTimerID;

    // 确定调用进程是否具有对指定地址的内存的读访问权
    if (timer == NULL || IsBadCodePtr(timer))
        return MMSYSERR_INVALPARAM;

    // 判断线程句柄是否有效
    DWORD lpThreadId = GetThreadId(timer->hThread);
    if (!lpThreadId || lpThreadId != timer->lpThreadId)
        return MMSYSERR_INVALPARAM;

    timer->__close = TRUE; // TerminateThread
    if ((timer->fuEvent & TIME_KILL_SYNCHRONOUS) == TIME_KILL_SYNCHRONOUS)
        MWFMO(timer->hThread, INFINITE);
    return TIMERR_NOERROR;
}