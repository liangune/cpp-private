/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   类型和宏定义
*************************************************************************/

#ifndef _WORK_POOL_TYPEDEF_H_
#define _WORK_POOL_TYPEDEF_H_

#include <memory>
#include <functional>

class TaskInterface; 
// 线程池每个线程队列任务最大数量
#define DefaultWorkerMaxTaskCount 1000  
typedef std::shared_ptr<TaskInterface> ShareptrTask;

#define IWORKER_INSTANCE Worker

// 任务状态
enum TaskStatus {
	TaskStatusOK = 0,
	TaskStatusFailed = 1,
};

using TaskCompleteFunc = std::function<void()>;
using TaskHashFunc = std::function<int32_t(const std::string&, int32_t)>;
using TaskCompleteStatusFunc = std::function<void(TaskStatus)>;

#endif // !_WORK_POOL_TYPEDEF_H_