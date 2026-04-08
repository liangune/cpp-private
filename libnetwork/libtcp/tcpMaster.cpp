/*************************************************************************
	> Date:   2020-12-22
	> Author: liangjun
	> Desc:   tcp master
*************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <cassert>
#include "event2/buffer.h"
#include "event2/util.h"
#include "event2/thread.h"
#include "zlog/zlog.h"
#include "connectionFactory.h"
#include "connection.h"
#include "tcpMaster.h"
#include "worker.h"
#include "typedef.h"
#include <chrono>
#include <thread>

#ifdef _MSC_VER
#include <winSock2.h>
#include <WS2tcpip.h>
#else
#include <netinet/tcp.h> // for TCP_NODELAY
#include <limits.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#endif

#include <chrono>

CTcpMaster::CTcpMaster() : m_pEventBase(NULL), m_pParserFactory(NULL), m_nThreadCount(0), m_nLastThread(0)
{
	m_isUseTimeout = false;
	m_isCheckKeepalive = false;
	m_nkeepaliveTimeout = 0; 
}

CTcpMaster::~CTcpMaster()
{
	this->Destroy();
}

void CTcpMaster::Destroy()
{
	m_acceptorClient.Free();
	zlog_fini();

#ifdef _MSC_VER
    WSACleanup();
#endif

	for(auto pWorker : m_vecWorker)
	{
		delete pWorker;
	}
	
	if (m_pEventBase) {
		event_base_free(m_pEventBase);
		m_pEventBase = NULL;
	}

	if (m_pEventTimer) {
		delete m_pEventTimer;
		m_pEventTimer = NULL;
	}
}

bool CTcpMaster::CheckLibeventVersion()
{
	const char * szLibeventVersion = event_get_version();
	if( szLibeventVersion == NULL)
		return false;
	dzlog_info("The libevent version is %s", szLibeventVersion);
	if( strncmp(szLibeventVersion, "2", 1) == 0 )
		return true;
	else
		return false;
}

bool CTcpMaster::InitMasterThread(IConnectionFactory* pParserFactory,\
								const char* szZlogConf, const char* szZlogCat,\
								int32_t nClientPort,\
								int32_t nWorkerCount,\
								int32_t nBacklog,\
								bool isUseTimeout,\
								int32_t nKeepaliveSec)
{
	m_isUseTimeout = isUseTimeout;
	m_nKeepaliveSec = nKeepaliveSec;

	//日志服务
	dzlog_init(szZlogConf, szZlogCat);

	//解析
	m_pParserFactory = pParserFactory;

	//版本
	if( !CheckLibeventVersion() )
	{
		dzlog_error("The server Require libevent 2.0.* version");
		return false;
	}

#ifdef _MSC_VER
	// Winsock init, 初始化socket库
	
	WSADATA wsa;
	int err = WSAStartup(MAKEWORD(2,2), &wsa);
    if (err != 0) {
        dzlog_error("WSAStartup failed with error: %d", err);
        return false;
    }
#else
    //忽略管道信号，防止发送数据给关闭的socket出现宕机
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return false;
#endif

#ifdef _MSC_VER
	event_config *conf = event_config_new();
	if (!conf) {
		dzlog_error("event_config_new is failed!");
		return false;
	}

	// windows iocp
	event_config_set_flag(conf, EVENT_BASE_FLAG_STARTUP_IOCP);
	evthread_use_windows_threads();

	// set cpu
	/*
	SYSTEM_INFO sinfo;
	GetSystemInfo(&sinfo);
	event_config_set_num_cpus_hint(conf, sinfo.dwNumberOfProcessors);
	*/
	m_pEventBase = event_base_new_with_config(conf);
	event_config_free(conf);
#else
	m_pEventBase = event_base_new();
#endif

	if(!m_pEventBase) {
		dzlog_error("event_base_new is failed!");
		return false;
	}
	
	// accept client, 接收客户端连接

	if(false==InitAcceptor(nClientPort, nBacklog)) {
		dzlog_error("InitAcceptor error");
		return false;
	}

	// timer
	if (m_isCheckKeepalive) {
		m_pEventTimer = new CEventTimer();
		m_pEventTimer->Setup();
	}
	
	/* worker thread, 创建worker线程，用来处理来自客户端的连接 */
	if(false==InitWorkerThread(nWorkerCount))
	{
		dzlog_error("InitWorkerThread error");
		return false;
	}

	return true;
}

bool CTcpMaster::InitAcceptor(int32_t nClientPort, int32_t nBacklog)
{
	AcceptorCbParam* pClientParam = new AcceptorCbParam();
	pClientParam->pObj = this;
	pClientParam->callback = &CTcpMaster::DispatchSfdToWorker;
	if (false == m_acceptorClient.Init(m_pEventBase, nClientPort, nBacklog, pClientParam) )
		return false;

	return true;
}

CWorker* CTcpMaster::CreateWorker()
{
	CWorker* p = new CWorker;
	return p;
}

bool CTcpMaster::InitWorkerThread(int32_t nThreadCount)
{
	if( nThreadCount < 1)
		nThreadCount = 1;

	for(int32_t i=0; i<nThreadCount; ++i)
	{
		CWorker* p = CreateWorker();

		if (p)
		{
			p->SetUseTimeout(m_isUseTimeout);
			p->SetKeepaliveSec(m_nKeepaliveSec);
			p->SetIndex(i);
			m_vecWorker.push_back(p);
		}
		else
		{
			dzlog_error("worker ptr null");
			return false;
		}
		
		if( !p->Setup() )
		{
			dzlog_error("SetupThread error.");
			return false;
		}

		if (!p->SetClientKeepaliveTimeout(m_isCheckKeepalive, m_nkeepaliveTimeout)) {
			dzlog_error("SetClientKeepaliveTimeout error.");
			return false;
		}
		if (m_pEventTimer && m_isCheckKeepalive) {
			m_pEventTimer->AddEvent(p->m_pipeCheckClientKeepalive, m_nkeepaliveTimeout);
		}
		
		p->m_pParserFactory = m_pParserFactory;
	}
	
	for(int32_t i=0; i<nThreadCount; ++i)
	{
		StartWorker(m_vecWorker[i]);
	}

	while(m_nThreadCount.load() != nThreadCount ) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return true;
}


void CTcpMaster::StartWorker(void *arg)
{
	std::thread th(&CTcpMaster::WorkRoutine, this, arg);
    th.detach();
}

void* CTcpMaster::WorkRoutine(void* param)
{
	CWorker* pWorker = static_cast<CWorker *>(param);
	pWorker->m_threadId = std::this_thread::get_id();
	
	m_nThreadCount++;

	event_base_dispatch(pWorker->m_pBase);
	return NULL;
}

bool CTcpMaster::DispatchSfdToWorker(evutil_socket_t sfd)
{

	if (evutil_make_socket_nonblocking(sfd) == -1)
	{
		int32_t errorCode = EVUTIL_SOCKET_ERROR();
		dzlog_error("AccepCb:set socket nonblocking error: %s", evutil_socket_error_to_string(errorCode));
#ifdef _MSC_VER
		closesocket(sfd);
#else 
		close(sfd);
#endif 
		return false;
	}

	int32_t enable = 1;
#ifdef _MSC_VER
	if (setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (char *)&enable, sizeof(int32_t)) != 0)
#else
	if (setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int32_t)) != 0)
	//if(setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int32_t)) == -1 )
#endif
	{
		dzlog_error("setsockopt error");
	}
	
	/* Round Robin*/
	int32_t tid = (m_nLastThread + 1) % m_nThreadCount.load();
	CWorker* pWorker = m_vecWorker[tid];
	m_nLastThread = tid;

	/* 将新连接的加入此worker线程连接队列 */

	CONN_INFO *pConnInfo = (CONN_INFO *)MALLOC(sizeof(CONN_INFO));
	if( !pConnInfo )
	{
#ifdef _MSC_VER
		closesocket(sfd);
#else 
		close(sfd);
#endif 
		return false;
	}
	memset(pConnInfo, 0, sizeof(CONN_INFO));
	pConnInfo->sfd             = sfd;
	pConnInfo->pConnection	   = NULL;
	pConnInfo->pClientTcpEvent = NULL;
	//tm
	int64_t nMS = GetNowMS();
	pConnInfo->nLastKeepalive = nMS/1000;
	pConnInfo->nAcceptTm = nMS;

	pWorker->m_connList.AddTail(pConnInfo);

	/* 通知此worker线程有新连接到来，可以读取了 */

	static char buf[] = {PIPE_NOTIFY_CHR};
#ifdef _MSC_VER
	if (send(pWorker->m_pipeNotify.nSendFd, buf, 1, 0) == SOCKET_ERROR) {
		dzlog_error("CTcpWorker::DispatchSfdToWorker:Send to thread notify socket fd failed.");
		CWorker::CloseConn(pConnInfo, NULL);
		return false;
	}
#else
	if (write(pWorker->m_pipeNotify.nSendFd, buf, 1) != 1)
	{
		dzlog_error("CTcpWorker::DispatchSfdToWorker:Writing to thread notify pipe failed.");
		CWorker::CloseConn(pConnInfo, NULL);
		return false;
	}
#endif
	
	return true;
}

void CTcpMaster::Start()
{
	dzlog_info("<<<<<<<<<<<<<<< Tcp Server start >>>>>>>>>>>>");
	if(m_pEventTimer && m_isCheckKeepalive) {
		m_pEventTimer->Start();
	}
	int32_t iRet = event_base_dispatch(m_acceptorClient.GetEventBase());
	if( iRet == -1)
	{
		int32_t errorCode = EVUTIL_SOCKET_ERROR();
		dzlog_error("event_base_dispatch error: %s", evutil_socket_error_to_string(errorCode));
		// exit(1);
		assert(false);
	}
	else if( iRet == 1)
	{
		dzlog_error("event_base_dispatch no events were registered.");
		// exit(1);
		assert(false);
	}
}

int64_t CTcpMaster::GetNowMS()
{
	auto now = std::chrono::system_clock::now();

	int64_t milliseconds = 0;
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	return milliseconds;
}

void CTcpMaster::SetClientKeepaliveTimeout(bool isCheckKeepalive, uint32_t nKeepaliveSec)
{
	m_isCheckKeepalive = isCheckKeepalive;
	m_nkeepaliveTimeout = nKeepaliveSec;
}

void CTcpMaster::Send(const uint32_t nWorkerIndex, const int32_t nFd, char* pBuffer, int32_t nLen)
{
	for (Vec_Worker::iterator it = m_vecWorker.begin(); it != m_vecWorker.end(); ++it)
    {
        CWorker *pWorker = (CWorker *)(*it);
        if (!pWorker)
        {
            dzlog_error("worker is nullptr, worker index: %d", nWorkerIndex);
            continue;
        }

        uint32_t nIndex = pWorker->GetIndex();
        if (nIndex != nWorkerIndex)
            continue;
		
		if (pWorker->Send(nFd, pBuffer, nLen) == -1)
		{
			dzlog_error("SendToClient:Send failed.");
		}
	}
}