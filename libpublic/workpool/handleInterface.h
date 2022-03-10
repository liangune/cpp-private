/*************************************************************************
	> Date:   2020-07-14
	> Author: liangjun
	> Desc:   处理接口
*************************************************************************/

#ifndef _WORK_POOL_HANDLE_INTERFACE_H_
#define _WORK_POOL_HANDLE_INTERFACE_H_
#include "typedef.h"

class HandleInterface 
{
public:
	HandleInterface() {}
    virtual ~HandleInterface() {}

	// 执行接口
	virtual void Execute(ShareptrTask pTask) {};
	virtual void Init() {};

};

#endif // !_WORK_POOL_HANDLE_INTERFACE_H_
