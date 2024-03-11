/*************************************************************************
	> Date:   2021-07-14
	> Author: liangjun
	> Desc:   handle工厂
*************************************************************************/

#ifndef _WORK_POOL_HANDLE_FACTORY_H_
#define _WORK_POOL_HANDLE_FACTORY_H_

#include "handleInterface.h"

class IHandleFactory
{
public:
	IHandleFactory() {}
	virtual ~IHandleFactory() {}
	virtual HandleInterface * GetHandle() = 0;
	virtual HandleInterfacePtr GetHandlePtr() = 0;
};

template<typename _Tp>
class HandleFactory : public IHandleFactory
{
public:
	HandleFactory() {}
	~HandleFactory() {}
	virtual HandleInterface * GetHandle()
	{
		HandleInterface *handPtr = new _Tp();
		return handPtr;
	}

	virtual HandleInterfacePtr GetHandlePtr()
	{
		HandleInterfacePtr handPtr(std::make_shared<_Tp>());
		return handPtr;
	}
};

#endif // !_WORK_POOL_HANDLE_FACTORY_H_
