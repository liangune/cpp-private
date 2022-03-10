/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   线程任务例子
*************************************************************************/

#ifndef _WORK_POOL_TASK_EXAMPLE_H_
#define _WORK_POOL_TASK_EXAMPLE_H_

#include <string>
#include "taskInterface.h"
#include <iostream>

class TaskExample : public TaskInterface
{
public:
    void Execute(void *argPtr) { std::cout << "task: " << m_sStr << std::endl; }

    void SetValue(std::string &str) { m_sStr = str; }

    std::string m_sStr;
};

#endif // !_WORK_POOL_TASK_EXAMPLE_H_