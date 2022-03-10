/*************************************************************************
	> Date:   2020-12-22
	> Author: liangjun
	> Desc:   工作线程
*************************************************************************/

#ifndef _LIB_TCP_WORKER_H_
#define _LIB_TCP_WORKER_H_

#include "typedef.h"
#include "event2/event.h"
#include "event2/bufferevent.h"
#include "syncList.h"
#include "connectionFactory.h"
#include <thread>

/* 利用客户端心跳超时机制处理半开连接 s */
//#define USE_TIMEOUT
#define HEARTBEAT_SEC      30

#ifndef IPARSER_INSTANCE
#define IPARSER_INSTANCE
#endif

class CWorker
{
friend class CTcpMaster;

public:
typedef struct 
{
	CWorker* pWorker;
	CONN_INFO* pConnInfo;
}WorkerAndConnInfo;

public:
	CWorker();
	~CWorker();
	void Destroy();

	virtual bool Setup();

	Map_ConnInfo& GetConnMap() { return m_connMap; };
	int32_t Send(int32_t nFd, char* pBuffer, int32_t nLen);

	void SetUseTimeout(bool isUseTimeout);
	void SetKeepaliveSec(uint32_t nSec);

	void SetIndex(uint32_t nIndex);
	uint32_t GetIndex();
	void SetWorkerThreadId(std::thread::id pid);
	std::thread::id GetWorkerThreadId();
	
	struct event_base* GetEventBase() { return m_pBase; }

	bool ForceCloseConn(const int nClientFd);
protected:
	bool SetupPipe(PIPE& pp, void (*func)(evutil_socket_t, short, void*));
	
protected:
	static void ReadPipeCb(evutil_socket_t fd, short event, void* arg);
	
	static CONN_INFO* InitNewConn(CONN_INFO *pConnInfo, CWorker* pWorker);
	
	static void ClientTcpReadCb(struct bufferevent *bev, void *arg);
	static void ClientTcpWriteCb(struct bufferevent *bev, void *arg);
	static void ClientTcpErrorCb(struct bufferevent *bev, short event, void *arg);
	static void ClientTcpErrorCbCaller(struct bufferevent *bev, short event);
	static void CloseConn(CONN_INFO* pConnInfo, struct bufferevent *bev);
	static void CloseConn(WorkerAndConnInfo* p, struct bufferevent *bev);
	
protected:
	std::thread::id            m_threadId;
	struct event_base*			m_pBase;

	PIPE						m_pipeNotify;

	CSyncList<CONN_INFO>	    m_connList;
	Map_ConnInfo				m_connMap;

	IConnectionFactory*		    m_pParserFactory;

	//超时关闭连接
	bool m_isUseTimeout;
	uint32_t m_nKeepaliveSec;

	uint32_t m_nIndex;
};

#endif //_TCP_WORKER_H_