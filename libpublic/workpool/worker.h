/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   工作线程
*************************************************************************/

#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_
#include <memory>
#include <atomic>
#include "taskInterface.h"
#include "../syncQueue.h"
#include "typedef.h"
#include "handleInterface.h"

// 工作线程
class Worker
{
public:
    using ShareptrQueue = SyncQueue<std::shared_ptr<TaskInterface>>;
public:
    Worker();
    virtual ~Worker();

    virtual void Run();
    virtual void Handle();

    void AddTask(ShareptrTask p);
    void Start();
    void Stop();

    void SetIndex(uint32_t nIndex);
    uint32_t GetIndex();
    
    bool IsQueueEmpty();

    void SetHandle(HandleInterface *handPtr);
    HandleInterface *GetHandle();

protected:
    ShareptrQueue m_queue;
    std::atomic<bool> m_isEnable;
    std::atomic<bool> m_isClosed;
    uint32_t m_nIndex;
    HandleInterface *m_pHandle;
};

#endif // !_WORKER_THREAD_H_