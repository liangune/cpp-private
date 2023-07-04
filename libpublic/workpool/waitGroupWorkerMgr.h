#ifndef _WAIT_GROUP_WORKER_MGR_H_
#define _WAIT_GROUP_WORKER_MGR_H_

#include <stdint.h>
#include <vector>
#include "typedef.h"
#include "handleFactory.h"
#include <mutex>
#include "waitGroup.h"
#include "waitGroupWorker.h"

namespace Sync{

class WaitGroupWorkerMgr
{
public:
    using VecWorker = std::vector<WaitGroupWorker*>;
public:
    ~WaitGroupWorkerMgr();
	WaitGroupWorkerMgr();

    void Init(uint32_t nWorkerCnt, WaitGroup * wg, IHandleFactory *pHandleFactory = nullptr);
    void Start();
    void Stop();

    WaitGroupWorker* CreateWorker();
    void InitWaitGroupWorker();

    uint32_t GetWorkerCount();
    void SetWorkerCount(uint32_t nCount);

    uint32_t GetTaskCount();
    uint32_t GetTaskFailedCount();
    
	void AddTask(ShareptrTask pTask);
    void TaskComplete();
    void TaskCompleteStatus(TaskStatus nStatus);
private:
	uint32_t m_nWorkerCount;
	uint32_t m_nWorkerIndex;

    std::atomic<uint32_t> m_nTaskCount;
    std::atomic<uint32_t> m_nTaskFailedCount;
    
	VecWorker m_vecWorker;
    std::mutex m_mutex;

    IHandleFactory *m_pHandleFactory;
    WaitGroup *m_pWaitGroup;
};

}

#endif // !_WAIT_GROUP_WORKER_MGR_H_
