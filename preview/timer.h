#ifndef TIMER_H
#define TIMER_H

#include <windows.h>

/**
* 为了解决死锁问题，此函数仿照原函数进行了重写，fuEvent禁止使用TIME_CALLBACK_EVENT_SET和TIME_CALLBACK_EVENT_PULSE。
* 为了保证结果与原函数相同，仿函数允许fuEvent传入禁值，但不会对其做任何操作。
*
* 以下为原函数内容：
*
* 该的timeSetEvent功能启动指定的定时器事件。多媒体计时器在其自己的线程中运行。
* 激活事件后，它将调用指定的回调函数或设置或脉冲指定的事件对象。
*
* 每次调用timeSetEvent进行周期性定时器事件都需要对timeKillEvent函数进行相应的调用。
* 使用TIME_KILL_SYNCHRONOUS和TIME_CALLBACK_FUNCTION标志创建事件可防止在调用timeKillEvent函数后发生事件。
*
* @param uDelay         事件延迟，以毫秒为单位。如果此值不在计时器支持的最小和最大事件延迟范围内，则该函数返回错误。
* @param uResolution    计时器事件的解决方案，以毫秒为单位。分辨率随着值的增加而增加;
*                       分辨率为0表示应尽可能准确地发生周期性事件。但是，为了减少系统开销，您应该使用适合您的应用程序的最大值。
* @param lpTimeProc     指向一个回调函数的指针，该函数在单个事件到期时调用一次，或者在周期性事件到期时定期调用。
*                       如果fuEvent指定TIME_CALLBACK_EVENT_SET或TIME_CALLBACK_EVENT_PULSE标志，
*                       则lpTimeProc参数将被解释为事件对象的句柄。事件将在单个事件完成后设置或脉冲，或在定期事件完成后定期设置。
*                       对于fuEvent的任何其他值，lpTimeProc参数是指向LPTIMECALLBACK类型的回调函数的指针。
* @param dwUser         用户提供的回调数据。
* @param fuEvent        定时器事件类型。此参数可以包括以下值之一。
*                       TIME_ONESHOT                在uDelay毫秒之后，事件发生一次。
*                       TIME_PERIODIC               每隔uDelay毫秒发生一次事件。
*                       所述fuEvent参数还可以包括以下值中的一个。
*                       TIME_CALLBACK_FUNCTION	    当计时器到期时，Windows会调用lpTimeProc参数指向的函数。这是默认值。
*                       TIME_CALLBACK_EVENT_SET	    当计时器到期时，Windows调用SetEvent函数来设置lpTimeProc参数指向的事件。
*                                                   该dwUser参数被忽略。
*                       TIME_CALLBACK_EVENT_PULSE   当计时器到期时，Windows调用PulseEvent函数来脉冲lpTimeProc参数指向的事件。
*                                                   该dwUser参数被忽略。
*                       TIME_KILL_SYNCHRONOUS	    传递此标志可防止在调用timeKillEvent函数后发生事件。
* @return               如果成功则返回计时器事件的标识符，否则返回错误。如果失败并且未创建计时器事件，则此函数返回NULL。
*                       （此标识符也传递给回调函数。）
*/
MMRESULT TIMER_timeSetEvent(
    UINT            uDelay,
    UINT            uResolution,
    LPTIMECALLBACK  lpTimeProc,
    DWORD_PTR       dwUser,
    UINT            fuEvent
);

/**
* 注意！由于是仿函数（TIMER_timeKillEvent），因此不支持取消原函数（timeSetEvent）创建的计时器事件。
* 同理，原函数（timeKillEvent）也无法释放由仿函数（TIMER_timeSetEvent）创建的计时器事件。
*
* 以下为原函数内容：
*
* 该timeKillEvent功能取消指定计时器事件。
*
* @param uTimerID   要取消的计时器事件的标识符。设置计时器事件时，timeSetEvent函数返回此标识符。
* @return           返回TIMERR_NOERROR如果成功或MMSYSERR_INVALPARAM如果指定计时器事件不存在。
*/
MMRESULT TIMER_timeKillEvent(UINT uTimerID);

/**
* 调用等待函数和直接或间接创建窗口的代码时要小心。如果一个线程创建了任何窗口，它必须处理消息。
* 消息广播被发送到系统中的所有窗口。使用没有超时间隔的等待函数的线程可能会导致系统死锁。
* 因此，如果您有一个创建窗口的线程，请使用 MsgWaitForMultipleObjects 或 MsgWaitForMultipleObjectsEx。
*/
DWORD MsgWaitForSingleObject(HANDLE hHandle, DWORD  dwMilliseconds);

#endif