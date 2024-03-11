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
// 线程休眠时间间隔, 单位毫秒
#define DefaultThreadSleepMillisecond 1
// 线程休眠时间间隔, 单位秒
#define DefaultThreadSleepSecond 1
// 线程休眠时间间隔, 单位微秒
#define DefaultThreadSleepMicrosecond 100
// 默认批量处理等待时间, 单位毫秒
#define DefaultBatchWaitTimeout 1000
// 默认批量值
#define DefaultMaxBatch 20

typedef std::shared_ptr<TaskInterface> ShareptrTask;

#define IWORKER_INSTANCE Worker
#define BATCH_WORKER_INSTANCE BatchWorker

// 任务状态
enum TaskStatus {
	TaskStatusOK = 0,
	TaskStatusFailed = 1,
};

using TaskCompleteFunc = std::function<void()>;
using TaskHashFunc = std::function<int32_t(const std::string&, int32_t)>;
using TaskCompleteStatusFunc = std::function<void(TaskStatus)>;

#endif // !_WORK_POOL_TYPEDEF_H_