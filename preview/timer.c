#include <windows.h>
#include <stdlib.h>

#include "timer.h"
#include "stopwatch.h"

typedef struct
{
    UINT            uDelay;
    UINT            uResolution;
    LPTIMECALLBACK  lpTimeProc;
    DWORD_PTR       dwUser;
    UINT            fuEvent;

    DWORD           lpThreadId; // 线程标识符
    HANDLE          hThread;    // 线程的句柄
    BOOL            __close;
    HANDLE          __cLock;
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
            break; // 终止线程运行的最佳方法是让它的线程函数返回
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

    // 将一个进程的优先级设置为 Realtime 是通知操作系统，我们绝不希望该进程将 CPU 时间出让给其它进程。
    // 如果你的程序误入一个无限循环，会发现甚至是操作系统也被锁住了，就只好去按电源按钮了o(>_<)o
    // 正是由于这一原因，High 通常是实时程序的最好选择。
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)
        || !SetThreadPriority(timer->hThread, THREAD_PRIORITY_HIGHEST)
        || timeBeginPeriod(timer->uResolution) == TIMERR_NOCANDO)
    {
        timer->__error = TRUE;
        ResumeThread(timer->hThread);

        MWFMO(timer->hThread, INFINITE);
        CloseHandle(timer->hThread);
        goto err; // 对于高精度来说任何一项失败都是致命的。
    }

    timer->__cLock = CreateMutex(NULL, FALSE, NULL);
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

    MWFMO(timer->__cLock, INFINITE);
    timer->__close = TRUE; // TerminateThread
    if ((timer->fuEvent & TIME_KILL_SYNCHRONOUS) == TIME_KILL_SYNCHRONOUS)
        timer->lpTimeProc = NULL;
    ReleaseMutex(timer->__cLock);

    MWFMO(timer->hThread, INFINITE); // Wait Functions
    timeEndPeriod(timer->uResolution);
    CH(timer->__cLock);
    CloseHandle(timer->hThread); // 关闭线程句柄不会终止关联的线程或删除线程对象
    free(timer);
    return TIMERR_NOERROR;
}