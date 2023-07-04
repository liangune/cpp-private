/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   线程池
*************************************************************************/

#ifndef _WORKER_POOL_H_
#define _WORKER_POOL_H_

#include <stdint.h>
#include <vector>
#include "worker.h"
#include "typedef.h"
#include <mutex>

class IWorkerFactory;
class IHandleFactory;
class WorkerPool 
{
public:
    using VecWorker = std::vector<Worker*>;
public:
    ~WorkerPool();
	WorkerPool();

    void Init(IWorkerFactory *pWorkerFactory, uint32_t nWorkerCnt, IHandleFactory *pHandleFactory = nullptr);
    void Start();
    void Stop();

    uint32_t GetWorkerCount();
    void SetWorkerCount(uint32_t nCount);

    uint32_t GetWorkerMaxTaskCount();
    void SetWorkerMaxTaskCount(uint32_t nCount);
    void EnableMaxTaskCount();
    uint32_t GetTaskCount();
    
    void SetTaskHashFunc(TaskHashFunc func);

	void AddTask(ShareptrTask pTask);
    void AddTask(const std::string sHashKey, ShareptrTask pTask);
    void TaskComplete();
private:
	uint32_t m_nWorkerCount;
	uint32_t m_nWorkerIndex;

    bool m_isEnableMaxTaskCount;
    uint32_t m_nWorkerMaxTaskCount;
    std::atomic<uint32_t> m_nTaskCount;
    
	VecWorker m_vecWorker;
    std::mutex m_mutex;

    IWorkerFactory *m_pWorkerFactory;
    IHandleFactory *m_pHandleFactory;

    std::condition_variable m_queueCond;

    TaskHashFunc m_taskHashFunc;
};

#endif // !_WORKER_POOL_H_
