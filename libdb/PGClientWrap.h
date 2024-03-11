/*************************************************************************
	> Date:   2020-12-09
	> Author: liangjun
	> Desc:   PG数据库连接操作
*************************************************************************/

#ifndef _PG_CLIENT_WRAP_H_
#define _PG_CLIENT_WRAP_H_

#include "PGConnectionPool.h"

class CPGClientWrap
{
public:
    CPGClientWrap(std::string &host, std::string &port, std::string &username, std::string &password, std::string &database, uint32_t nTimeout = CPGClient::m_nDefaultTimeout, uint32_t nSSL=PG_DISABLE_SSL)
	{
        m_pConnItem = NULL;
        m_pPGClient = NULL;
        CPGConnectionPool::Connection conn = CPGConnectionPool::getInstance()->getConnection(host, port, username, password, database, nTimeout, nSSL);
        if(conn.pConnItem) {
            m_pConnItem = conn.pConnItem;
            m_pPGClient = &(m_pConnItem->PGClient);
            m_sConnectionURL = conn.sConnectionURL;
        } else {
            m_sError = conn.sError;
            m_sConnectionURL = conn.sConnectionURL;
        }
    }

    ~CPGClientWrap() {
        if(m_pConnItem) {
            CPGConnectionPool::getInstance()->returnConnection(m_pConnItem); 
        }
    }

    bool isVaild() {
        return (m_pPGClient != NULL);
    }

    CPGClient *operator->() {
        return m_pPGClient;
    }

    std::string getErrorMessage() const
    {
        return m_sError;
    }

    std::string getConnectionURL() const 
    {
        return m_sConnectionURL;
    }

private:
    CPGConnectionPool::ConnItem *m_pConnItem;
    CPGClient *m_pPGClient;
    std::string m_sError;
    std::string m_sConnectionURL;
};

#endif //!_PG_CLIENT_WRAP_H_
