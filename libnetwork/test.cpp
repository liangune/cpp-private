#include "./libtcp/tcpMaster.h"
#include "./libtcp/connection.h"
#include "./libtcp/connectionFactory.h"

#include <iostream>
#include <string>
#include <stdint.h>
#include <stdio.h>

class CParserTest: public CConnection
{
public:
	CParserTest() {}
	~CParserTest() {}

public:
	void Init() {}

	virtual void OnRecv()
	{
		std::cout << "CParserTest OnRead: \n";
		std::string sRep = "reply Hi";
		int32_t nDataLen = GetBuffTotalLen();
		if( nDataLen > 0)
		{
			char *pBuff = new char[nDataLen+1];
			if( this->Recv(pBuff, nDataLen) > 0)
			{
				printf("%s\n", pBuff);
				this->Send(sRep.c_str(), sRep.length()+1);
			}
			delete []pBuff;
		}
	}
	virtual void OnSend()
	{
		printf("OnSend\n");
	}

	virtual void OnDealError()
	{
		printf("OnSend\n");
	}
};

int main()
{
#ifdef _MSC_VER
    std::string sFilePrefix = ".\\";
#else
    std::string sFilePrefix = "./";
#endif

	std::string sZlogCate = "log_server";
	std::string sZlogPath = sFilePrefix + "zlog_windows.conf";

	CTcpMaster theServer;
	theServer.InitMasterThread(new CConnectionFactory<CParserTest>(), sZlogPath.c_str(), sZlogCate.c_str(), 9100, 2, 1024);
	theServer.Start();

	return 0;
}
