
#include <iostream>
#include "libredis/redisClientPoll.h"
#include "libredis/redisClientWrap.h"
#include <string>
#include <stdint.h>

using namespace std;

#define GetRedisClient(pRedis)	\
	std::string sRedisHost = "192.168.74.232:7001,192.168.74.232:7002,192.168.74.232:7003";	\
	int32_t nRedisPort = 0;     \
    bool isCluster = false;     \
	if (!isCluster)             \
	{ \
		sRedisHost = "192.168.74.232";	\
		nRedisPort = 6379;    \
	} \
	CRedisClientWrap pRedis(sRedisHost, nRedisPort, isCluster);

int main(int argc, const char** argv) {
    CRedisClientPool::getInstance()->setClientCnt(100);
    GetRedisClient(pRedis)

    if (!pRedis.valid()) 
    { 
        cout << "redis ptr invalid\n"; 
        //return -1; 
    }

    CRedisClient *client = pRedis.getClient();
    if(client == NULL) {
        cout << "redis ptr invalid\n"; 
    } 
	else if(client->m_ctx.pCluster == NULL) {
        cout << "client->m_ctx.pCluster NULL\n"; 
    } else  {
		/*
		cout << client->m_ctx.pCluster->errstr << endl;

        cout << client->m_ctx.pCluster->ip  << endl;
        cout << client->m_ctx.pCluster->port << endl;
        cout << client->m_ctx.pCluster->connection_type << endl;
		*/
    }

    cout << pRedis->set("testkey", "liangjun") << endl;
	CRedisClientPool::getInstance()->test();

    while(1) {}
    return 0;
}