/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   线程池
*************************************************************************/
#include "workerPool.h"
#include "workerFactory.h"
#include "handleFactory.h"
#include <iostream>

WorkerPool::WorkerPool()
{
    m_nWorkerIndex = 0;
    m_nWorkerCount = 1;
    m_pWorkerFactory = nullptr;
    m_pHandleFactory = nullptr;
}

WorkerPool::~WorkerPool()
{

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
    std::lock_guard<std::mutex> guard(m_mutex);
    m_nWorkerIndex++;
    if(m_nWorkerIndex >= m_nWorkerCount) {
        m_nWorkerIndex = 0;
    }

    Worker *p = m_vecWorker[m_nWorkerIndex];
    if(p == NULL)
        return;

    pTask->SetIndex(p->GetIndex());
    p->AddTask(pTask);
}   

