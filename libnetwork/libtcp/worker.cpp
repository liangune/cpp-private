/*************************************************************************
	> Date:   2020-12-22
	> Author: liangjun
	> Desc:   工作线程
*************************************************************************/

#include "worker.h"
#include "tcpMaster.h"
#include "event2/bufferevent.h"
#include "event2/thread.h"

#ifdef _MSC_VER
#include <winsock2.h>
#else
#include <unistd.h>
#endif

CWorker::CWorker(): m_pBase(NULL), m_pParserFactory(NULL), m_nIndex(0)
{

}

CWorker::~CWorker()
{
	this->Destroy();
}

void CWorker::Destroy()
{
	while (m_connList.GetCount() > 0)
	{
		CONN_INFO *pConnInfo = m_connList.GetHeadAndRemove();
		if (pConnInfo)
		{
			CloseConn(pConnInfo, pConnInfo->pClientTcpEvent);
		}
	}
	
	if(m_pBase)
	{
		event_base_free(m_pBase);
		m_pBase = NULL;
	}

#ifdef _MSC_VER
	closesocket(m_pipeNotify.nReceiveFd);
	closesocket(m_pipeNotify.nSendFd);
#else
	close(m_pipeNotify.nReceiveFd);
	close(m_pipeNotify.nSendFd);
#endif
	m_pipeNotify.nReceiveFd = 0;
	m_pipeNotify.nSendFd = 0;
}


void CWorker::SetUseTimeout(bool isUseTimeout)
{
	m_isUseTimeout = isUseTimeout;
}

void CWorker::SetKeepaliveSec(uint32_t nSec)
{
	m_nKeepaliveSec = nSec;
}

void CWorker::SetIndex(uint32_t nIndex)
{
	m_nIndex = nIndex;
}

uint32_t CWorker::GetIndex()
{
	return m_nIndex;
}

void CWorker::SetWorkerThreadId(std::thread::id pid)
{
	m_threadId = pid;
}

std::thread::id CWorker::GetWorkerThreadId()
{
	return m_threadId;
}

bool CWorker::Setup()
{
#ifdef _MSC_VER
	event_config *conf = event_config_new();
	if (!conf) {
		dzlog_error("event_config_new is failed!");
		return false;
	}

	// windows iocp
	event_config_set_flag(conf, EVENT_BASE_FLAG_STARTUP_IOCP);
	evthread_use_windows_threads();

	/*
	SYSTEM_INFO sinfo;
	GetSystemInfo(&sinfo);
	event_config_set_num_cpus_hint(conf, sinfo.dwNumberOfProcessors);
	*/
	m_pBase = event_base_new_with_config(conf);
	event_config_free(conf);
#else
	m_pBase = event_base_new();
#endif

	if(m_pBase == NULL) {
		dzlog_error("event_base_new is failed!");
		return false;
	}
	
	//主线程通知有新的客户端连接
	if (false == SetupPipe(m_pipeNotify, ReadPipeCb) )
		return false;
    
    return true;
}

bool CWorker::SetupPipe(PIPE& pp, void (*func)(evutil_socket_t, short, void*))
{
#ifdef _MSC_VER
	evutil_socket_t fds[2];
	if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0) {
		dzlog_error("evutil_socketpair is failed!");
		return false;
	}
	evutil_make_socket_nonblocking(fds[0]);
	evutil_make_socket_nonblocking(fds[1]);
#else
	int32_t fds[2];
	if (pipe(fds) != 0)
	{
		dzlog_error("Can't create notify pipe");
		return false;
	}
#endif
	pp.nReceiveFd = fds[0];
	pp.nSendFd    = fds[1];
	
	pp.pEvent = event_new(m_pBase, pp.nReceiveFd, EV_READ|EV_PERSIST, func, (void*)this);
	if(pp.pEvent == NULL)
		return false;
	
	if(event_add(pp.pEvent, NULL) == -1)
	{
		int32_t errorCode = EVUTIL_SOCKET_ERROR();
		dzlog_error("event_add error: %s", evutil_socket_error_to_string(errorCode));
		return false;
	}
	
	return true;
}

void CWorker::ReadPipeCb(evutil_socket_t fd, short event, void* arg)
{
	CWorker* pWorker = static_cast<CWorker*>(arg);
#ifdef _MSC_VER
	char buf[1];
	if (recv(fd, buf, 1, 0) == SOCKET_ERROR || buf[0] != PIPE_NOTIFY_CHR) {
		dzlog_error("Can't read from libevent socket pair.");
		return;
	}
#else
	/* read from master-thread had write, a byte 代表一个客户端连接 */
	char buf[1];
	if (read(fd, buf, 1) != 1 || buf[0] != PIPE_NOTIFY_CHR)
	{
		dzlog_error("Can't read from libevent pipe.");
		return;
	}
#endif
	while(pWorker->m_connList.GetCount() > 0)
	{
		/* 将主线程塞到队列中的连接pop出来 */

		CONN_INFO *pConnInfo = pWorker->m_connList.GetHeadAndRemove();
		if( !pConnInfo )
		{
			dzlog_error("get connInfo failed.");
			return;
		}

		/*初始化新连接，将连接事件注册入libevent */

		if( pConnInfo->sfd > 0)
		{
			CONN_INFO* conn = InitNewConn(pConnInfo, pWorker);
			if(NULL == conn)
			{
				dzlog_error("InitNewConn(%d) error.", pConnInfo->sfd);
				CloseConn(pConnInfo, NULL);
				continue;
			}
			
			pWorker->m_connMap[pConnInfo->sfd] = pConnInfo;
		}
	}

}

CONN_INFO* CWorker::InitNewConn(CONN_INFO *pConnInfo, CWorker* pWorker)
{
	/* 创建一个structbufferevent *bev，关联该sockfd，托管给event_base */
	//  | BEV_OPT_THREADSAFE
#ifdef _MSC_VER
	struct bufferevent *pClientTcpEvent = bufferevent_socket_new(pWorker->m_pBase, pConnInfo->sfd, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
#else
	struct bufferevent *pClientTcpEvent = bufferevent_socket_new(pWorker->m_pBase, pConnInfo->sfd, BEV_OPT_CLOSE_ON_FREE);
#endif
	if (NULL == pClientTcpEvent)
	{
		int32_t errorCode = EVUTIL_SOCKET_ERROR();
		dzlog_error("bufferevent_socket_new error: %s", evutil_socket_error_to_string(errorCode));
		return NULL;
	}

	WorkerAndConnInfo* pArg = new WorkerAndConnInfo;
	if (pArg==NULL)
	{
		dzlog_error("new WorkerAndConnInfo");
		bufferevent_free(pConnInfo->pClientTcpEvent);
		pConnInfo->pClientTcpEvent = NULL;
		return NULL;
	}

	pConnInfo->pClientTcpEvent = pClientTcpEvent;
	pConnInfo->pConnection     = pWorker->m_pParserFactory->GetConnection(pWorker->GetIndex(), pConnInfo->nAcceptTm );
	pArg->pWorker = pWorker;
	pArg->pConnInfo = pConnInfo;
	
	/*设置读写对应的回调函数*/

	bufferevent_setcb(pClientTcpEvent, ClientTcpReadCb, ClientTcpWriteCb, ClientTcpErrorCb, (void*)pArg);

	/* 利用客户端心跳超时机制处理半开连接 */
	//#ifdef USE_TIMEOUT
	if (pWorker->m_isUseTimeout)
	{
		struct timeval heartbeat_sec;
		heartbeat_sec.tv_sec = pWorker->m_nKeepaliveSec;
		heartbeat_sec.tv_usec= 0;
		bufferevent_set_timeouts(pClientTcpEvent, &heartbeat_sec, NULL);
	}
	//#endif
		
	int32_t flag = EV_READ | EV_PERSIST;
	/* 启用读写事件,其实是调用了event_add将相应读写事件加入事件监听队列poll。
	如果相应事件不置为true，bufferevent是不会读写数据的 */

	bufferevent_enable(pClientTcpEvent, flag);

	return pConnInfo;
}

void CWorker::ClientTcpReadCb(struct bufferevent *bev, void *arg)
{
	WorkerAndConnInfo* p = static_cast<WorkerAndConnInfo*>(arg);

# ifdef IPARSER_INSTANCE
	if( p->pConnInfo && p->pConnInfo->pConnection )
	{
		TwoVoidPtrParam param;
		param.p1 = (void*)bev;
		param.p2 = (void*)(p->pWorker);
		p->pConnInfo->pConnection->OnRead((void*)(&param) );
	}
# endif

}

void CWorker::ClientTcpWriteCb(struct bufferevent *bev, void *arg)
{
	WorkerAndConnInfo* p = static_cast<WorkerAndConnInfo*>(arg);

# ifdef IPARSER_INSTANCE
	if( p->pConnInfo && p->pConnInfo->pConnection )
	{
		TwoVoidPtrParam param;
		param.p1 = (void*)bev;
		param.p2 = (void*)(p->pWorker);
		p->pConnInfo->pConnection->OnWrite((void*)(&param) );
	}
# endif
}

void CWorker::ClientTcpErrorCb(struct bufferevent *bev, short event, void *arg)
{
	WorkerAndConnInfo* p = static_cast<WorkerAndConnInfo*>(arg);
	
# ifdef IPARSER_INSTANCE
	if( p->pConnInfo && p->pConnInfo->pConnection )
	{
		TwoVoidPtrParam param;
		param.p1 = (void*)bev;
		param.p2 = (void*)(p->pWorker);
		p->pConnInfo->pConnection->OnError((void*)(&param));
	}
# endif

	if (event & BEV_EVENT_TIMEOUT)
	{
		dzlog_warn("CTcpWorker::ClientTcpErrorCb:TimeOut.");
	}
	else if (event & BEV_EVENT_EOF)
	{
        if (p->pConnInfo)
		    dzlog_debug("ClientTcpErrorCb close(%d)", p->pConnInfo->sfd);
        else
            dzlog_debug("ClientTcpErrorCb close(-1)");
	}
	else if (event & BEV_EVENT_ERROR)
	{
		int32_t errorCode = EVUTIL_SOCKET_ERROR();
		dzlog_warn("CTcpWorker::ClientTcpErrorCb:some other error: %s", evutil_socket_error_to_string(errorCode));
	}

	CloseConn(p, bev);
}

void CWorker::CloseConn(WorkerAndConnInfo* p, struct bufferevent *bev)
{
	p->pWorker->m_connMap.erase(p->pConnInfo->sfd);
	CloseConn(p->pConnInfo, bev);
	p->pConnInfo = NULL;
	delete p;
}

void CWorker::CloseConn(CONN_INFO* pConnInfo, struct bufferevent *bev)
{
	if( pConnInfo->pClientTcpEvent && pConnInfo->pClientTcpEvent == bev)
	{
		bufferevent_free(pConnInfo->pClientTcpEvent);
		pConnInfo->pClientTcpEvent = NULL;
	}
	else
	{
		if (pConnInfo->sfd > 0)
		{
#ifdef _MSC_VER
        	closesocket(pConnInfo->sfd);
#else
        	close(pConnInfo->sfd);
#endif			
			
			pConnInfo->sfd = 0;
		}

		dzlog_error("CloseConn error: pConnInfo->pClientTcpEvent != bev");
	}

	if (pConnInfo->pConnection)
	{
		delete pConnInfo->pConnection;
		pConnInfo->pConnection = NULL;
	}
	
	FREE(pConnInfo);
}

void CWorker::ClientTcpErrorCbCaller(struct bufferevent *bev, short event)
{
    WorkerAndConnInfo* pArg = NULL;
    //bufferevent_getcb(bev, NULL, NULL, NULL, (void**)&pArg);
    ClientTcpErrorCb(bev, event, pArg);
    bufferevent_setcb(bev, NULL, NULL, NULL, NULL);
}

int32_t CWorker::Send(int32_t nFd, char* pBuffer, int32_t nLen)
{
	Map_ConnInfo::iterator itFind = m_connMap.find(nFd);
	if (itFind == m_connMap.end())
	{
		dzlog_error("Can not find fd=%d", nFd);
		return -1;
	}

	return bufferevent_write(itFind->second->pClientTcpEvent, pBuffer, nLen)==0?nLen:-1;
}

bool CWorker::ForceCloseConn(const int nClientFd)
{
	Map_ConnInfo::iterator itFind = m_connMap.find(nClientFd);
	if (itFind != m_connMap.end())
	{
		CONN_INFO *pConnInfo = itFind->second;
		if (pConnInfo)
		{
			dzlog_error("Force close fd=%d\n", itFind->second->sfd);
			ClientTcpErrorCbCaller(itFind->second->pClientTcpEvent, 0);
			return true;
		}
		else
		{
			dzlog_error("ptr null where fd=%d\n", nClientFd);
			return false;
		}
	}
	else
	{
		dzlog_error("Can not find fd=%d\n", nClientFd);
		return false;
	}
}
