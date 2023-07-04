#ifndef _WAIT_GROUP_WORKER_H_
#define _WAIT_GROUP_WORKER_H_

#include <memory>
#include <atomic>
#include "taskInterface.h"
#include "../syncQueue.h"
#include "typedef.h"
#include "handleInterface.h"

namespace Sync {
    
class WaitGroupWorker
{
public:
    using ShareptrQueue = SyncQueue<std::shared_ptr<TaskInterface>>;
public:
    WaitGroupWorker();
    virtual ~WaitGroupWorker();

    virtual void Run();
    virtual void Handle();

    void AddTask(ShareptrTask p);
    void Start();
    void Stop();

    void SetIndex(uint32_t nIndex);
    uint32_t GetIndex();
    
    bool IsQueueEmpty();
    size_t GetQueueSize();

    void SetHandle(HandleInterface *handPtr);
    HandleInterface *GetHandle();

    void SetTaskCompleteFunc(TaskCompleteFunc func);
    void SetTaskCompleteStatuFunc(TaskCompleteStatusFunc func);
protected:
    ShareptrQueue m_queue;
    std::atomic<bool> m_isEnable;
    std::atomic<bool> m_isClosed;
    uint32_t m_nIndex;
    HandleInterface *m_pHandle;
    TaskCompleteFunc m_taskCompleteFunc; 
    TaskCompleteStatusFunc m_taskCompleteStatusFunc;
};

}
#endif // !_WAIT_GROUP_WORKER_H_