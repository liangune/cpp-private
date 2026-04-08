/*定时事件，使用管道通知各个工作线程
**比如定时检查客户端连接keepalive
*/

#ifndef _EVENT_TIMER_H_
#define _EVENT_TIMER_H_

#include "event2/event.h"
#include <stdint.h>
#include "typedef.h"
#include "zlog/zlog.h"
#include <vector>

typedef struct {
    struct event *pEvent;
    int32_t nFd;
    int32_t nTimeSpace;
}evTimerParam;

class CEventTimer
{
public:
    CEventTimer();
    ~CEventTimer();
    
    bool Setup();
    void Start();
    void Loop();
    bool AddEvent(PIPE& pipeCheckClientKeepalive, const int32_t nTimeSpace);

private:
    static void EventCallback(evutil_socket_t fd, short events, void *arg);

private:
    struct event_base* m_pEventBase;
    std::vector<evTimerParam*> m_vecTimerParam;
};

#endif  //_EVENT_TIMER_H_
