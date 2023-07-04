/*************************************************************************
	> Date:   2020-9-22
	> Author: liangjun
	> Desc:   基于hredis的client
*************************************************************************/

#include <stdarg.h>
#include "redisClient.h"
#include <string.h>
#include <stdlib.h>
#include "libpublic/stdStrUtils.h"
#include "hiredis/win32.h"

CRedisClient::CRedisClient():
m_reply(NULL)
{
	m_ctx.pSingle = NULL;
}

CRedisClient::~CRedisClient()
{
	close();
}
/*
 * @param: timeout 毫秒
*/
int CRedisClient::connect(const string_t &ip, int port, long timeout, string_t passwd)
{
	struct timeval tv;
	tv.tv_sec  = timeout/1000;
	tv.tv_usec = (timeout%1000) * 1000;

	redisContext * c = redisConnectWithTimeout(C_STR(ip), port, tv);

	if( c == NULL )
	{
		printf("redis connect error: context is null\n");
		return -1;
	}
	else if( c->err )
	{
		printf("redis connect error:: %s\n", c->errstr);
		return -1;
	}

	m_ctx.pSingle = c;
	return 0;
}

void CRedisClient::close()
{
	if( m_ctx.pSingle )
	{
		redisFree(m_ctx.pSingle);
		m_ctx.pSingle = NULL;
	}
	this->freeReply();
}

bool CRedisClient::ping()
{
	if( this->query(REDIS_REPLY_STATUS, "%s", "PING") )
	{
		static const char PONG[] = "PONG";
		return (m_reply->str && strncmp(m_reply->str, PONG, sizeof(PONG)-1) == 0);
	}
	return false;
}

void CRedisClient::freeReply()
{
	if( m_reply )
	{
		freeReplyObject(m_reply);
		m_reply = NULL;
	}
}

bool CRedisClient::setTimeout(long timeout)
{
	struct timeval tv;
	tv.tv_sec  = timeout/1000;
	tv.tv_usec = (timeout%1000) * 1000;
	return redisSetTimeout(m_ctx.pSingle, tv) != REDIS_ERR;
}

bool CRedisClient::setKeepAlive()
{
	return redisEnableKeepAlive(m_ctx.pSingle) != REDIS_ERR;
}

bool CRedisClient::query(unsigned char type, const char* fmt, ...)
{
	if( m_ctx.pSingle )
	{
		this->freeReply();
		va_list args;
		va_start(args, fmt);
		m_reply = (redisReply*)redisvCommand(m_ctx.pSingle, fmt, args);
		va_end(args);
		if( m_reply && m_reply->type == type)
			return true;
		// 使用事务情况下返回
		else if( m_reply && m_reply->type == REDIS_REPLY_STATUS)
		{
			if( strncmp(m_reply->str, "QUEUED", 6) == 0)
			{
				m_reply->integer = 1;
				return true;
			}
		}
	}

	return false;
}

bool CRedisClient::query(const char* fmt, ...)
{
	if( m_ctx.pSingle )
	{
		this->freeReply();
		va_list args;
		va_start(args, fmt);
		m_reply = (redisReply*)redisvCommand(m_ctx.pSingle, fmt, args);
		va_end(args);
		if( m_reply && (m_reply->type != REDIS_REPLY_ERROR) )
			return true;
	}

	return false;
}

bool CRedisClient::query(unsigned char type, int argc, const char **argv, const size_t *argvlen)
{
	if( m_ctx.pSingle )
	{
		this->freeReply();
		m_reply = (redisReply*)redisCommandArgv(m_ctx.pSingle, argc, argv, argvlen);
		if( m_reply && (m_reply->type == type) )
			return true;
	}

	return false;	
}

bool CRedisClient::del_match_keys(const string_t &match_key)
{
	redis_array_t keys_arry = keys(match_key);
	for (redis_array_t::iterator itr = keys_arry.begin(); itr != keys_arry.end(); ++itr)
	{
		expire(*itr, 1);
	}

	return true;
}

const char *CRedisClient::strerror()const
{
	if( m_reply && m_reply->type == REDIS_REPLY_ERROR)
	{
		return m_reply->str;
	}
	else if( m_ctx.pSingle && m_ctx.pSingle->err )
	{
		return m_ctx.pSingle->errstr;
	}
	return "";
}

bool CRedisClient::HasError()const
{
	if( m_reply && m_reply->type == REDIS_REPLY_ERROR)
	{
		return true;
	}
	else if( m_ctx.pSingle && m_ctx.pSingle->err )
	{
		return true;
	}
	else if ( !m_ctx.pSingle)
	{
		return true;
	}

	return false;
}

////////////// key //////////////
///
redis_integer_t CRedisClient::ttl(const string_t &key)
{
	if( this->query(REDIS_REPLY_INTEGER, "TTL %s", C_STR(key)) )
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::pttl(const string_t &key)
{
	if (this->query(REDIS_REPLY_INTEGER, "PTTL %s", C_STR(key)))
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::del(const string_t &key)
{
	if( this->query(REDIS_REPLY_INTEGER, "DEL %s", C_STR(key)) )
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::exists(const string_t &key)
{
	if( this->query(REDIS_REPLY_INTEGER, "EXISTS %s", C_STR(key)) )
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::expire(const string_t &key, unsigned int seconds)
{
	if( this->query(REDIS_REPLY_INTEGER, "EXPIRE %s %u", C_STR(key), seconds) )
		return m_reply->integer;
	return false;
}

redis_integer_t CRedisClient::pexpire(const string_t &key, unsigned int ms)
{
	if (this->query(REDIS_REPLY_INTEGER, "PEXPIRE %s %u", C_STR(key), ms))
		return m_reply->integer;
	return false;
}

redis_array_t CRedisClient::keys(const string_t &key)
{
	redis_array_t redis_array;
	if( this->query(REDIS_REPLY_ARRAY, "KEYS %s", C_STR(key)) )
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
		return redis_array;
	}
	return redis_array;
}

bool CRedisClient::rename(const string_t &key, const string_t &newkey)
{
	if( this->query(REDIS_REPLY_STATUS, "RENAME %s %s", C_STR(key), C_STR(newkey)) )
		return REPLY_STATUS_OK(m_reply->str);
	return false;
}

////////////// string //////////////
string_t CRedisClient::get(const string_t &key)
{
	string_t sValue;
	if( this->query(REDIS_REPLY_STRING, "GET %s", C_STR(key)) )
		sValue = m_reply->str;
	// if( m_reply && m_reply->type == REDIS_REPLY_STRING )
		// return m_reply->str;
	return sValue;
}
redis_integer_t CRedisClient::incr(const string_t &key)
{
	if( this->query(REDIS_REPLY_INTEGER, "INCR %s", C_STR(key)) )
		return m_reply->integer;
	return 0;
}
redis_integer_t CRedisClient::incrby(const string_t &key, redis_integer_t nIncr)
{
	if( this->query(REDIS_REPLY_INTEGER, "INCRBY %s %lld", C_STR(key), nIncr) )
		return m_reply->integer;
	return 0;
}
redis_integer_t CRedisClient::decr(const string_t &key)
{
	if( this->query(REDIS_REPLY_INTEGER, "DECR %s", C_STR(key)) )
		return m_reply->integer;
	return 0;
}
redis_integer_t CRedisClient::decrby(const string_t &key, redis_integer_t nIncr)
{
	if( this->query(REDIS_REPLY_INTEGER, "DECRBY %s %lld", C_STR(key), nIncr) )
		return m_reply->integer;
	return 0;
}

bool CRedisClient::set(const string_t &key, const string_t &val)
{
	if( this->query(REDIS_REPLY_STATUS, "SET %s %s", C_STR(key), C_STR(val)) )
		return REPLY_STATUS_OK(m_reply->str);
	return false;
}

bool CRedisClient::setex(const string_t& key, const string_t& val, redis_integer_t expire)
{
	if (this->query(REDIS_REPLY_STATUS, "SET %s %s EX %d", C_STR(key), C_STR(val), expire))
		return REPLY_STATUS_OK(m_reply->str);
	return false;
}

redis_integer_t CRedisClient::setnx(const string_t &key, const string_t &val)
{
	if( this->query(REDIS_REPLY_INTEGER, "SETNX %s %s", C_STR(key), C_STR(val) ) )
		return m_reply->integer;
	return 0;
}

/*
@expire:0--永久生效；其他--key过期时间，单位 s
*/
bool CRedisClient::setnx_ex(const string_t& key, const string_t& val, redis_integer_t expire/*=0*/)
{
	string_t sQuery = StdStrUtils::Format("SET %s %s NX ", C_STR(key), C_STR(val));
	if (expire > 0)
	{
		sQuery.append(StdStrUtils::Format("EX %d", expire));
	}
	if (this->query(REDIS_REPLY_STATUS, C_STR(sQuery)))
		return REPLY_STATUS_OK(m_reply->str);

	return false;
}

string_t CRedisClient::getset(const string_t &key, const string_t &val)
{
	string_t sValue;
	if(this->query(REDIS_REPLY_STRING, "GETSET %s %s", C_STR(key), C_STR(val)))
		sValue = m_reply->str;

	return sValue;
}

////////////// set //////////////
redis_integer_t CRedisClient::sadd(const string_t &key, const string_t &member)
{
	if( this->query(REDIS_REPLY_INTEGER, "SADD %s %s", C_STR(key), C_STR(member)) )
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::srem(const string_t &key, const string_t &member)
{
	if( this->query(REDIS_REPLY_INTEGER, "SREM %s %s", C_STR(key), C_STR(member)) )
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::sismember(const string_t &key, const string_t &member)
{
	if( this->query(REDIS_REPLY_INTEGER, "SISMEMBER %s %s", C_STR(key), C_STR(member)) )
		return m_reply->integer;
	return 0;
}

redis_array_t CRedisClient::smembers(const string_t &key)
{
	redis_array_t redis_array;
	if( this->query(REDIS_REPLY_ARRAY, "SMEMBERS %s", C_STR(key)) )
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
		return redis_array;
	}
	return redis_array;
}


bool CRedisClient::smembers(const string_t &key, redis_array_t& arrOut)
{
	arrOut.clear();
	if( this->query(REDIS_REPLY_ARRAY, "SMEMBERS %s", C_STR(key)) )
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			arrOut.push_back(m_reply->element[i]->str);
		}
	}
	return true;
}

bool CRedisClient::smembers(const string_t &key, redis_set_t& arrOut)
{
	arrOut.clear();
	if( this->query(REDIS_REPLY_ARRAY, "SMEMBERS %s", C_STR(key)) )
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			arrOut.insert(m_reply->element[i]->str);
		}
	}
	return true;
}


size_t CRedisClient::scard(const string_t &key)
{
    if (this->query(REDIS_REPLY_INTEGER, "SCARD %s", C_STR(key)))
        return m_reply->integer;
    return 0;
}

redis_array_t CRedisClient::srandmember(const string_t &key, unsigned int nCnt)
{
	redis_array_t redis_array;
	if( this->query(REDIS_REPLY_ARRAY, "SRANDMEMBER %s %u", C_STR(key), nCnt) )
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
		return redis_array;
	}

	return redis_array;
}

redis_integer_t CRedisClient::sinterstore(const string_t &des, const string_t &key1, const string_t &key2)
{
	if( this->query(REDIS_REPLY_INTEGER, "SINTERSTORE %s %s %s",
		C_STR(des), C_STR(key1), C_STR(key2)) )
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::sdiffstore(const string_t &sDstSet, const string_t &sSet1,
	const string_t &sSet2)
{
	if( this->query(REDIS_REPLY_INTEGER, "SDIFFSTORE %s %s %s",
		C_STR(sDstSet), C_STR(sSet1), C_STR(sSet2)) )
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::sdiffstore(const string_t &sDstSet, const string_t &sSet1, const string_t &sSet2, const string_t &sSet3)
{
	if( this->query(REDIS_REPLY_INTEGER, "SDIFFSTORE %s %s %s %s",
		C_STR(sDstSet), C_STR(sSet1), C_STR(sSet2), C_STR(sSet3)) )
		return m_reply->integer;
	return 0;
}

string_t CRedisClient::spop(const string_t &key)
{
	string_t sVal;
	if( this->query(REDIS_REPLY_STRING, "SPOP %s", C_STR(key)) )
		sVal = m_reply->str;

	return sVal;
}

////////////// hash //////////////
redis_integer_t CRedisClient::hset(const string_t &key, const string_t &field, const string_t &value)
{
	if( this->query(REDIS_REPLY_INTEGER, "HSET %s %s %s", C_STR(key), C_STR(field), C_STR(value)) )
		return m_reply->integer;
	return 0;
}
string_t CRedisClient::hget(const string_t &key, const string_t &field)
{
	string_t sValue;
	if( this->query(REDIS_REPLY_STRING, "HGET %s %s", C_STR(key), C_STR(field)) )
		sValue = m_reply->str;
	return sValue;
}
redis_integer_t CRedisClient::hdel(const string_t &key, const string_t &field)
{
	if( this->query(REDIS_REPLY_INTEGER, "HDEL %s %s", C_STR(key), C_STR(field)) )
		return m_reply->integer;
	return 0;
}
redis_integer_t CRedisClient::hlen(const string_t &key)
{
	if( this->query(REDIS_REPLY_INTEGER, "HLEN %s", C_STR(key)) )
		return m_reply->integer;
	return 0;
}
redis_integer_t CRedisClient::hexists(const string_t &key, const string_t &field)
{
	if( this->query(REDIS_REPLY_INTEGER, "HEXISTS %s %s", C_STR(key), C_STR(field)) )
		return m_reply->integer;
	return 0;
}
redis_array_t CRedisClient::hkeys(const string_t &key)
{
	redis_array_t redis_array;
	if( this->query(REDIS_REPLY_ARRAY, "HKEYS %s", C_STR(key)) )
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
		return redis_array;
	}
	return redis_array;
}


void CRedisClient::hkeys(const string_t &key, redis_set_t &redis_set)
{
	if (this->query(REDIS_REPLY_ARRAY, "HKEYS %s", C_STR(key)))
	{
		for (size_t i = 0; i < m_reply->elements; ++i)
		{
			redis_set.insert(m_reply->element[i]->str);
		}
	}
}

void CRedisClient::hkeysLess(string_t key, int64_t now, std::set<int64_t> &sKeySet)
{
	int64_t time = 0;
	if (this->query(REDIS_REPLY_ARRAY, "HKEYS %s", C_STR(key)))
	{
		for (size_t i = 0; i < m_reply->elements; ++i)
		{
			time = StdStrUtils::Str2Int(m_reply->element[i]->str);
			if (now < time)
			{
				continue;
			}
			sKeySet.insert(time);
		}
	}
}

void CRedisClient::hkeysBigger(string_t key, int64_t now, std::set<int64_t> &sKeySet)
{
	int64_t time = 0;
	if (this->query(REDIS_REPLY_ARRAY, "HKEYS %s", C_STR(key)))
	{
		for (size_t i = 0; i < m_reply->elements; ++i)
		{
			time = StdStrUtils::Str2Int(m_reply->element[i]->str);
			if (now > time)
			{
				continue;
			}
			sKeySet.insert(time);
		}
	}
}

redis_array_t CRedisClient::hvals(const string_t &key)
{
	redis_array_t redis_array;
	if( this->query(REDIS_REPLY_ARRAY, "HVAlS %s", C_STR(key)) )
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
		return redis_array;
	}
	return redis_array;
}

void CRedisClient::hvals(const string_t &key /*IN*/, redis_array_t &redis_array /*OUT*/)
{
	redis_array.clear();
	if (this->query(REDIS_REPLY_ARRAY, "HVAlS %s", C_STR(key)))
	{
		for (size_t i = 0; i < m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
	}
}

redis_array_t CRedisClient::hgetall(const string_t &key)
{
	redis_array_t redis_array;
	if( this->query(REDIS_REPLY_ARRAY, "HGetALL %s", C_STR(key)) )
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
		return redis_array;
	}
	return redis_array;
}

bool CRedisClient::hgetall(const string_t &key, redis_map_t &mapOut)
{
	mapOut.clear();
	if( this->query(REDIS_REPLY_ARRAY, "HGetALL %s", C_STR(key)) )
	{
		if (m_reply->elements % 2 != 0)
			return false;
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			mapOut[m_reply->element[i]->str] = m_reply->element[i+1]->str;
			++i;
		}
	}
	return true;
}

bool CRedisClient::hgetall(const string_t &key, redis_map_int_str_t &mapOut)
{
	mapOut.clear();
	if( this->query(REDIS_REPLY_ARRAY, "HGetALL %s", C_STR(key)) )
	{
		if (m_reply->elements % 2 != 0)
			return false;
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			int64_t nKey = atoll(m_reply->element[i]->str);
			mapOut[nKey] = m_reply->element[i+1]->str;
			++i;
		}
	}
	return true;
}

bool CRedisClient::hgetall(const string_t &key, redis_desc_map_int_str_t &mapOut)
{
	mapOut.clear();
	if (this->query(REDIS_REPLY_ARRAY, "HGetALL %s", C_STR(key)))
	{
		if (m_reply->elements % 2 != 0)
			return false;
		for (size_t i = 0; i < m_reply->elements; ++i)
		{
			int64_t nKey = atoll(m_reply->element[i]->str);
			mapOut[nKey] = m_reply->element[i + 1]->str;
			++i;
		}
	}
	return true;
}

redis_integer_t CRedisClient::hincrby(const string_t &key, const string_t &field, redis_integer_t nIncr)
{
	if( this->query(REDIS_REPLY_INTEGER, "HINCRBY %s %s %lld", C_STR(key), C_STR(field), nIncr) )
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::hscan(const string_t &key, redis_integer_t cursor, redis_integer_t count, redis_map_t &mapOut)
{
	mapOut.clear();
	redis_integer_t replyCursor = 0;
	if( this->query(REDIS_REPLY_ARRAY, "HSCAN %s %lld COUNT %lld", C_STR(key), cursor, count) )
	{
		if (m_reply->elements != 2)
			return replyCursor;

		replyCursor = atoll(m_reply->element[0]->str);
		for(size_t i=0; i<m_reply->element[1]->elements; ++i)
		{
			mapOut[m_reply->element[1]->element[i]->str] = m_reply->element[1]->element[i+1]->str;
			++i;
		}
	}
	return replyCursor;	
}

bool CRedisClient::hmset(const string_t &key, redis_map_t &fields)
{
	redis_array_t vecCmd;
	vecCmd.push_back("HMSET");
	vecCmd.push_back(key);

	string_t sFields;
	for(auto it = fields.begin(); it != fields.end(); it++) {
		vecCmd.push_back(it->first);
		vecCmd.push_back(it->second);
	}
	std::vector<const char *> argv(vecCmd.size());
	std::vector<size_t> argvlen(vecCmd.size());
	uint32_t i = 0;
	for(auto it = vecCmd.begin(); it != vecCmd.end(); it++, i++) {
		argv[i] = it->c_str();
		argvlen[i] = it->size();
	}

	if( this->query(REDIS_REPLY_STATUS, argv.size(), &(argv[0]), &(argvlen[0])) )
	{
		return REPLY_STATUS_OK(m_reply->str);
	}
 	
	return false;
}

redis_array_t CRedisClient::hmget(const string_t &key, std::initializer_list<string_t> fields)
{
	redis_array_t vecCmd;
	vecCmd.push_back("HMGET");
	vecCmd.push_back(key);

	string_t sFields;
	for(auto it = fields.begin(); it != fields.end(); it++) {
		vecCmd.push_back(*it);
	}
	std::vector<const char *> argv(vecCmd.size());
	std::vector<size_t> argvlen(vecCmd.size());
	uint32_t i = 0;
	for(auto it = vecCmd.begin(); it != vecCmd.end(); it++, i++) {
		argv[i] = it->c_str();
		argvlen[i] = it->size();
	}

	redis_array_t redis_array;
	if( this->query(REDIS_REPLY_ARRAY, argv.size(), &(argv[0]), &(argvlen[0])) )
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			if (m_reply->element[i]->str == NULL) {
				redis_array.push_back("");
			} else {
				redis_array.push_back(m_reply->element[i]->str);
			}
		}
		return redis_array;
	}

	return redis_array;
}


//////////////// list ////////////////
redis_array_t CRedisClient::lrange(const string_t &key, int iBeg, int iEnd)
{
	/*unsigned int len = 0;
	if( this->query(REDIS_REPLY_INTEGER, "LLEN  %s", C_STR(key)) )
	{
		len = m_reply->integer;
	}*/
	redis_array_t redis_array;
	if(this->query(REDIS_REPLY_ARRAY,"LRANGE %s %d %d", C_STR(key), iBeg, iEnd))
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
		return redis_array;
	}
	return redis_array;
}

redis_integer_t CRedisClient::rpush(const string_t &key, const string_t &val)
{
	if( this->query(REDIS_REPLY_INTEGER, "RPUSH %s %s", C_STR(key), C_STR(val)) )
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::lpush(const string_t &key, const string_t &val)
{
	if( this->query(REDIS_REPLY_INTEGER, "LPUSH %s %s", C_STR(key), C_STR(val)) )
		return m_reply->integer;
	return 0;
}

string_t CRedisClient::lpop(const string_t &key)
{
	string_t sVal;
	if( this->query(REDIS_REPLY_STRING, "LPOP %s", C_STR(key)) )
		sVal = m_reply->str;

	return sVal;
}

redis_integer_t CRedisClient::lrem(const string_t &key, const string_t &val)
{
	if( this->query(REDIS_REPLY_INTEGER, "LREM %s 1 %s", C_STR(key), C_STR(val)) )
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::llen(const string_t &key)
{
	if( this->query(REDIS_REPLY_INTEGER, "LLEN %s", C_STR(key)) )
		return m_reply->integer;
	return 0;
}
///////// 事务
bool CRedisClient::multi()
{
	if( this->query(REDIS_REPLY_STATUS, "MULTI") )
		return REPLY_STATUS_OK(m_reply->str);
	return false;
}
bool CRedisClient::discard()
{
	if( this->query(REDIS_REPLY_STATUS, "DISCARD") )
		return REPLY_STATUS_OK(m_reply->str);
	return false;
}
int CRedisClient::exec()
{
	if( this->query(REDIS_REPLY_STATUS, "EXEC") )
		return REPLY_STATUS_OK(m_reply->str);
	return false;
}

////////////////////////// formatted command //////////////////////////
const char* CRedisClient::formatCommand(const char *pFmt, ...)
{
	char *pFmtCmd = NULL;

	va_list ap;
    int len;
    va_start(ap,pFmt);
    len = redisvFormatCommand(&pFmtCmd, pFmt, ap);
    va_end(ap);

    /* The API says "-1" means bad result, but we now also return "-2" in some
     * cases.  Force the return value to always be -1. */
    if (len > 0)
    	m_sFmtCmd = pFmtCmd;
    else
    	m_sFmtCmd = "";
    if( pFmtCmd )
    	free(pFmtCmd);
    return m_sFmtCmd.c_str();
}

bool CRedisClient::formatQuery(const string_t &cmd)
{
	if( m_ctx.pSingle )
	{
		if( redisAppendFormattedCommand(m_ctx.pSingle, cmd.c_str(), cmd.length()) != REDIS_OK)
			return false;
		if (m_ctx.pSingle->flags & REDIS_BLOCK)
		{
			void *reply;
	        if (redisGetReply(m_ctx.pSingle, &reply) == REDIS_OK )
	        {
	        	m_reply = static_cast<redisReply*>(reply);
	        	if( !m_reply || m_reply->type == REDIS_REPLY_ERROR )
	        		return false;

	            return true;
	        }
	    }
	}
	return false;
}

redis_integer_t CRedisClient::zcard(const string_t &key)
{
	if( this->query(REDIS_REPLY_INTEGER, "ZCARD %s", C_STR(key)))
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::zadd(const string_t &key,const string_t & score,const string_t & member)
{
	if( this->query(REDIS_REPLY_INTEGER, "ZADD %s %s %s", C_STR(key),C_STR(score),C_STR(member)))
		return m_reply->integer;
	return 0;
}

redis_array_t  CRedisClient::zrevrange(const string_t &key,int iBeg, int iEnd)
{
	redis_array_t redis_array;
	if( this->query(REDIS_REPLY_ARRAY, "ZREVRANGE %s %d %d", C_STR(key),iBeg,iEnd ))
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
		return redis_array;
	}
	return redis_array;
}

redis_array_t  CRedisClient::zrange(const string_t &key,int iBeg, int iEnd)
{
	redis_array_t redis_array;
	if( this->query(REDIS_REPLY_ARRAY, "ZRANGE %s %d %d", C_STR(key),iBeg,iEnd ))
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
		return redis_array;
	}
	return redis_array;
}

redis_array_t CRedisClient::zrangebyscore(const string_t &key,int iBeg, int iEnd)
{
	redis_array_t redis_array;
	if( this->query(REDIS_REPLY_ARRAY, "ZRANGEBYSCORE %s %d %d", C_STR(key),iBeg,iEnd ))
	{
		for(size_t i=0; i<m_reply->elements; ++i)
		{
			redis_array.push_back(m_reply->element[i]->str);
		}
		return redis_array;
	}
	return redis_array;
}

redis_integer_t CRedisClient::zrem(const string_t &key,const string_t &member)
{
	if( this->query(REDIS_REPLY_INTEGER, "ZREM %s %s", C_STR(key),C_STR(member)))
		return m_reply->integer;
	return 0;
}

string_t CRedisClient::zscore(const string_t &key,const string_t &member)
{
	string_t sRes;
	if( this->query(REDIS_REPLY_STRING, "ZSCORE %s %s", C_STR(key),C_STR(member)))
		sRes = m_reply->str;
	return sRes;
}
//bitmap
redis_integer_t CRedisClient::bitCount(const string_t &key)
{
	if( this->query(REDIS_REPLY_INTEGER, "BITCOUNT %s", C_STR(key)))
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::setBit(const string_t &key,const string_t &sOffset,const string_t &sValue)
{
	if( this->query(REDIS_REPLY_INTEGER, "SETBIT %s %s %s", C_STR(key), C_STR(sOffset), C_STR(sValue)))
		return m_reply->integer;
	return 0;
}

redis_integer_t CRedisClient::getBit(const string_t &key,const string_t &sOffset)
{
	if( this->query(REDIS_REPLY_INTEGER, "GETBIT %s %s", C_STR(key),C_STR(sOffset)))
		return m_reply->integer;
	return 0;
}