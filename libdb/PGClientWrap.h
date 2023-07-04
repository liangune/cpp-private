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
    CPGClientWrap(std::string &host, std::string &port, std::string &username, std::string &password, std::string &database, uint32_t nTimeout = CPGClient::m_nDefaultTimeout) 
	{
        m_pConnItem = NULL;
        m_pPGClient = NULL;
        m_pConnItem = CPGConnectionPool::getInstance()->getConnection(host, port, username, password, database, nTimeout);
        if(m_pConnItem) {
            m_pPGClient = &(m_pConnItem->PGClient);
        } else {
            m_sError = "can not get connection from PGConnectionPool";
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

private:
    CPGConnectionPool::ConnItem *m_pConnItem;
    CPGClient *m_pPGClient;
    std::string m_sError;
};

#endif //!_PG_CLIENT_WRAP_H_
