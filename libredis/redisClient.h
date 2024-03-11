/*************************************************************************
	> Date:   2020-9-22
	> Author: liangjun
	> Desc:   基于hredis的client
*************************************************************************/

#ifndef _REDISCLIENT_H_
#define _REDISCLIENT_H_

#include "hiredis/hiredis.h"
#include "typedef.h"

struct redisReply;
struct redisContext;
struct redisClusterContext;

class CRedisClient
{
 public:
	CRedisClient();
	virtual ~CRedisClient();
    virtual int connect(const string_t &ip = "127.0.0.1", int port = 6379, long timeout = 1000*3, string_t passwd="");
    virtual void close();
	bool setTimeout(long seconds);
	bool setKeepAlive();
	virtual bool ping();


	bool query(const char* fmt, ...);
	// 删除匹配的keys“keys_*”
	bool del_match_keys(const string_t &match_key);

	// key
	redis_integer_t del(const string_t &key);
	redis_integer_t ttl(const string_t &key);
	redis_integer_t pttl(const string_t &key);
	redis_integer_t exists(const string_t &key);
	redis_integer_t expire(const string_t &key, unsigned int seconds);
	redis_integer_t pexpire(const string_t &key, unsigned int ms);
	redis_array_t keys(const string_t &key);
	bool rename(const string_t &key, const string_t &newkey);
	
	// string
	string_t get(const string_t &key);
	redis_integer_t incr(const string_t &key);
	redis_integer_t incrby(const string_t &key, redis_integer_t nIncr);
	redis_integer_t decr(const string_t &key);
	redis_integer_t decrby(const string_t &key, redis_integer_t nIncr);
	bool set(const string_t &key, const string_t &val);
	bool setex(const string_t& key, const string_t& val, redis_integer_t expire);
	redis_integer_t setnx(const string_t &key, const string_t &val);
	bool setnx_ex(const string_t& key, const string_t& val, redis_integer_t expire=0);
	string_t getset(const string_t &key, const string_t &val);

	// set
	redis_integer_t sadd(const string_t &key, const string_t &member);
	redis_integer_t srem(const string_t &key, const string_t &member);
	redis_integer_t sismember(const string_t &key, const string_t &member);
	redis_array_t smembers(const string_t &key);
	bool smembers(const string_t &key, redis_array_t& arrOut);
	bool smembers(const string_t &key, redis_set_t& arrOut);
    size_t scard(const string_t &key);
    redis_array_t srandmember(const string_t &key, unsigned int nCnt = 1);
	redis_integer_t sinterstore(const string_t &des, const string_t &key1, const string_t &key2);

    redis_integer_t sdiffstore(const string_t &sDstSet, const string_t &sSet1, const string_t &sSet2);
    redis_integer_t sdiffstore(const string_t &sDstSet, const string_t &sSet1, const string_t &sSet2, const string_t &sSet3);
	string_t spop(const string_t &key);

	// hash
	redis_integer_t hset(const string_t &key, const string_t &field, const string_t &value);
	string_t hget(const string_t &key, const string_t &field);
	redis_integer_t hdel(const string_t &key, const string_t &filed);
	redis_integer_t hexists(const string_t &key, const string_t &field);
	redis_array_t hkeys(const string_t &key);
	void hkeys(const string_t &key, redis_set_t &redis_set);
	void hkeysLess(string_t key, int64_t now, std::set<int64_t> &sKeySet);
	void hkeysBigger(string_t key, int64_t now, std::set<int64_t> &sKeySet);
	redis_array_t hvals(const string_t &key);
	void hvals(const string_t &key /*IN*/, redis_array_t &redis_array /*OUT*/);
	redis_array_t hgetall(const string_t &key);
	bool hgetall(const string_t &key, redis_map_t &mapOut);
	bool hgetall(const string_t &key, redis_map_int_str_t &mapOut);
	bool hgetall(const string_t &key, redis_desc_map_int_str_t &mapOut);
	redis_integer_t hincrby(const string_t &key, const string_t &field, redis_integer_t nIncr = 1);
	redis_integer_t hlen(const string_t &key);
	redis_integer_t hscan(const string_t &key, redis_integer_t cursor, redis_integer_t count, redis_map_t &mapOut);
	bool hmset(const string_t &key, redis_map_t &fields);
	redis_array_t hmget(const string_t &key, std::initializer_list<string_t> fields);
	redis_integer_t hdel(const string_t &key, std::initializer_list<string_t> fields);

	// list
	redis_array_t lrange(const string_t &key, int iBeg = 0, int iEnd = -1);
	redis_integer_t lpush(const string_t &key, const string_t &val);
	redis_integer_t rpush(const string_t &key, const string_t &val);
	redis_integer_t lrem(const string_t &key, const string_t &val);
	redis_integer_t llen(const string_t &key);
	string_t lpop(const string_t &key);

	//sortset
	redis_integer_t zcard(const string_t &key);
	redis_integer_t zadd(const string_t &key,const string_t & score,const string_t & member);
	redis_array_t zrevrange(const string_t &key,int iBeg = 0, int iEnd = -1);
	redis_array_t zrange(const string_t &key,int iBeg = 0, int iEnd = -1);
	redis_array_t zrangebyscore(const string_t &key,int iBeg = 0, int iEnd = -1);

	redis_integer_t zrem(const string_t &key,const string_t &member);
	string_t zscore(const string_t &key,const string_t &member);

	//bitmap
	redis_integer_t bitCount(const string_t &key);
	redis_integer_t setBit(const string_t &key,const string_t &sOffset,const string_t &sValue);
	redis_integer_t getBit(const string_t &key,const string_t &sOffset);
	
	// 事务
	bool multi();    // 标记一个事务块的开始
	bool discard();  // 取消事务，放弃执行事务块内的所有命令
	int exec();      // 执行所有事务块内的命令

	//
	virtual const char *strerror()const;
	virtual bool HasError()const;
	////// Format a command according to the Redis protocol ////////
	// "*3\r\n$3\r\nSET\r\n$10\r\ntest_hello\r\n$11\r\nhello world\r\n"
	virtual const char* formatCommand(const char *pFmt, ...);
	virtual bool formatQuery(const string_t &cmd);
 protected:
	virtual bool query(unsigned char type, const char* fmt, ...);
	virtual bool query(unsigned char type, int argc, const char **argv, const size_t *argvlen);
	void freeReply();
	//
 public:
	union {
		redisContext *pSingle;
		redisClusterContext *pCluster;
	} m_ctx;
	redisReply   *m_reply;
	std::string   m_sFmtCmd;
};

#endif /* _REDISCLIENT_H_ */
