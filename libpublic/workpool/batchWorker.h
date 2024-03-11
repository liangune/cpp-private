#ifndef _BATCH_WORKER_THREAD_H_
#define _BATCH_WORKER_THREAD_H_

#include <memory>
#include <atomic>
#include "batchTaskInterface.h"
#include "../syncQueue.h"
#include "typedef.h"
#include "handleInterface.h"
#include "batchTaskListInterface.h"

class BatchWorker
{
public:
    BatchWorker();
    virtual ~BatchWorker();

    virtual void Run();
    virtual void Handle();

    void AddTask(BatchTaskInterfacePtr p);
    void Start();
    void Stop();

    void SetIndex(const uint32_t nIndex);
    uint32_t GetIndex();
    
    bool IsQueueEmpty();
    size_t GetQueueSize();

    void SetMaxBatch(uint32_t nMaxBatch);
    uint32_t GetMaxBatch();

    void SetWaitTimeout(uint32_t nWaitTimeout);
    uint32_t GetWaitTimeout();

    void SetHandlePtr(HandleInterfacePtr handPtr);
    HandleInterfacePtr GetHandlePtr();

    void SetTaskCompleteFunc(TaskCompleteFunc func);

    void SetBatchTaskListPtr(BatchTaskListInterfacePtr ptr);
    BatchTaskListInterfacePtr GetBatchTaskListPtr();
protected:
    SyncQueue<BatchTaskInterfacePtr> m_queue;
    std::atomic<bool> m_isEnable;
    std::atomic<bool> m_isClosed;
    uint32_t m_nIndex;

    uint32_t m_nMaxBatch;
    uint32_t m_nWaitTimeout;  // 单位毫秒

    HandleInterfacePtr m_pHandle;
    TaskCompleteFunc m_taskCompleteFunc; 

    BatchTaskListInterfacePtr m_taskListInterface;
};

typedef std::shared_ptr<BatchWorker> BatchWorkerPtr;

#endif // !_BATCH_WORKER_THREAD_H_