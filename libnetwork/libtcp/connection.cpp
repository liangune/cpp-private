/*************************************************************************
	> Date:   2020-12-22
	> Author: liangjun
	> Desc:   解析器
*************************************************************************/

#include "typedef.h"
#include "connection.h"
#include "worker.h"
#include <string.h>

#ifdef _MSC_VER

#else
#include<sys/socket.h> 
#include<netinet/in.h>  
#include<unistd.h>  
#include<arpa/inet.h>
#endif

/*ILibevServer*/

CConnectionData::CConnectionData() : m_pClientTcpEvent(NULL), m_pWorker(NULL)
{
}

void CConnectionData::OnRead(void *param)
{
	TwoVoidPtrParam* p = (TwoVoidPtrParam*)param;
	m_pClientTcpEvent = (bufferevent*)(p->p1);
	m_pWorker = (CWorker*)(p->p2);

	this->OnRecv();
}

void CConnectionData::OnWrite(void *param)
{
	TwoVoidPtrParam* p = (TwoVoidPtrParam*)param;
	m_pClientTcpEvent = (bufferevent*)(p->p1);
	m_pWorker = (CWorker*)(p->p2);
	this->OnSend();
}

void CConnectionData::OnError(void *param)
{
	TwoVoidPtrParam* p = (TwoVoidPtrParam*)param;
	m_pClientTcpEvent = (bufferevent*)(p->p1);
	m_pWorker = (CWorker*)(p->p2);
	this->OnDealError();
}

size_t CConnectionData::GetBuffTotalLen()
{
	if(m_pClientTcpEvent)
	{
		struct evbuffer * input = bufferevent_get_input(m_pClientTcpEvent);
		return evbuffer_get_length(input);
	}

	return 0;
}

int32_t CConnectionData::BuffCopy(char *buff, uint32_t nNeedLen)
{
	if(m_pClientTcpEvent)
	{
		struct evbuffer * input = bufferevent_get_input(m_pClientTcpEvent);
		if (input==NULL)
			return -1;

		uint32_t nReadLen = 0;
		while( (evbuffer_get_length(input) >= nNeedLen) && (nReadLen < nNeedLen))
		{
			int32_t nRet = evbuffer_copyout(input, buff+nReadLen, nNeedLen - nReadLen);
			if( nRet == -1)
			{
				return -1;
			}
			nReadLen += nRet;
		}
		return nReadLen;
	}
	return -1;
}

size_t CConnectionData::BufferSearch(const char *what, size_t len)
{
	if(m_pClientTcpEvent)
	{
		struct evbuffer * input = bufferevent_get_input(m_pClientTcpEvent);
		struct evbuffer_ptr ptr = evbuffer_search(input, what, len, NULL);	
		return ptr.pos; 
	}

	return -1;
}

int32_t CConnectionData::BufferReadln(char *buff)
{
	size_t nLen = 0;
	struct evbuffer * input = bufferevent_get_input(m_pClientTcpEvent);
	char *pszBuff = evbuffer_readln(input, &nLen, EVBUFFER_EOL_LF);
	if(pszBuff) {
		memcpy(buff, pszBuff, nLen);
		FREE(pszBuff);

		return nLen;
	}

	return -1;
}

int32_t CConnectionData::Recv(char *buff, int32_t nRecvLen)
{
	if( m_pClientTcpEvent )
	{
		return bufferevent_read(m_pClientTcpEvent, buff, nRecvLen);
	}
	return -1;
}

int32_t CConnectionData::Send(const char *data, size_t nDataLen)
{
	if(m_pClientTcpEvent)
		return bufferevent_write(m_pClientTcpEvent, data, nDataLen)==0?nDataLen:-1;
	else
		return -1;
}

int32_t CConnectionData::GetFd()
{
	if(m_pClientTcpEvent)
		return int32_t( bufferevent_getfd(m_pClientTcpEvent) );
	else
		return -1;
}

std::string CConnectionData::GetPeerIp()
{
    int32_t nFd = GetFd();
	if (nFd < 0)
		return "";

    struct sockaddr_in addr;
#ifdef _MSC_VER
    int nAddrLen = sizeof(addr);
#else
	socklen_t nAddrLen = sizeof(addr);
#endif
    memset(&addr, 0, nAddrLen);

    int nRes = getpeername(nFd, (struct sockaddr*)(&addr), &nAddrLen );
    if (0!=nRes)
        return "";

    return inet_ntoa(addr.sin_addr);
}

int32_t CConnectionData::GetPeerPort()
{
    int32_t nFd = GetFd();
	if (nFd < 0)
		return 0;

    struct sockaddr_in addr;
#ifdef _MSC_VER
    int nAddrLen = sizeof(addr);
#else
	socklen_t nAddrLen = sizeof(addr);
#endif
    memset(&addr, 0, nAddrLen);

    int nRes = getpeername(nFd, (struct sockaddr*)(&addr), &nAddrLen );
    if (0!=nRes)
        return 0;

    return addr.sin_port;
}

uint32_t CConnectionData::GetEvtbufLetfLen()
{
	uint32_t nLeft = 0;
	if(m_pClientTcpEvent)
	{
		struct evbuffer* input = bufferevent_get_input(m_pClientTcpEvent);
		if (input)
			nLeft = evbuffer_get_length(input);
	}
	return nLeft;
}