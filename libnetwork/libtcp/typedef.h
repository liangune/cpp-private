/*************************************************************************
	> Date:   2020-12-22
	> Author: liangjun
	> Desc:   宏定义
*************************************************************************/

#ifndef _LIB_TCP_TYPEDEF_H_
#define _LIB_TCP_TYPEDEF_H_

#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include <stdint.h>
#include <map>

/*define*/
#if !defined(MALLOC) || !defined(FREE) || !defined(NEW) || !defined(DELETE)
# define MALLOC malloc
# define FREE(p)   if(p) free(p)
# define NEW new
# define DELETE delete
#endif

#define PIPE_NOTIFY_CHR 'P'
/*define*/

/*CONN_INFO*/
class CConnection;
typedef struct CONN_INFO_S{
	int32_t sfd; // socket fd
	struct bufferevent *pClientTcpEvent;
	CConnection* pConnection;
	
	CONN_INFO_S *prev, *next;

	int64_t nLastKeepalive;
	int64_t nAcceptTm;
} CONN_INFO;
typedef std::map<int32_t, CONN_INFO*> Map_ConnInfo;
/*CONN_INFO*/

/*PIPE*/
typedef struct {
	struct event	*pEvent;
	int32_t			nReceiveFd;
	int32_t			nSendFd;
}PIPE;
/*PIPE*/

/*acceptor call back*/
class CTcpMaster;
typedef struct {
	CTcpMaster* pObj;
	typedef bool (CTcpMaster::*fun)(evutil_socket_t);
	fun callback;
}AcceptorCbParam;
/*acceptor call back*/

/*void* param*/
typedef struct {
	void* p1;
	void* p2;
}TwoVoidPtrParam;
/*void* param*/

// 系统不可用最大周期数（如一周期为一分钟，则可容纳最长不可用时间为3分钟）
#define SYSTEM_LOSE_AVAILABILITY_ROUND 2
// 接入层检测客户端keepalive的时间间隔
#define GATEWAY_CHECK_CLIENT_KEEPALIVE 150
// 客户端连接keepalive超时事件
#define BEV_EVENT_CONNECTION_KEEPALIVE_TIMEOUT 0x100
// 强制关闭客户端连接
#define BEV_EVENT_FORCE_CLOSE_CONNECTION 0x200

#endif // !_TCP_TYPEDEF_H_