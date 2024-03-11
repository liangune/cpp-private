/*************************************************************************
	> Date:   2020-9-22
	> Author: liangjun
	> Desc:   
*************************************************************************/
#include "redisClusterClient.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "hiredis/win32.h"

CRedisClusterClient::CRedisClusterClient()
{
	m_ctx.pCluster = NULL;
}

CRedisClusterClient::~CRedisClusterClient()
{
	this->close();
}

int CRedisClusterClient::connect(const string_t &sAddres, int flags, long timeout, string_t password)
{
	struct timeval tv;
	tv.tv_sec  = timeout/1000;
	tv.tv_usec = (timeout%1000) * 1000;

	redisClusterContext * c = redisClusterConnectWithTimeoutAuth(C_STR(sAddres), tv, flags, NULL, password.c_str());

	if( c == NULL )
	{
		printf("redis cluster connect error: context is null\n");
		return -1;
	}
	else if( c->err )
	{
		printf("redis cluster connect error:: %s\n", c->errstr);
		return -1;
	}

	m_ctx.pCluster = c;
	return 0;
}

void CRedisClusterClient::close()
{
	if( m_ctx.pCluster )
	{
		redisClusterFree(m_ctx.pCluster);
		m_ctx.pCluster = NULL;
	}
	this->freeReply();
}

bool CRedisClusterClient::query(unsigned char type, const char* fmt, ...)
{
	if( m_ctx.pCluster )
	{
		this->freeReply();
		va_list args;
		va_start(args, fmt);
		m_reply = (redisReply*)redisClustervCommand(m_ctx.pCluster, fmt, args);
		va_end(args);

		if (m_reply && m_reply->type == type) {
			return true;
		}
		// 使用事务情况下返回
		
		else if( m_reply && m_reply->type == REDIS_REPLY_STATUS)
		{
			if( strncmp(m_reply->str, "QUEUED", 6) == 0)
			{
				m_reply->integer = 1;
				return true;
			}
		}
		else
		{
			//printf("err: m_reply = %p\n", m_reply);
		}
	}

	return false;
}

bool CRedisClusterClient::query(unsigned char type, int argc, const char **argv, const size_t *argvlen)
{
	if( m_ctx.pCluster )
	{
		this->freeReply();
		m_reply = (redisReply*)redisClusterCommandArgv(m_ctx.pCluster, argc, argv, argvlen);
		if( m_reply && (m_reply->type == type) )
			return true;
	}

	return false;	
}

bool CRedisClusterClient::formatQuery(const string_t &cmd)
{
	if( m_ctx.pCluster )
	{
		if( redisClusterAppendFormattedCommand(m_ctx.pCluster, (char*)cmd.c_str(), cmd.length()) != REDIS_OK)
			return false;
		if (m_ctx.pCluster->flags & REDIS_BLOCK)
		{
			void *reply;
	        if (redisClusterGetReply(m_ctx.pCluster, &reply) == REDIS_OK)
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

const char *CRedisClusterClient::strerror()const
{
	if( m_reply && m_reply->type == REDIS_REPLY_ERROR)
	{
		return m_reply->str;
	}
	else if( m_ctx.pCluster && m_ctx.pCluster->err )
	{
		return m_ctx.pCluster->errstr;
	}
	return "";
}

bool CRedisClusterClient::HasError()const
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

bool CRedisClusterClient::ping()
{
	return true;
}
