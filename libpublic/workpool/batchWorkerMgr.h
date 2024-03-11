#ifndef _BATCH_WORKER_MGR_H_
#define _BATCH_WORKER_MGR_H_

#include <stdint.h>
#include <vector>
#include "batchWorker.h"
#include "typedef.h"
#include <mutex>

class IWorkerFactory;
class IHandleFactory;
class IBatchTaskListFactory;

class BatchWorkerMgr 
{
public:
    using VecWorker = std::vector<BatchWorkerPtr>;
public:
    ~BatchWorkerMgr();
	BatchWorkerMgr();

    void Init(IWorkerFactory *pWorkerFactory, uint32_t nWorkerCnt, IBatchTaskListFactory *pBatchTaskListFactory, IHandleFactory *pHandleFactory = nullptr);
    void Start();
    void Stop();

    uint32_t GetWorkerCount();
    void SetWorkerCount(const uint32_t nCount);

    uint32_t GetWorkerMaxTaskCount();
    void SetWorkerMaxTaskCount(const uint32_t nCount);
    void EnableMaxTaskCount();
    uint32_t GetTaskCount();
    
    void SetTaskHashFunc(TaskHashFunc func);
    void TaskComplete();

	void AddTask(BatchTaskInterfacePtr pTask);
    void AddTask(const std::string sHashKey, BatchTaskInterfacePtr pTask);

    void SetMaxBatch(uint32_t nMaxBatch);
    uint32_t GetMaxBatch();

    void SetWaitTimeout(uint32_t nWaitTimeout);
    uint32_t GetWaitTimeout();

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
    IBatchTaskListFactory *m_pBatchTaskListFactory;

    std::condition_variable m_queueCond;

    TaskHashFunc m_taskHashFunc;

    uint32_t m_nMaxBatch;
    uint32_t m_nWaitTimeout;  // 单位毫秒
};

typedef std::shared_ptr<BatchWorkerMgr> BatchWorkerMgrPtr;

#endif // !_BATCH_WORKER_MGR_H_