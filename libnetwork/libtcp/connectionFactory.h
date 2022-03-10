/*************************************************************************
	> Date:   2020-12-22
	> Author: liangjun
	> Desc:   解析器
*************************************************************************/

#ifndef _TCP_PARSER_FACTORY_H_
#define _TCP_PARSER_FACTORY_H_
#include "connection.h"
#include <stdint.h>

class IConnectionFactory
{
public:
	IConnectionFactory(){}
	virtual ~IConnectionFactory(){}
	virtual CConnection * GetConnection(const uint32_t nIndex, const int64_t nAcceptTm) = 0;
};
//
template<typename _Tp>
class CConnectionFactory: public IConnectionFactory
{
public:
	CConnectionFactory()
	{

	}
	~CConnectionFactory()
	{

	}
	virtual CConnection * GetConnection(const uint32_t nIndex, const int64_t nAcceptTm)
	{
		CConnection * p = new _Tp();
		p->SetIndex(nIndex);
		p->SetAcceptTm(nAcceptTm);
		p->Init();
		return p;
	}

};
#endif /* _TCP_PARSER_FACTORY_H_ */
