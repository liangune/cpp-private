/*************************************************************************
	> Date:   2020-12-22
	> Author: liangjun
	> Desc:   解析器
*************************************************************************/

#ifndef _LIB_TCP_IPARSER_H_
#define _LIB_TCP_IPARSER_H_

#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "zlog/zlog.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <string>

class CWorker;
// tcpserver 统一接口
class CConnectionBase
{
public:
	virtual ~CConnectionBase(){}
	virtual void OnRead(void *param) = 0;
	virtual void OnWrite(void *param) = 0;
	virtual void OnError(void *param) = 0;
	virtual size_t GetBuffTotalLen() = 0;
	virtual int32_t BuffCopy(char *buff, uint32_t nNeedLen) = 0;
	virtual int32_t Recv(char *buff, int32_t nRecvLen) = 0;
	virtual int32_t Send(const char *data, size_t nDataLen) = 0;
};

// libevent
class CConnectionData: public CConnectionBase
{
	friend class CWorker;
public:
	CConnectionData();
	virtual ~CConnectionData(){};

	virtual void OnRead(void *param);
	virtual void OnWrite(void *param);
	virtual void OnError(void *param);

	int32_t GetFd();
	std::string GetPeerIp();
	int32_t GetPeerPort();

protected:
	virtual void OnRecv() {};
	virtual void OnDealError() {};
	virtual void OnSend() {};

protected:
	size_t GetBuffTotalLen();

	/**
	 * 从缓存中copy数据，并且缓存中数据不删除
	 */
	int32_t BuffCopy(char *buff, uint32_t nNeedLen);

	int32_t Recv(char *buff, int32_t nRecvLen);
	int32_t Send(const char *data, size_t nDataLen);
	uint32_t GetEvtbufLetfLen();
	
	/**
	 * 从缓存中搜索字符串位置
	 */
	size_t BufferSearch(const char *what, size_t len);

	//char *evbuffer_readln(struct evbuffer *buffer, size_t *n_read_out, enum evbuffer_eol_style eol_style);
	int32_t BufferReadln(char *buff);

protected:
	struct bufferevent * m_pClientTcpEvent;
	CWorker* m_pWorker;
};

// 协议解析
class CConnection: public CConnectionData
{
public:
	CConnection(): m_bFirstReq(false) {}
	virtual ~CConnection() {}
	virtual void Init() = 0;

	void SetIndex(uint32_t nIndex)
	{
		m_nIndex = nIndex;
	}

	void SetAcceptTm(int64_t nTm)
	{
		m_bFirstReq = true;
		m_nAcceptTm = nTm;
	}

protected:
	uint32_t m_nIndex;

	//accept的时间戳
	int64_t m_nAcceptTm;
	bool m_bFirstReq;
};


#endif /* _TCP_IPARSER_H_ */
