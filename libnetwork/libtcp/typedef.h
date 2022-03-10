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

#endif // !_TCP_TYPEDEF_H_