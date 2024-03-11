#ifndef _WAIT_GROUP_WORKER_MGR_H_
#define _WAIT_GROUP_WORKER_MGR_H_

#include <stdint.h>
#include <vector>
#include "typedef.h"
#include "handleFactory.h"
#include <mutex>
#include "waitGroup.h"
#include "waitGroupWorker.h"

namespace Sync {

class WaitGroupWorkerMgr
{
public:
    using VecWorker = std::vector<WaitGroupWorkerPtr>;
public:
    ~WaitGroupWorkerMgr();
	WaitGroupWorkerMgr();

    void Init(const uint32_t nWorkerCnt, IHandleFactory *pHandleFactory = nullptr);
    void Start();
    void Stop();

    WaitGroupWorkerPtr CreateWorker();
    void InitWaitGroupWorker();

    uint32_t GetWorkerCount();
    void SetWorkerCount(const uint32_t nCount);

    uint32_t GetWorkerMaxTaskCount();
    void SetWorkerMaxTaskCount(const uint32_t nCount);
    void EnableMaxTaskCount();

    uint32_t GetTaskCount();
    uint32_t GetTaskFailedCount();
    
	void AddTask(ShareptrTask pTask);
    void TaskCompleteStatus(TaskStatus nStatus);

    void Reset();
    void ResetTaskFailedCount();
    void ResetTaskCount();
    void Wait();

private:
	uint32_t m_nWorkerCount;
	uint32_t m_nWorkerIndex;

    bool m_isEnableMaxTaskCount;
    uint32_t m_nWorkerMaxTaskCount;
    std::atomic<uint32_t> m_nTaskCount;
    std::atomic<uint32_t> m_nTaskFailedCount;
    
	VecWorker m_vecWorker;
    std::mutex m_mutex;

    IHandleFactory *m_pHandleFactory;
    WaitGroupPtr m_waitGroupPtr;

    std::condition_variable m_queueCond;
    
};

typedef std::shared_ptr<WaitGroupWorkerMgr> WaitGroupWorkerMgrPtr;

}

#endif // !_WAIT_GROUP_WORKER_MGR_H_
