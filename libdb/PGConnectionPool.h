/*************************************************************************
	> Date:   2020-12-09
	> Author: liangjun
	> Desc:   PG数据库连接池
*************************************************************************/

#ifndef _PG_CONNECTION_POOL_H_
#define _PG_CONNECTION_POOL_H_

#include <thread>
#include <mutex>
#include <vector>
#include <map>
#include <list>
#include "PGClient.h"

class CPGConnectionPool
{
public:
    typedef enum {
		eConn_Invalid = 0,
		eConn_Unused  = 1,
		eConn_Used    = 2,
	}ConnState;

	struct ConnItem {
		std::string sKey;
		CPGClient PGClient;
		ConnState eState; 
		uint64_t nLastAccessedTime;
		uint64_t nConnectingTime;
		int nPoolIndex;
		std::string sUrl;
	};

    typedef std::vector<ConnItem*> VecConnectionPool;
	typedef std::list<int> ListUseabledConnection;
	typedef std::map<std::string, ListUseabledConnection > MapUseabledConnection;

private:
    CPGConnectionPool();
    ~CPGConnectionPool();

public:
    static CPGConnectionPool *getInstance();
    ConnItem *getConnection(std::string &host, std::string &port, std::string &username, std::string &password, std::string &dbName, uint32_t nTimeout = CPGClient::m_nDefaultTimeout);
    
    void setMaxConnectionCnt(uint32_t nCount);
	//设置超时关闭时间
	void setTimeout(int nTimeout);
	void returnConnection(ConnItem *pConnItem);
	void removeConnection(ConnItem *pConnItem);

private:
    void checkConnection();
    void ticker();

private:
    static CPGConnectionPool *m_pInstance;

    static const uint32_t m_nDefaultTimeout = 60;     // 60s
    static const uint32_t m_nDefaultConnectionCnt = 100;
    static const uint32_t m_checkInterval = 30;

    uint32_t m_nTick;
    uint32_t m_nTimeout;
    uint32_t m_nMaxConnectionCnt;
	uint32_t m_nUsedConnectionCnt;

    std::mutex m_mutex;

	VecConnectionPool     m_vecConnPool;
	MapUseabledConnection m_mapUseableConn;
	ListUseabledConnection m_listUseableConn;
};

#endif // !_PG_CONNECTION_POOL_H_