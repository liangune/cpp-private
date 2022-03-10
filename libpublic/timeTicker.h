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

class Timer;
class TimeTicker {
public:
    typedef std::list<Timer*> LIST_TIMER;
    typedef std::map<uint64_t, LIST_TIMER> MAP_TIMER;
public:
    TimeTicker();
    void stop();
    void start();
    void addTimer(Timer* pTimer);

private:
	void run();

private:
    std::mutex m_mutex;

    uint32_t m_nTimerCnt;
    std::atomic<bool> m_isEnable;

    MAP_TIMER m_mapTimer;
};


#endif //!_TIME_TICKER_H_