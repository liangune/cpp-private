/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   定时器
*************************************************************************/
#include "timer.h"
#include <chrono>

using namespace std::chrono;

uint64_t Timer::getMillisecond()
{
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

Timer::Timer()
{
    m_nNextTime = 0;
    m_nDuration = 0; 
    m_nType = eTimerTypeLoop;
}

Timer::Timer(eTimerType type)
{
    m_nType = type;
    m_nNextTime = 0;
    m_nDuration = 0; 
}


Timer::~Timer() 
{

}

void Timer::setType(eTimerType type) 
{
    m_nType = type;
}

Timer::eTimerType Timer::getType()
{
    return m_nType;
}

uint64_t Timer::getSortTime()
{
    return m_nSortTime;
}

uint64_t Timer::getNextTime()
{
    return m_nNextTime;
}

void Timer::setTimer(uint64_t nDuration, CallbackFunc func, void *arg)
{
    uint64_t now = getMillisecond();
    m_nNextTime = now;
    m_nDuration = nDuration;
    m_nSortTime = now + nDuration;

    m_pTimerCallback = std::make_shared<Callback>(func, arg);
}

void Timer::setTimerAfter(uint64_t nAfterTime, uint64_t nDuration, CallbackFunc func, void *arg)
{
    uint64_t now = getMillisecond();
    m_nNextTime = nAfterTime + now;
    m_nDuration = nDuration;
    m_nSortTime = nAfterTime + now + nDuration;

    m_pTimerCallback = std::make_shared<Callback>(func, arg);
}

void Timer::setTimer(uint64_t nDuration, const CallbackFunctor func, void *arg)
{
    uint64_t now = getMillisecond();
    m_nNextTime = now;
    m_nDuration = nDuration;
    m_nSortTime = now + nDuration;

    m_pTimerCallback = std::make_shared<Callback>(func, arg);
}

void Timer::setTimerAfter(uint64_t nAfterTime, uint64_t nDuration, const CallbackFunctor func, void *arg)
{
    uint64_t now = getMillisecond();
    m_nNextTime = nAfterTime + now;
    m_nDuration = nDuration;
    m_nSortTime = nAfterTime + now + nDuration;

    m_pTimerCallback = std::make_shared<Callback>(func, arg);
}

void Timer::reset()
{
    m_nNextTime += m_nDuration;
}
