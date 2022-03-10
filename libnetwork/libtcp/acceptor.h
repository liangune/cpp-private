/*************************************************************************
	> Date:   2020-12-22
	> Author: liangjun
	> Desc:   接收器
*************************************************************************/

#ifndef _LIB_TCP_ACCEPTOR_H_
#define _LIB_TCP_ACCEPTOR_H_

#include "event2/event.h"
#include "zlog/zlog.h"
#include <stdint.h>
#include "typedef.h"

class CAcceptor
{
public:
    CAcceptor();
    ~CAcceptor();

    bool Init(struct event_base* pBase, int32_t port, int32_t nBacklog, void* pParam);
    bool Free();

    struct event_base* GetEventBase(){return m_pEventBase;};

private:
    void SetListenIp(const char*pszIp);
    evutil_socket_t CreateListenSocket(int32_t port, int32_t nBacklog);
    static void AccepCb(evutil_socket_t listenSocket, short event, void* arg);

private:
	struct event_base*  m_pEventBase;
	struct event*       m_pListenEvent;
	evutil_socket_t		m_listenSocket;
    uint32_t			m_nListenIp;
};

#endif  //_LIB_TCP_ACCEPTOR_H_
