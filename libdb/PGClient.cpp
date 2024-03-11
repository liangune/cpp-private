/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   PG数据库客户端
*************************************************************************/
#include "PGClient.h"
#include <string>
#include <string.h>
#include "url.h"

CPGClient::CPGClient()
{
	m_nTimeout = m_nDefaultTimeout; 
}

CPGClient::~CPGClient()
{

}

bool CPGClient::connect(std::string &host, std::string &port, std::string &username, std::string &password, std::string &dbName, uint32_t SSL)
{
	m_sHost = host;
	m_sPort = port;
	m_sUser = username;
	m_sPassword = password;
	m_sDbName = dbName;
	m_nSSL = SSL;

	std::string sConninfo = getConninfo(m_sHost, m_sPort, m_sUser, m_sPassword, m_sDbName, m_nSSL);

	m_pPGConn = PQconnectdb(sConninfo.c_str());
	if (PQstatus(m_pPGConn) != CONNECTION_OK){
		m_sError = PQerrorMessage(m_pPGConn);
		close();
		return false;
	}

	return true;
}

std::string CPGClient::getConninfo(std::string &host, std::string &port, std::string &username, std::string &password, std::string &dbName, uint32_t SSL)
{
	std::string sTimeout = std::to_string(m_nTimeout); 

	if (SSL == PG_ENABLE_SSL) {
		m_sUrl = "postgresql://" + host + ":" + port + "/" + dbName +"?" 
				+ "user=" + CUrl::escapeUrl(username) + "&password=" + CUrl::escapeUrl(password)
				+  "&connect_timeout=" + sTimeout
				+  "&sslmode=require";
	} else {
		m_sUrl = "postgresql://" + host + ":" + port + "/" + dbName +"?" 
				+ "user=" + CUrl::escapeUrl(username) + "&password=" + CUrl::escapeUrl(password)
				+  "&connect_timeout=" + sTimeout
				+  "&sslmode=disable";
	}

	return m_sUrl;
}

bool CPGClient::reConnect()
{
	std::string sConninfo = getConninfo(m_sHost, m_sPort, m_sUser, m_sPassword, m_sDbName, m_nSSL);

	m_pPGConn = PQconnectdb(sConninfo.c_str());
	if (PQstatus(m_pPGConn) != CONNECTION_OK){
		m_sError = PQerrorMessage(m_pPGConn);
		close();
		return false;
	}

	return true;
}

void CPGClient::close()
{
	if(m_pPGConn) {
		PQfinish(m_pPGConn);
		m_pPGConn = NULL;
	}
}

PGconn *CPGClient::getConn() const
{
	return m_pPGConn;
}

std::string CPGClient::getErrorMessage() const
{
	return m_sError;
}

std::string CPGClient::getConnectionURL() const 
{
	return m_sUrl;
}

bool CPGClient::getStatus() 
{
	ConnStatusType ret = PQstatus(m_pPGConn);
	if (ret != CONNECTION_OK) {
		return false;
	}

	return true;
}

bool CPGClient::ping()
{
	std::string sConninfo = getConninfo(m_sHost, m_sPort, m_sUser, m_sPassword, m_sDbName);
	PGPing ret = PQping(sConninfo.c_str());
	if(ret != PQPING_OK) {
		m_sError = getPGPingMsg(ret);
		return false;
	}
	return true;
}

std::string CPGClient::getPGPingMsg(PGPing val) 
{
	//PQPING_OK,					/* server is accepting connections */
	//PQPING_REJECT,				/* server is alive but rejecting connections */
	//PQPING_NO_RESPONSE,			/* could not establish connection */
	//PQPING_NO_ATTEMPT			    /* connection not attempted (bad params) */

	switch (val) {
	case PQPING_OK: return "PQPING_OK";
	case PQPING_REJECT: return "PQPING_REJECT";
	case PQPING_NO_RESPONSE: return "PQPING_NO_RESPONSE";
	case PQPING_NO_ATTEMPT: return "PQPING_NO_ATTEMPT";
	default:return"UNKNOWN";
	}
}

bool CPGClient::exec(const char *command, CPGResult &res)
{
	if(!m_pPGConn) {
		m_sError = "invaild connection";
		return false;
	}

	if(!command) {
		m_sError = "command is null";
		return false;
	}

	PGresult *pgRes = PQexec(m_pPGConn, command);
	ExecStatusType status = PQresultStatus(pgRes);
	if(status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK) {
		m_sError = PQerrorMessage(m_pPGConn);

		return false;
	}

	res.storeResult(pgRes);
	return true;
}

size_t CPGClient::escapeString(std::string &to, const std::string &from)
{
	size_t length = from.length();
	size_t toLength = length*2+1;
	char *pszTo = new char[toLength];
    memset(pszTo, 0, toLength);
	size_t nSize = PQescapeString(pszTo, from.c_str(), length);
	to = std::string(pszTo, nSize);
	if (pszTo) {
		delete[] pszTo;
	}
	return nSize;
}

void CPGClient::setTimeout(int nTimeout)
{
    m_nTimeout = nTimeout;
}