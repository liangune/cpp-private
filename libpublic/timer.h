/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   定时器
*************************************************************************/

#ifndef _TIMER_H_
#define _TIMER_H_
#include <stdint.h>
#include "callback.h"

class Timer {
public:
    typedef enum eTimerType {
        eTimerTypeOnce = 1,   // 定时执行一次 
        eTimerTypeLoop = 2,   // 定时执行多次, 默认执行多次
    }TimerType;
public:
    Timer();
    Timer(TimerType type);
    virtual ~Timer();

    void setType(TimerType type);
    TimerType getType();
    uint64_t getNextTime();
    uint64_t getSortTime();

    void setTimer(uint64_t nDuration, CallbackFunc func, void *arg);
    void setTimerAfter(uint64_t nAfterTime, uint64_t nDuration, CallbackFunc func, void *arg);

    void reset();

    static uint64_t getMillisecond();

    Callback *m_pTimerCallback;

private:
    uint64_t m_nSortTime;
    uint64_t m_nNextTime;   
    uint64_t m_nDuration;
    TimerType m_nType;
};



#endif //!_TIMER_H_