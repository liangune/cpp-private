/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   类型和宏定义
*************************************************************************/

#ifndef _WORK_POOL_TYPEDEF_H_
#define _WORK_POOL_TYPEDEF_H_

#include <memory>
#include "taskInterface.h"

// 线程休眠时间间隔, 单位毫秒
#define THREAD_SLEEP_INTERVAL 10
typedef std::shared_ptr<TaskInterface> ShareptrTask;

#define IWORKER_INSTANCE Worker

#endif // !_WORK_POOL_TYPEDEF_H_