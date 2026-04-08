/*************************************************************************
	> Date:   2020-12-22
	> Author: liangjun
	> Desc:   tcp master
*************************************************************************/

#ifndef _LIB_TCP_MASTER_H_
#define _LIB_TCP_MASTER_H_

#include "event2/event.h"
#include "zlog/zlog.h"
#include "acceptor.h"
#include <stdint.h>
#include <atomic>
#include <vector>
#include "eventTimer.h"

class IConnectionFactory;
class CWorker;

class CTcpMaster
{
public:
	typedef std::vector<CWorker*> Vec_Worker;
public:
	CTcpMaster();
	virtual ~CTcpMaster();

	virtual bool InitMasterThread(IConnectionFactory* pParserFactory,\
								const char* szZlogConf, const char* szZlogCat,\
								int32_t nClientPort,\
								int32_t nWorkerCount,\
								int32_t nBacklog,\
								bool isUseTimeout=false,\
								int32_t nKeepaliveSec=30);
								
	virtual bool InitAcceptor(int32_t nClientPort, int32_t nBacklog);
	virtual bool InitWorkerThread(int32_t nThreadCount);
	virtual CWorker* CreateWorker();

	void Start();

	int64_t GetNowMS();

	void SetClientKeepaliveTimeout(bool isCheckKeepalive, uint32_t nKeepaliveSec = SYSTEM_LOSE_AVAILABILITY_ROUND * GATEWAY_CHECK_CLIENT_KEEPALIVE);
	
	void Send(const uint32_t nWorkerIndex, const int32_t nFd, char* pBuffer, int32_t nLen);

protected:
	bool CheckLibeventVersion();
	void Destroy();
	void StartWorker(void*arg);
	bool DispatchSfdToWorker(evutil_socket_t sfd);

	void* WorkRoutine(void* param);

public:
	struct event_base*  m_pEventBase;
	CAcceptor m_acceptorClient;
	
	//解析
	IConnectionFactory*	m_pParserFactory;
	//工作线程
	Vec_Worker m_vecWorker;
	
	std::atomic<int32_t> m_nThreadCount;
	int32_t m_nLastThread;

	//超时关闭连接
	bool m_isUseTimeout;
	uint32_t m_nKeepaliveSec;

	//定时检查客户端连接keepalive
	bool m_isCheckKeepalive;
	uint32_t m_nkeepaliveTimeout;
	CEventTimer *m_pEventTimer;
};
#endif /* _TCP_MASTER_H_ */
