
/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   PG数据库结果集
*************************************************************************/
#include "PGResult.h"
#include <stdlib.h>
#include <string.h>

CPGResult::CPGResult() : m_nRowIndex(0), m_nRowCount(0), m_pPGResult(NULL)
{

}

CPGResult::~CPGResult()
{
    freeResult();
}

void CPGResult::storeResult(PGresult *res)
{
    m_pPGResult = res;
    m_nRowCount = getRows();
}

PGresult *CPGResult::getResult()
{
    return m_pPGResult;
}

int32_t CPGResult::getRows() const
{
    return PQntuples(m_pPGResult);
}

int32_t CPGResult::getCols() const
{
    return PQnfields(m_pPGResult);
}

bool CPGResult::fetchRow()
{
    if(m_nRowIndex >= m_nRowCount) 
        return false;

    return true;
}

void CPGResult::rowIndexIncrement()
{
    m_nRowIndex++;
}

void CPGResult::freeResult()
{
    if(m_pPGResult) {
        PQclear(m_pPGResult);
    }
}

bool CPGResult::getValue(int nColIndex, std::string &sValue)
{
    if(!fetchRow()) {
        return false;
    }

    if(nColIndex >= getCols()) {
        return false;
    }

    sValue = PQgetvalue(m_pPGResult, m_nRowIndex, nColIndex);
    return true;
}

bool CPGResult::getValue(int nColIndex, int &value)
{
    std::string v;
	if(getValue(nColIndex, v))
	{
		value = atoi(v.c_str());
		return true;
	}
	return false;
}

bool CPGResult::getValue(int nColIndex, bool &value)
{
	std::string v;
	if(getValue(nColIndex, v) )
	{
		value = strieq(v.c_str(), "true");
		return true;
	}
	return false;
}

bool CPGResult::getValue(int nColIndex, float &value)
{
	std::string v;
	if(getValue(nColIndex, v))
	{
		value = atof(v.c_str());
		return true;
	}
	return false;
}

bool CPGResult::getValue(int nColIndex, double &value)
{
	std::string v;
	if(getValue(nColIndex, v))
	{
		value = atof(v.c_str());
		return true;
	}
	return false;
}

bool CPGResult::getValue(int nColIndex, int64_t &value)
{
    std::string v;
	if(getValue(nColIndex, v))
	{
		value = strtoll(v.c_str(), NULL, 10);
		return true;
	}
	return false;
}

bool CPGResult::getValue(int nColIndex, uint32_t &value)
{
    std::string v;
	if(getValue(nColIndex, v))
	{
		value = atoi(v.c_str());
		return true;
	}
	return false;
}

bool CPGResult::getValue(int nColIndex, uint64_t &value)
{
    std::string v;
	if(getValue(nColIndex, v))
	{
		value = strtoull(v.c_str(), NULL, 10);
		return true;
	}
	return false;
}