/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   PG数据库客户端
*************************************************************************/

#ifndef _PG_CLIENT_H_
#define _PG_CLIENT_H_

#include <string>
#include "libpq_c/libpq-fe.h"
#include "PGResult.h"

class CPGClient {
public:
	CPGClient();
	~CPGClient();

public:	
	bool connect(std::string &host, std::string &port, std::string &username, std::string &password, std::string &database);
	void close();
	// 重连
	bool reConnect(); 
	std::string getConninfo(std::string &host, std::string &port, std::string &username, std::string &password, std::string &dbName);

	std::string getErrorMessage() const;
	PGconn *getConn() const;

	bool getStatus();
	bool ping();
	std::string getPGPingMsg(PGPing val);
	bool exec(const char *command, CPGResult &res);
	
	size_t escapeString(char *to, const char *from, size_t length);

	void setTimeout(int nTimeout);

public:
	static const int m_nDefaultTimeout = 10; //秒

private:
	PGconn *m_pPGConn;
	uint32_t m_nTimeout;

	std::string m_sError;

	std::string m_sHost;
	std::string m_sPort;
	std::string m_sUser;
	std::string m_sPassword;
	std::string m_sDbName;
};

#endif // !_PG_CLIENT_H_