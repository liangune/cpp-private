
#ifndef _REDISCLIENTWRAP_H_
#define _REDISCLIENTWRAP_H_

#include "redisClientEx.h"
#include "redisClientPoll.h"

class CRedisClientWrap
{
public: 
	CRedisClientWrap()=default;
    CRedisClientWrap(const CRedisClientWrap&)=delete;
    CRedisClientWrap& operator=(const CRedisClientWrap&)=delete;

public:
	CRedisClientWrap(std::string ip, int port, bool bCluster=false, std::string passwd=""):m_pClient(NULL)
	{
		if (false==bCluster)
			m_pClient =  CRedisClientPool::getInstance()->getClient(ip, port, passwd);
		else
			m_pClient =  CRedisClientPool::getInstance()->getClusterClient(ip, passwd);
	}

	~CRedisClientWrap()
	{
		if (m_pClient)
		{
			CRedisClientPool::getInstance()->returnClient(m_pClient);
		}
	}

	CRedisClient* getClient()
	{
		if( m_pClient )
		{
			return m_pClient->getClient();
		}
		return NULL;
	}
	bool valid()const
	{
		return m_pClient != NULL;
	}
	CRedisClient *operator->()
	{
		return m_pClient->getClient();
	}

private:
	CRedisClientEx* m_pClient;
};

#endif //_REDISCLIENTWRAP_H_

