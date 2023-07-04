/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   PG数据库结果
*************************************************************************/

#ifndef _PG_RESULT_H_
#define _PG_RESULT_H_

#include <string>
#include "libpq_c/libpq-fe.h"

#ifdef _MSC_VER
	#define strieq(s1, s2) (!stricmp((s1), (s2)))
#else 
	#define strieq(s1, s2) (!strcasecmp((s1), (s2)))
#endif
#define streq(s1, s2)  (!strcmp((s1), (s2)))

class CPGResult 
{
	CPGResult &operator=(const CPGResult&);
 public:
	CPGResult();
	~CPGResult();

	void storeResult(PGresult *res);
    PGresult *getResult();

	int32_t getRows() const;   
	int32_t getCols() const;  

	bool fetchRow();
	void freeResult();
	void rowIndexIncrement();

	bool getValue(int nColIndex, std::string &sValue);
	bool getValue(int nColIndex, int &value);
	bool getValue(int nColIndex, bool &value);
	bool getValue(int nColIndex, float &value);
	bool getValue(int nColIndex, double &value);
	bool getValue(int nColIndex, int64_t &value);
	bool getValue(int nColIndex, uint32_t &value);
	bool getValue(int nColIndex, uint64_t &value);

private:
    uint32_t m_nRowIndex;  // 从0开始
    uint32_t m_nRowCount; 

    PGresult *m_pPGResult;
};

#endif // !_PG_RESULT_H_
