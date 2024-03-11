/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   定时检测回调
*************************************************************************/

#include "timeTicker.h"

TimeTicker::TimeTicker() : m_nTimerCnt(0), m_isEnable(false)
{

}

TimeTicker::~TimeTicker()
{
    stop();
}

void TimeTicker::stop()
{
    m_isEnable.store(false);
    std::lock_guard<std::mutex> guard(m_mutex);

    MAP_TIMER::iterator mapIt = m_mapTimer.begin();
    for (; mapIt != m_mapTimer.end(); mapIt++) {            
        LIST_TIMER &list = mapIt->second;
        LIST_TIMER::iterator listIt = list.begin();
        for(; listIt != list.end();) {
            TimerPtr p = *listIt;
            if(p != NULL) {
                m_nTimerCnt--;
#ifdef _MSC_VER
                listIt = list.erase(listIt);
#else
                list.erase(listIt++);
#endif
            } else {
                listIt++;
            }
        }
    }

    m_mapTimer.clear();
}

void TimeTicker::addTimer(Timer* pTimer)
{  
    if(pTimer==NULL)
        return;

    TimerPtr timerPtr(pTimer);
    uint64_t nSortTime = timerPtr->getSortTime();
    
    std::lock_guard<std::mutex> guard(m_mutex);
    MAP_TIMER::iterator mapIt = m_mapTimer.find(nSortTime);
    if(mapIt != m_mapTimer.end()) {
        LIST_TIMER &list = mapIt->second;
        list.push_back(timerPtr);
    } else {
        LIST_TIMER list;
        list.push_back(timerPtr);
        m_mapTimer[nSortTime] = list;
    }

    m_nTimerCnt++;
}

void TimeTicker::addTimer(uint64_t nDuration, const CallbackFunctor func, void *arg)
{  
    TimerPtr timerPtr(std::make_shared<Timer>());
    timerPtr->setTimer(nDuration, func, arg);
    uint64_t nSortTime = timerPtr->getSortTime();
	
    std::lock_guard<std::mutex> guard(m_mutex);
    MAP_TIMER::iterator mapIt = m_mapTimer.find(nSortTime);
    if(mapIt != m_mapTimer.end()) {
        LIST_TIMER &list = mapIt->second;
        list.push_back(timerPtr);
    } else {
        LIST_TIMER list;
        list.push_back(timerPtr);
        m_mapTimer[nSortTime] = list;
    }

    m_nTimerCnt++;
}

void TimeTicker::addTimerAfter(uint64_t nAfterTime, uint64_t nDuration, const CallbackFunctor func, void *arg)
{
    TimerPtr timerPtr(std::make_shared<Timer>());
    timerPtr->setTimerAfter(nAfterTime, nDuration, func, arg);
    uint64_t nSortTime = timerPtr->getSortTime();

    std::lock_guard<std::mutex> guard(m_mutex);
    MAP_TIMER::iterator mapIt = m_mapTimer.find(nSortTime);
    if(mapIt != m_mapTimer.end()) {
        LIST_TIMER &list = mapIt->second;
        list.push_back(timerPtr);
    } else {
        LIST_TIMER list;
        list.push_back(timerPtr);
        m_mapTimer[nSortTime] = list;
    }

    m_nTimerCnt++;  
}

void TimeTicker::start() 
{
    m_isEnable.store(true);
    std::thread th(&TimeTicker::run, std::ref(*this));
    th.detach();
}

void TimeTicker::run()
{
    while (m_isEnable.load()) {
        m_mutex.lock();

        MAP_TIMER::iterator mapIt = m_mapTimer.begin();
        for (; mapIt != m_mapTimer.end(); mapIt++) {
            uint64_t nCurTime = Timer::getMillisecond();
            LIST_TIMER &list = mapIt->second;
            LIST_TIMER::iterator listIt = list.begin();
            for(; listIt != list.end();) {
                TimerPtr p = *listIt;
                if(p == NULL) {
                    listIt++;
                    continue;
                }

                if(nCurTime < p->getNextTime()) {
                    listIt++;
                    continue;
                }

                if(p->m_pTimerCallback) {
                    p->m_pTimerCallback->Execute();
                }

                if(p->getType() == Timer::eTimerTypeOnce) {
                    m_nTimerCnt--;
#ifdef _MSC_VER
                    listIt = list.erase(listIt);
#else
                    list.erase(listIt++);
#endif
                } else {
                    p->reset();
                    listIt++;
                }
            }
        }

        m_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
