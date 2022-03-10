/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   定时检测回调
*************************************************************************/

#include "timer.h"
#include "timeTicker.h"

TimeTicker::TimeTicker() : m_nTimerCnt(0), m_isEnable(false)
{

}

void TimeTicker::stop()
{
    m_isEnable.store(false);
    std::lock_guard<std::mutex> guard(m_mutex);

    MAP_TIMER::iterator mapIt = m_mapTimer.begin();
    for (; mapIt != m_mapTimer.begin(); mapIt++) {            
        LIST_TIMER &list = mapIt->second;
        LIST_TIMER::iterator listIt = list.begin();
        for(; listIt != list.end();) {
            Timer *p = *listIt;
            if(p != NULL) {
                delete p;
                p = NULL;
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

    std::lock_guard<std::mutex> guard(m_mutex);

    uint64_t nSortTime = pTimer->getSortTime();
    MAP_TIMER::iterator mapIt = m_mapTimer.find(nSortTime);
    if(mapIt != m_mapTimer.end()) {
        LIST_TIMER &list = mapIt->second;
        list.push_back(pTimer);
    } else {
        LIST_TIMER list;
        list.push_back(pTimer);
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
                Timer *p = *listIt;
                if(p == NULL) {
                    listIt++;
                    continue;
                }

                if(nCurTime < p->getNextTime()) {
                    listIt++;
                    continue;
                }

                if(p->m_pTimerCallback) {
                    void *arg = p->m_pTimerCallback->m_pArg;
                    p->m_pTimerCallback->m_pFunc(arg);
                }

                if(p->getType() == Timer::eTimerTypeOnce) {
                    delete p;
                    p = NULL;
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
