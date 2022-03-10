
#ifndef _REDISCLIENTEX_H_
#define _REDISCLIENTEX_H_

#include <string>
#include "redisClient.h"
#include "redisClusterClient.h"

class CRedisClientEx
{
	friend class CRedisClientPool;
public:
    CRedisClientEx(const CRedisClientEx&)=delete;
    CRedisClientEx& operator=(const CRedisClientEx&)=delete;

private:
        CRedisClientEx(bool bCluster=false)
		{
			m_Used = false;
			m_Timestamp = 0;

			if (false==bCluster)
			{
				m_Client = new CRedisClient;
			}
			else
			{
				m_Client = new CRedisClusterClient;
			}
		};
		
		~CRedisClientEx()
		{
			delete m_Client;
		}

        bool m_Used;
        uint64_t m_Timestamp;
        int m_Index;
        std::string m_Host;
        CRedisClient* m_Client;
public:
        CRedisClient* getClient()
        {
			return m_Client;
        }
};
#endif	//_REDISCLIENTEX_H_
