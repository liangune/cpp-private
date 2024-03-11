/*************************************************************************
	> Date:   2020-9-22
	> Author: liangjun
	> Desc:   
*************************************************************************/
#ifndef _REDISCLUSTERCLIENT_H_
#define _REDISCLUSTERCLIENT_H_

#include "hiredis/hircluster.h"
#include "redisClient.h"

class CRedisClusterClient: public CRedisClient
{
 public:
 	CRedisClusterClient();
	~CRedisClusterClient();

	int connect(const string_t &sAddres = "127.0.0.1:6379", int flags = HIRCLUSTER_FLAG_NULL, long timeout = 1000*3, string_t password="");
	void close();
	bool formatQuery(const string_t &cmd);
	const char *strerror() const;
	bool HasError() const;
	// no support
	bool ping();
 protected:
	bool setTimeout(long seconds);
	bool setKeepAlive();
	bool query(unsigned char type, const char* fmt, ...);
	bool query(unsigned char type, int argc, const char **argv, const size_t *argvlen);
 protected:
	string_t m_sError;
};

#endif /* _REDISCLUSTERCLIENT_H_ */
