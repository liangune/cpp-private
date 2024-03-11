/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   定时检测回调
*************************************************************************/

#ifndef _TIME_TICKER_H_
#define _TIME_TICKER_H_
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <stdint.h>
#include "timer.h"

class TimeTicker {
public:
    typedef std::list<TimerPtr> LIST_TIMER;
    typedef std::map<uint64_t, LIST_TIMER> MAP_TIMER;
public:
    TimeTicker();
    ~TimeTicker();
    void stop();
    void start();
    
    // 回调函数为静态函数可使用, 回调函数为函数对象windows平台可用, linux平台调用会崩溃
    void addTimer(Timer* pTimer);  // DEPRECATED

    // 回调函数为函数对象时使用
    void addTimer(uint64_t nDuration, const CallbackFunctor func, void *arg);
    void addTimerAfter(uint64_t nAfterTime, uint64_t nDuration, const CallbackFunctor func, void *arg);

private:
	void run();

private:
    std::mutex m_mutex;

    uint32_t m_nTimerCnt;
    std::atomic<bool> m_isEnable;

    MAP_TIMER m_mapTimer;
};

using TimeTickerPtr = std::shared_ptr<TimeTicker>;

#endif //!_TIME_TICKER_H_