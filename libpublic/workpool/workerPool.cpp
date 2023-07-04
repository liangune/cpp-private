/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   线程池
*************************************************************************/
#include "workerPool.h"
#include "workerFactory.h"
#include "handleFactory.h"

WorkerPool::WorkerPool()
{
    m_nWorkerIndex = 0;
    m_nWorkerCount = 1;
    m_pWorkerFactory = nullptr;
    m_pHandleFactory = nullptr;
    m_isEnableMaxTaskCount = false;
    m_nWorkerMaxTaskCount = DefaultWorkerMaxTaskCount;
    m_nTaskCount = 0;
    m_taskHashFunc = nullptr;
}

WorkerPool::~WorkerPool()
{
    if (m_pHandleFactory != nullptr) {
        delete m_pHandleFactory;
        m_pHandleFactory = nullptr;
    }

    if (m_pWorkerFactory != nullptr) {
        delete m_pWorkerFactory;
        m_pWorkerFactory = nullptr;
    }
    Stop();
}

void WorkerPool::Init(IWorkerFactory *pWorkerFactory, uint32_t nWorkerCnt, IHandleFactory *pHandleFactory)
{
    m_nWorkerCount = nWorkerCnt;
    m_pWorkerFactory = pWorkerFactory;
    m_pHandleFactory = pHandleFactory;
}

uint32_t WorkerPool::GetWorkerCount()
{
    return m_nWorkerCount;
}

void WorkerPool::SetWorkerCount(uint32_t nCount)
{
    m_nWorkerCount = nCount;
}

uint32_t WorkerPool::GetWorkerMaxTaskCount()
{
    return m_nWorkerMaxTaskCount;
}

void WorkerPool::SetWorkerMaxTaskCount(uint32_t nCount)
{
    m_nWorkerMaxTaskCount = nCount;
}

void WorkerPool::EnableMaxTaskCount()
{
    m_isEnableMaxTaskCount = true;
}

uint32_t WorkerPool::GetTaskCount()
{
    return m_nTaskCount.load();
}

void WorkerPool::TaskComplete()
{
    m_nTaskCount--;
    m_queueCond.notify_one();
}

void WorkerPool::SetTaskHashFunc(TaskHashFunc func)
{
    m_taskHashFunc = func;
}

void WorkerPool::Start()
{
    for(uint32_t i = 0; i < m_nWorkerCount; i++)
    {
        Worker *p = m_pWorkerFactory->GetWorker();
        p->SetIndex(i);
        m_vecWorker.push_back(p);
        if(m_pHandleFactory != nullptr) {
            HandleInterface *handPtr = m_pHandleFactory->GetHandle();
            handPtr->Init();
            p->SetHandle(handPtr);
        }
        if (m_isEnableMaxTaskCount) {
            auto f = std::bind(&WorkerPool::TaskComplete, this);
            p->SetTaskCompleteFunc(f);
        }
        p->Start();
    }
}

void WorkerPool::Stop()
{
    for(uint32_t i = 0; i < m_nWorkerCount; i++)
    {
        Worker *p = m_vecWorker[i];
        p->Stop();
        delete p;
    }
}

void WorkerPool::AddTask(ShareptrTask pTask)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_nWorkerIndex++;
    if(m_nWorkerIndex >= m_nWorkerCount) {
        m_nWorkerIndex = 0;
    }

    Worker *p = m_vecWorker[m_nWorkerIndex];
    if(p == NULL)
        return;

    if (m_isEnableMaxTaskCount) {    
        m_queueCond.wait(lock, [this]{return !(this->m_nTaskCount.load() > m_nWorkerMaxTaskCount * m_nWorkerCount);});
        m_nTaskCount++;
    }

    pTask->SetIndex(p->GetIndex());
    p->AddTask(pTask);
}

void WorkerPool::AddTask(const std::string sHashKey, ShareptrTask pTask)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    Worker *p = nullptr;
    if (m_taskHashFunc != nullptr) {
        uint32_t nWorkerIndex = m_taskHashFunc(sHashKey, m_nWorkerCount);
        if(nWorkerIndex >= m_nWorkerCount) {
            nWorkerIndex = 0;
        }
        p = m_vecWorker[nWorkerIndex];
    } else {
        m_nWorkerIndex++;
        if(m_nWorkerIndex >= m_nWorkerCount) {
            m_nWorkerIndex = 0;
        }

        p = m_vecWorker[m_nWorkerIndex];
    }

    if(p == NULL)
        return;

    if (m_isEnableMaxTaskCount) {    
        m_queueCond.wait(lock, [this]{return !(this->m_nTaskCount.load() > m_nWorkerMaxTaskCount * m_nWorkerCount);});
        m_nTaskCount++;
    }

    pTask->SetIndex(p->GetIndex());
    p->AddTask(pTask);
}