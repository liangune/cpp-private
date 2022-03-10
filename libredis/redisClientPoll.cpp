#include "redisClientPoll.h"
#include "hiredis/win32.h"

CRedisClientPool* CRedisClientPool::m_pInstance = NULL;

CRedisClientPool* CRedisClientPool::getInstance()
{
	if (NULL==m_pInstance)
	{
		m_pInstance = new CRedisClientPool();
		std::cout<<"CRedisClientPool::GetInstance, new instance!"<<std::endl;
	}
	return m_pInstance;
}

CRedisClientPool::CRedisClientPool(): m_nUsedClientCnt(0)
{
	m_nCurClientCnt = m_nDefaultClientCnt;
	m_nCurTimeout = m_nDefaultTimeout;
	m_nTick = 0;
	m_MapUseable.clear();
	m_VecClient.clear();

	ListUseableIndex listIndex;
	for (unsigned int i=0; i<m_nCurClientCnt; i++)
	{
		listIndex.push_back(i);
		m_VecClient.push_back(NULL);
	}
	m_MapUseable["NULL"] = listIndex;

	std::thread th(&CRedisClientPool::ticker, std::ref(*this));
    th.detach();
}

CRedisClientPool::~CRedisClientPool()
{
	for (VecRedisClient::iterator iter=m_VecClient.begin(); iter!=m_VecClient.end(); iter++)
	{
		CRedisClientEx* p = *iter;
		if (p)
		{
			delete p;
		}
	}
}

void CRedisClientPool::checkConnection()
{
	m_mutex.lock();

	//规定时间没用的连接关闭掉
	for (VecRedisClient::iterator iter=m_VecClient.begin(); iter!=m_VecClient.end(); iter++)
	{
		unsigned long long nCurTime = getCurTimestamp();
		CRedisClientEx* pClient = *iter;
		if ( (pClient && !pClient->m_Used && nCurTime-pClient->m_Timestamp>m_nCurTimeout) || (pClient && !pClient->m_Used && pClient->getClient() && pClient->getClient()->ping()==false) )
		{
			//std::cout<<"redis-conn lose!!!!!!!! "<<nCurTime-pClient->m_Timestamp<<" "<<pClient->m_Host<<std::endl;
			MapUseableHost::iterator itFind = m_MapUseable.find(pClient->m_Host);
			if (itFind != m_MapUseable.end())
			{
				itFind->second.remove(pClient->m_Index);
				if( itFind->second.empty() )
				{
					m_MapUseable.erase(itFind);
				}
			}

			itFind = m_MapUseable.find("NULL");
			if (itFind != m_MapUseable.end())
				itFind->second.push_back(pClient->m_Index);

			delete pClient;
			pClient = NULL;
			*iter = NULL;
			--m_nUsedClientCnt;
		}
	}


	if (m_VecClient.size() > m_nCurClientCnt)
	{
		bool bPopRes = false;
		do
		{
			bPopRes = false;
			CRedisClientEx* pClient = m_VecClient.at(m_VecClient.size()-1);
			if (NULL==pClient || (pClient && !pClient->m_Used) )
			{
				string strHost;
				if (pClient)
				{
					strHost = pClient->m_Host;
					delete pClient;
					pClient = NULL;
					--m_nUsedClientCnt;
				}
				else
					strHost = "NULL";


				MapUseableHost::iterator itFind = m_MapUseable.find(strHost);
				if (itFind != m_MapUseable.end())
				{
					itFind->second.remove(m_VecClient.size()-1);
					if( itFind->second.empty() )
					{
						m_MapUseable.erase(itFind);
					}
				}
				m_VecClient.pop_back();
				bPopRes = true;
			}
		} while(bPopRes && (m_VecClient.size()>m_nCurClientCnt) );
	}

	m_mutex.unlock();
}

unsigned long long CRedisClientPool::getCurTimestamp()
{
	/*
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000 + tv.tv_usec/1000;
	*/
	return time(NULL);
}

int CRedisClientPool::connectRedis(CRedisClient* client, const string& ip, int port, string passwd)
{
	if (0 != client->connect(ip, port, 10*1000, passwd))
    {
        return -1;
   	}
	return 0;
}

int CRedisClientPool::connectClusterRedis(CRedisClient* client, const string& sHost, string passwd)
{
	if (0 != client->connect(sHost, HIRCLUSTER_FLAG_NULL, 10*1000, passwd))
    {
        return -1;
   	}
	return 0;
}


CRedisClientEx* CRedisClientPool::getClient(const string& ip, int port, string passwd)
{
	string strHost = createHostString(ip, port);

	m_mutex.lock();

	ListUseableIndex* listIndex = getUseableList(strHost);
	if (NULL==listIndex || (listIndex && listIndex->empty()) )
	{
		std::cout<<"CRedisClientPool::GetClient, no client can use!"<<std::endl;
		m_mutex.unlock();
		return NULL;
	}

	CRedisClientEx* pUseableClient = NULL;
	try
	{
		int nUseableIndex = listIndex->front();
		pUseableClient = m_VecClient.at(nUseableIndex);
		if (NULL==pUseableClient)
		{
			pUseableClient = new CRedisClientEx();
			if (0 != connectRedis(pUseableClient->m_Client, ip, port, passwd))
			{
				std::cout<<"CRedisClientPool::GetClient, connect to redis failed!"<<std::endl;
				m_mutex.unlock();
				return NULL;
			}
			pUseableClient->m_Timestamp = getCurTimestamp();
			pUseableClient->m_Host = strHost;
			m_VecClient.at(nUseableIndex) = pUseableClient;
			++m_nUsedClientCnt;
		}

		pUseableClient->m_Index = nUseableIndex;
		pUseableClient->m_Used = true;
		listIndex->pop_front();

		if( listIndex->empty() )
		{
			m_MapUseable.erase(strHost);
		}
	}
	catch(...)
	{
		m_mutex.unlock();
		return NULL;
	}

	m_mutex.unlock();

	return pUseableClient;
}

CRedisClientEx* CRedisClientPool::getClusterClient(const string& strHost, string passwd)
{
	m_mutex.lock();

	ListUseableIndex* listIndex = getUseableList(strHost);
	if (NULL==listIndex || (listIndex && listIndex->empty()) )
	{
		std::cout<<"CRedisClientPool::GetClient, no client can use!"<<std::endl;
		m_mutex.unlock();
		return NULL;
	}

	CRedisClientEx* pUseableClient = NULL;
	try
	{
		int nUseableIndex = listIndex->front();
		pUseableClient = m_VecClient.at(nUseableIndex);
		if (NULL==pUseableClient)
		{
			pUseableClient = new CRedisClientEx(true);
			if (0 != connectClusterRedis(pUseableClient->m_Client, strHost, passwd))
			{
				std::cout<<"CRedisClientPool::GetClusterClient, connect to redis failed!"<<std::endl;
				m_mutex.unlock();
				return NULL;
			}
			pUseableClient->m_Timestamp = getCurTimestamp();
			pUseableClient->m_Host = strHost;
			m_VecClient.at(nUseableIndex) = pUseableClient;
			++m_nUsedClientCnt;
		}

		pUseableClient->m_Index = nUseableIndex;
		pUseableClient->m_Used = true;
		listIndex->pop_front();

		if( listIndex->empty() )
		{
			m_MapUseable.erase(strHost);
		}
	}
	catch(...)
	{
		m_mutex.unlock();
		return NULL;
	}

	m_mutex.unlock();

	return pUseableClient;
}

int CRedisClientPool::returnClient(CRedisClientEx* pClient)
{
	if (NULL == pClient) {
		return -1;
	}
	
	m_mutex.lock();

	pClient->m_Used = false;
	pClient->m_Timestamp = getCurTimestamp();

	MapUseableHost::iterator itFind = m_MapUseable.find(pClient->m_Host);
	if (itFind != m_MapUseable.end())
	{
		itFind->second.push_back(pClient->m_Index);
	}
	else
	{
		ListUseableIndex listIndex;
		listIndex.push_back(pClient->m_Index);
		m_MapUseable[pClient->m_Host] = listIndex;
	}

	m_mutex.unlock();

	return 0;
}


int CRedisClientPool::setClientCnt(unsigned int nCnt)
{
	if (nCnt == m_nCurClientCnt) {
		return 0;
	}

	m_mutex.lock();

	if (nCnt > m_nCurClientCnt)
	{
		for(unsigned int i=m_nCurClientCnt-1; i<nCnt; i++)
		{
			MapUseableHost::iterator itFind = m_MapUseable.find("NULL");
			if (itFind != m_MapUseable.end())
			{
				ListUseableIndex::iterator itList = find(itFind->second.begin(), itFind->second.end(), i);
				if (itList == itFind->second.end())
					itFind->second.push_back(i);
			}
		}

		int nTmp = nCnt - m_VecClient.size();
		for (int i=0; i<nTmp; i++)
		{
			m_VecClient.push_back(NULL);
		}
	}
	else if (nCnt < m_nCurClientCnt)
	{
		if (m_VecClient.size() > nCnt)
		{
			bool bPopRes = false;
			do
			{
				bPopRes = false;
				CRedisClientEx* pClient = m_VecClient.at(m_VecClient.size()-1);
				if (NULL==pClient || (pClient && !pClient->m_Used) )
				{
					string strHost;
					if (pClient)
					{
						strHost = pClient->m_Host;
						delete pClient;
						pClient = NULL;
					}
					else
						strHost = "NULL";

					MapUseableHost::iterator itFind = m_MapUseable.find(strHost);
					if (itFind != m_MapUseable.end())
					{
						itFind->second.remove(m_VecClient.size()-1);
						if( itFind->second.empty() )
						{
							m_MapUseable.erase(itFind);
						}
					}
					m_VecClient.pop_back();
					bPopRes = true;
				}
			} while(bPopRes && (m_VecClient.size()>nCnt) );
		}
	}

	m_nCurClientCnt = nCnt;

	m_mutex.unlock();

	return 0;
}

int CRedisClientPool::setTimeout(unsigned int nTimeout)
{
	if (0==nTimeout || nTimeout==m_nCurTimeout)
		return 0;

	m_mutex.lock();
	m_nCurTimeout = nTimeout;
	m_mutex.unlock();

	return 0;
}

string CRedisClientPool::createHostString(const string& ip, int port)
{
	string strHost;
	char strPort[10] = {0};
	sprintf(strPort, "%d", port);
	strHost = ip + ":" + string(strPort);
	return strHost;
}

CRedisClientPool::ListUseableIndex* CRedisClientPool::getUseableList(const string& strHost)
{
	MapUseableHost::iterator itFind	= m_MapUseable.find(strHost);
	if (itFind == m_MapUseable.end())
	{
		MapUseableHost::iterator itNull = m_MapUseable.find("NULL");
		if (itNull != m_MapUseable.end())
			return &(itNull->second);
	}
	else
	{
		return &(itFind->second);
	}

	return NULL;
}

void CRedisClientPool::ticker()
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

void CRedisClientPool::test()
{
	m_mutex.lock();

	std::cout<<"CRedisClientPool::Test, vector size="<<m_VecClient.size()<<std::endl;
	for (VecRedisClient::iterator iter=m_VecClient.begin(); iter!=m_VecClient.end(); iter++)
	{
		CRedisClientEx* p = *iter;
		if (p)
			std::cout<<"CRedisClientPool::Test, index="<<p->m_Index<<", used="<<p->m_Used<<", host="<<p->m_Host<<std::endl;
	}

	for (MapUseableHost::iterator iter=m_MapUseable.begin(); iter!=m_MapUseable.end(); iter++)
	{
		cout<<"CRedisClientPool::Test, host="<<iter->first<<", id=";
		for (ListUseableIndex::iterator it=iter->second.begin(); it!=iter->second.end(); it++)
		{
			cout<<*it<<",";
		}
		cout<<endl;
	}
	m_mutex.unlock();
}