

#ifndef _REDISCLIENTPOLL_H_
#define _REDISCLIENTPOLL_H_

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <algorithm>

#ifdef _MSC_VER
#include <time.h>
#else 
#include <sys/time.h>
#endif

#include <thread>
#include <mutex>
#include "redisClientEx.h"
#include <stdint.h>

using namespace std;

class CRedisClientPool
{
public:
	using ListUseableIndex = std::list<int>;
	using MapUseableHost = std::map<string, ListUseableIndex>;
	using VecRedisClient = std::vector<CRedisClientEx*>;

public:
	static CRedisClientPool* getInstance();

	//取得一个可用连接
	CRedisClientEx* getClient(const string& ip = "127.0.0.1", int port = 6379, string passwd="");
	CRedisClientEx* getClusterClient(const string& strHost, string passwd="");
	//归还一个连接
	int returnClient(CRedisClientEx* pClient);

	//设置连接数量
	int setClientCnt(unsigned int nCnt);
	//设置超时关闭时间
	int setTimeout(unsigned int nTimeout);

	//取得当前时间戳
	unsigned long long getCurTimestamp();

	void test();

private:
	int connectRedis(CRedisClient* client, const string& ip, int port, string passwd="");
	int connectClusterRedis(CRedisClient* client, const string& sHost, string passwd="");

	string createHostString(const string& ip, int port);
	ListUseableIndex* getUseableList(const string& strHost);

	void ticker();
	void checkConnection();

private:
	CRedisClientPool();
	~CRedisClientPool();

private:
	static CRedisClientPool* m_pInstance;

	//默认连接数量
	static const uint32_t m_nDefaultClientCnt = 10;
	//实际最大的连接数量
	uint32_t m_nCurClientCnt;
	// 已使用的连接数量
	uint32_t m_nUsedClientCnt;
	//默认超时关闭时间(s)
	static const uint32_t m_nDefaultTimeout = 300;
	//当前的超时关闭时间(s)
	uint32_t m_nCurTimeout;
	//
	static const uint32_t m_checkInterval = 60;

	//可用连接下标集合
	MapUseableHost m_MapUseable;
	//可用连接集合
	VecRedisClient m_VecClient;

	std::mutex m_mutex;
	uint32_t m_nTick;
};
#endif	//_REDISCLIENTPOLL_H_
