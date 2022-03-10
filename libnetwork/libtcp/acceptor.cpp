/*************************************************************************
	> Date:   2020-12-22
	> Author: liangjun
	> Desc:   接收器
*************************************************************************/

#include "acceptor.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <cassert>

#ifdef _MSC_VER
#include <WS2tcpip.h>
#else
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

CAcceptor::CAcceptor(): m_pEventBase(NULL), m_pListenEvent(NULL), m_listenSocket(0), m_nListenIp(INADDR_ANY)
{
}

CAcceptor::~CAcceptor()
{
}

void CAcceptor::SetListenIp(const char*pszIp)
{
	if( pszIp )
		m_nListenIp = inet_addr(pszIp);
}

evutil_socket_t CAcceptor::CreateListenSocket(int32_t port, int32_t nBacklog)
{	
	evutil_socket_t listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if( listenSocket < 0)
	{
		dzlog_error("create socket error: %s", strerror(errno));
		return -1;
	}

	int32_t flags = 1;
#ifdef _MSC_VER
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flags, sizeof(flags)) != 0)
#else
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags)) != 0)
#endif
	{
		dzlog_error("setsockopt SO_REUSEADDR error: %s\n", strerror(errno));
#ifdef _MSC_VER
        closesocket(listenSocket);
#else
        close(listenSocket);
#endif
		return -1;
	}

	sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = m_nListenIp;
	servAddr.sin_port = htons(port);
	dzlog_info("<<<<<<<<<<<<<<< listen port: %d >>>>>>>>>>>>", port);

	if (bind(listenSocket, (const sockaddr*)&servAddr, sizeof(servAddr)) != 0)
	{
		dzlog_error("bind socket error: %s", strerror(errno));
#ifdef _MSC_VER
        closesocket(listenSocket);
#else
        close(listenSocket);
#endif
		return -1;
	}

	if(listen(listenSocket, nBacklog) != 0)
	{
		dzlog_error("listen socket error: %s", strerror(errno));
#ifdef _MSC_VER
        closesocket(listenSocket);
#else
        close(listenSocket);
#endif
		return -1;
	}

	return listenSocket;
}


bool CAcceptor::Init(struct event_base* pBase, int32_t port, int32_t nBacklog, void* pParam)
{
	m_pEventBase   = pBase;
	evutil_socket_t listenSocket = CreateListenSocket(port, nBacklog);
	if( listenSocket < 0)
	{
		dzlog_error("create socket(*:%d) error: %s", port, strerror(errno));
		return false;
	}

    // 设置非阻塞
	evutil_make_socket_nonblocking(listenSocket);
    //evutil_make_listen_socket_reuseable(listenSocket);

	m_listenSocket = listenSocket;
	struct event * pListenEvent = event_new(pBase, listenSocket, EV_READ|EV_PERSIST, AccepCb, pParam);
	if( !pListenEvent )
	{
		int32_t errorCode = EVUTIL_SOCKET_ERROR();
		dzlog_error("event_new listen error: %s", evutil_socket_error_to_string(errorCode));
#ifdef _MSC_VER
        closesocket(listenSocket);
#else
        close(listenSocket);
#endif
		return false;
	}

	m_pListenEvent = pListenEvent;
	if(event_add(m_pListenEvent, NULL) == -1)
	{
		int32_t errorCode = EVUTIL_SOCKET_ERROR();
		dzlog_error("event_add error: %s", evutil_socket_error_to_string(errorCode));
#ifdef _MSC_VER
        closesocket(listenSocket);
#else
        close(listenSocket);
#endif
		return false;
	}

    return true;
}

void CAcceptor::AccepCb(evutil_socket_t listenSocket, short event, void* arg)
{
    AcceptorCbParam* pParam = static_cast<AcceptorCbParam*>(arg);

	evutil_socket_t sfd;
	struct sockaddr_in sin;
	socklen_t slen = sizeof(sin);
	while(true)
	{
		sfd = accept(listenSocket, (struct sockaddr *) &sin, &slen);
		if( sfd < 0)
		{
#ifdef _MSC_VER
            if (errno == EWOULDBLOCK || errno == EAGAIN  || errno == EPROTO || 
                errno == ENOPROTOOPT || errno == EOPNOTSUPP ||
				errno == ENETDOWN || errno == ENETUNREACH ||
				errno == EHOSTUNREACH || errno == ECONNABORTED || errno == ENOENT || errno == EACCES || errno == EEXIST)
#else
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == ENONET || 
				errno == EPROTO || errno == ENOPROTOOPT || errno == EOPNOTSUPP ||
				errno == ENETDOWN || errno == ENETUNREACH || errno == EHOSTDOWN ||
				errno == EHOSTUNREACH || errno == ECONNABORTED || errno == ENOENT || errno == EACCES || errno == EEXIST)
#endif
			{
				break;
			}
			else if( errno == EINTR)
			{
				continue;
			}
			else if( errno == EMFILE)
			{
				dzlog_warn("AccepCb: accept error: %s", strerror(errno));
				break;
			}
			else
			{
				if(errno == 0) {
					break;
				}
				printf("AccepCb: accept error\n");
				dzlog_error("AccepCb: accept error: %s", strerror(errno));
				//assert(false);
				break;
			}
		}

        (pParam->pObj->*(pParam->callback) )(sfd);
	}
}

bool CAcceptor::Free()
{
    if( m_pEventBase )
	{
		event_base_free(m_pEventBase);
		m_pEventBase = NULL;
	}

	m_pListenEvent = NULL;

	if( m_listenSocket )
	{
#ifdef _MSC_VER
        closesocket(m_listenSocket);
#else
        close(m_listenSocket);
#endif
		m_listenSocket = 0;
	}

    return true;
}