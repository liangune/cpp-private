#include "eventTimer.h"
#include <thread>
#include "event2/thread.h"
#include "event2/util.h"
#include <string.h>

#ifdef _MSC_VER
#include <winsock2.h>
#else
#include <unistd.h>
#endif

CEventTimer::CEventTimer() 
{
    
}

CEventTimer::~CEventTimer()
{
    if(m_pEventBase)
	{
		event_base_free(m_pEventBase);
		m_pEventBase = NULL;
	}

	for(auto iter = m_vecTimerParam.begin(); iter != m_vecTimerParam.end(); iter++)
	{
		evTimerParam *param = *iter;
		if(param) {
			event_free(param->pEvent);
			delete param;
			param = NULL;
		}
	}
}

bool CEventTimer::Setup()
{
	m_pEventBase = event_base_new();
    if(m_pEventBase == NULL) {
		dzlog_error("CEventTimer::Setup event_base_new is failed!");
		return false;
	}
    
    return true;
}

void CEventTimer::Start()
{
    std::thread th(&CEventTimer::Loop, std::ref(*this));
    th.detach();
}

void CEventTimer::Loop()
{
	int nRet = event_base_dispatch(m_pEventBase);
	if(nRet != 0) {
        dzlog_error("CEventTimer::Loop event_base_dispatch failed!, %d", nRet);
    }
}

bool CEventTimer::AddEvent(PIPE& pipeCheckClientKeepalive, const int32_t nTimeSpace)
{
	evTimerParam *param = new evTimerParam;
	param->nFd = pipeCheckClientKeepalive.nSendFd;
	param->nTimeSpace = nTimeSpace;
	m_vecTimerParam.push_back(param);

	param->pEvent = evtimer_new(m_pEventBase, EventCallback, (void *)param);
	if(pipeCheckClientKeepalive.pEvent == NULL) {
		dzlog_error("CEventTimer::AddEvent event_new failed!");
		return false;
	}
	
    struct timeval tv;
	evutil_timerclear(&tv);
    // x秒后触发
    tv.tv_sec = nTimeSpace;  
    tv.tv_usec = 0;

    if (evtimer_add(param->pEvent, &tv) == -1) {
        int32_t errorCode = EVUTIL_SOCKET_ERROR();
		dzlog_error("Could not add the timer to the event base, %s", evutil_socket_error_to_string(errorCode));
        return false;
    }

    return true;
}

void CEventTimer::EventCallback(evutil_socket_t fd, short events, void *arg)
{
	evTimerParam *param = static_cast<evTimerParam *>(arg);
    if (param == NULL) {
		return;
	}

	int32_t sendFd = param->nFd;

	static char buf[] = {PIPE_NOTIFY_CHR};
#ifdef _MSC_VER
	if (send(sendFd, buf, 1, 0) == SOCKET_ERROR) {
		dzlog_error("CEventTimer::EventCallback:Send to thread notify socket fd failed.");
		return;
	}
#else
	if (write(sendFd, buf, 1) != 1)
	{
		dzlog_error("CEventTimer::EventCallback:Writing to thread notify pipe failed.");
		return;
	}
#endif

    struct timeval tv;
	evutil_timerclear(&tv);
    tv.tv_sec = param->nTimeSpace;  
    tv.tv_usec = 0;
	evtimer_add(param->pEvent, &tv);
}