/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   线程任务接口, 由子类实现
*************************************************************************/

#ifndef _WORK_POOL_TASK_INTERFACE_H_
#define _WORK_POOL_TASK_INTERFACE_H_
#include <stdint.h>
#include "typedef.h"

class Worker;
class TaskInterface 
{
public:
	TaskInterface() : m_nIndex(0), m_pExtraArg(nullptr), m_nStatus(TaskStatusOK)  {}
    virtual ~TaskInterface() {}

	void SetIndex(uint32_t nIndex) { 
		m_nIndex = nIndex; 
	}
	uint32_t GetIndex() { 
		return m_nIndex; 
	}
	
	void *GetExtraArg() { 
		return m_pExtraArg; 
	}
	void SetExtraArg(void *pArg) { 
		m_pExtraArg = pArg; 
	}

	TaskStatus GetStatus() { 
		return m_nStatus; 
	}
	void SetStatus(TaskStatus nStatus) {
		m_nStatus = nStatus;
	}

	// 任务执行接口
	virtual void Execute(void *argPtr) = 0;
	
protected:
	uint32_t m_nIndex;
	void *m_pExtraArg;
	TaskStatus m_nStatus;
};

#endif // !_WORK_POOL_TASK_INTERFACE_H_