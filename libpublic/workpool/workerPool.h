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
	void AddTask(ShareptrTask pTask);

private:
	uint32_t m_nWorkerCount;
	uint32_t m_nWorkerIndex;
    
	VecWorker m_vecWorker;
    std::mutex m_mutex;

    IWorkerFactory *m_pWorkerFactory;
    IHandleFactory *m_pHandleFactory;
};

#endif // !_WORKER_POOL_H_
