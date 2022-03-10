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
};
#endif /* _TCP_MASTER_H_ */
