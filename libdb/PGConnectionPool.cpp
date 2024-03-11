/*************************************************************************
	> Date:   2020-12-09
	> Author: liangjun
	> Desc:   PG数据库连接池
*************************************************************************/
#include "PGConnectionPool.h"

CPGConnectionPool *CPGConnectionPool::m_pInstance = NULL;

CPGConnectionPool *CPGConnectionPool::getInstance()
{
    if(m_pInstance == NULL) {
        m_pInstance = new CPGConnectionPool();
    }

    return m_pInstance;
}

CPGConnectionPool::CPGConnectionPool()
{
	m_nMaxConnectionCnt = m_nDefaultConnectionCnt;
	m_nTimeout = m_nDefaultTimeout; 
    for(uint32_t i = 0; i < m_nMaxConnectionCnt; i++)
    {
        m_vecConnPool.push_back(NULL);
        m_listUseableConn.push_back(i);
    }

    std::thread th(&CPGConnectionPool::ticker, std::ref(*this));
    th.detach();
}

CPGConnectionPool::~CPGConnectionPool()
{
	VecConnectionPool::iterator iter = m_vecConnPool.begin();
	for(; iter != m_vecConnPool.end(); ++iter)
	{
		ConnItem *pItem = *iter;
		if( pItem && (pItem->eState != eConn_Invalid) )
		{
			pItem->PGClient.close();
			delete pItem;
			*iter = NULL;
		}
	}
}

void CPGConnectionPool::setMaxConnectionCnt(uint32_t nCount)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if(nCount > m_nMaxConnectionCnt) {
        for(uint32_t i = m_nMaxConnectionCnt; i < nCount; i++)
        {
            m_vecConnPool.push_back(NULL);
            m_listUseableConn.push_back(i);
        }
    }
}

void CPGConnectionPool::setTimeout(int nTimeout)
{
    m_nTimeout = nTimeout;
}

CPGConnectionPool::Connection CPGConnectionPool::getConnection(std::string &host, std::string &port, std::string &username, std::string &password, std::string &dbName, uint32_t nTimeout, uint32_t SSL)
{
    std::string sUrl = "postgresql://" + host + ":" + port + "/" + dbName +"?" + "user=" + username + "&password=" + password;
    std::lock_guard<std::mutex> guard(m_mutex);

	Connection conn;
    MapUseabledConnection::iterator iter = m_mapUseableConn.find(sUrl);
    if(iter != m_mapUseableConn.end()) {
        ListUseabledConnection *pUseableConnList = &(iter->second);
        if(pUseableConnList->empty()) {
			if(m_listUseableConn.empty()) {
				conn.pConnItem = nullptr;
				conn.sError = "PG Connection Pool has not idle connection.";
				return conn;
			}
		} else {
            int index = pUseableConnList->front();
            pUseableConnList->pop_front();
            ConnItem *pItem = m_vecConnPool[index];
            pItem->eState = eConn_Used;
			++m_nUsedConnectionCnt;
			conn.pConnItem = pItem;
			conn.sError = "";
			conn.sConnectionURL = pItem->PGClient.getConnectionURL();
            return conn;
        }
    } else if(m_listUseableConn.empty()) {
        conn.pConnItem = nullptr;
		conn.sError = "PG Connection Pool has not idle connection.";
		return conn;
    }

    int index = m_listUseableConn.front();
    if(!m_vecConnPool[index])
	{
		ConnItem *pItem = new ConnItem;
		pItem->sKey              = sUrl;
		pItem->nLastAccessedTime = time(NULL);
		pItem->nConnectingTime   = time(NULL);
		pItem->nPoolIndex        = index;
		pItem->sUrl              = sUrl;
		//
		if(nTimeout > CPGClient::m_nDefaultTimeout) {
			pItem->PGClient.setTimeout(nTimeout);
		}
		if( !pItem->PGClient.connect(host, port, username, password, dbName, SSL))
		{
			conn.pConnItem = nullptr;
			conn.sError = pItem->PGClient.getErrorMessage();
			conn.sConnectionURL = pItem->PGClient.getConnectionURL();
			delete pItem;
			return conn;
		}

		m_listUseableConn.pop_front();
		pItem->eState = eConn_Used;

		m_vecConnPool[index] = pItem;
		++m_nUsedConnectionCnt;

		conn.pConnItem = pItem;
		conn.sError = "";
		conn.sConnectionURL = pItem->PGClient.getConnectionURL();
        return conn;
	}

    conn.pConnItem = nullptr;
	conn.sError = "PG Connection Pool has not idle connection.";
	return conn;
}

void CPGConnectionPool::returnConnection(ConnItem *pConnItem)
{    
    if(!pConnItem)
		return;
    
	std::lock_guard<std::mutex> guard(m_mutex);

	std::string sKey = pConnItem->sKey;
	int index = pConnItem->nPoolIndex;
	pConnItem->nLastAccessedTime = time(NULL);
	pConnItem->eState  = eConn_Unused;
	--m_nUsedConnectionCnt;
	
	if (!pConnItem->PGClient.getStatus()) {
		removeConnection(pConnItem);
		return;
	}

	MapUseabledConnection::iterator iter = m_mapUseableConn.find(sKey);
	if(iter != m_mapUseableConn.end())
	{
		iter->second.push_back(index);
	}
	else
	{
		m_mapUseableConn[sKey].push_back(index);
	}
}

void CPGConnectionPool::removeConnection(ConnItem *pConnItem)
{
	if(!pConnItem)
		return;

    std::string sKey = pConnItem->sKey;
	int nPoolIndex = pConnItem->nPoolIndex;
	MapUseabledConnection::iterator iter = m_mapUseableConn.find(sKey);
	if( iter != m_mapUseableConn.end() )
	{
		iter->second.remove(nPoolIndex);
	}
	pConnItem->PGClient.close();
	m_listUseableConn.push_back(nPoolIndex);
	delete pConnItem;
    pConnItem = NULL;
}

void CPGConnectionPool::checkConnection()
{
    std::lock_guard<std::mutex> guard(m_mutex);
	VecConnectionPool::iterator iter = m_vecConnPool.begin();
	for(; iter != m_vecConnPool.end(); ++iter)
	{
		ConnItem *pItem = *iter;
		if( pItem && (pItem->eState == eConn_Unused))
		{
			uint32_t nNowTime = time(NULL);
			if( (nNowTime - pItem->nLastAccessedTime) > m_nTimeout )
			{
				removeConnection(pItem);
				*iter = NULL;
			}
			else if(!pItem->PGClient.ping() || !pItem->PGClient.getStatus())
			{
				removeConnection(pItem);
				*iter = NULL;
			}
		}
	}
}

void CPGConnectionPool::ticker()
{
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        m_nTick++;
        if(m_nTick % m_checkInterval == 0)
        {
            checkConnection();
        }
    } 
}