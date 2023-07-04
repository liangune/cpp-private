
#include "waitGroupWorkerMgr.h"

namespace Sync {

WaitGroupWorkerMgr::WaitGroupWorkerMgr()
{
    m_nWorkerIndex = 0;
    m_nWorkerCount = 1;
    m_pHandleFactory = nullptr;
    m_pWaitGroup = nullptr;
    m_nTaskCount = 0;
    m_nTaskFailedCount = 0;
}

WaitGroupWorkerMgr::~WaitGroupWorkerMgr()
{
    if (m_pHandleFactory != nullptr) {
        delete m_pHandleFactory;
        m_pHandleFactory = nullptr;
    }
    Stop();
}

void WaitGroupWorkerMgr::Init(uint32_t nWorkerCnt, WaitGroup * wg, IHandleFactory *pHandleFactory)
{
    m_nWorkerCount = nWorkerCnt;
    m_pHandleFactory = pHandleFactory;
    m_pWaitGroup = wg;
    InitWaitGroupWorker();
}

uint32_t WaitGroupWorkerMgr::GetWorkerCount()
{
    return m_nWorkerCount;
}

void WaitGroupWorkerMgr::SetWorkerCount(uint32_t nCount)
{
    m_nWorkerCount = nCount;
}

uint32_t WaitGroupWorkerMgr::GetTaskCount()
{
    return m_nTaskCount.load();
}

uint32_t WaitGroupWorkerMgr::GetTaskFailedCount()
{
    return m_nTaskFailedCount.load();
}

void WaitGroupWorkerMgr::TaskComplete()
{
    m_nTaskCount--;
    if (m_pWaitGroup != nullptr) {
        m_pWaitGroup->Done();
    }
}

void WaitGroupWorkerMgr::TaskCompleteStatus(TaskStatus nStatus)
{
    if (nStatus == TaskStatusFailed) {
        m_nTaskFailedCount++;
    }
}

WaitGroupWorker* WaitGroupWorkerMgr::CreateWorker()
{
	WaitGroupWorker* p = new WaitGroupWorker();
	return p;
}

void WaitGroupWorkerMgr::InitWaitGroupWorker()
{
    for(uint32_t i = 0; i < m_nWorkerCount; i++)
    {
        WaitGroupWorker *p = CreateWorker();
        p->SetIndex(i);
        m_vecWorker.push_back(p);
        if(m_pHandleFactory != nullptr) {
            HandleInterface *handPtr = m_pHandleFactory->GetHandle();
            handPtr->Init();
            p->SetHandle(handPtr);
        }

        auto f1 = std::bind(&WaitGroupWorkerMgr::TaskComplete, this);
        p->SetTaskCompleteFunc(f1);

        auto f2 = std::bind(&WaitGroupWorkerMgr::TaskCompleteStatus, this, std::placeholders::_1);
        p->SetTaskCompleteStatuFunc(f2);
    }
}

void WaitGroupWorkerMgr::Start()
{
    for(uint32_t i = 0; i < m_nWorkerCount; i++)
    {
        WaitGroupWorker *p = m_vecWorker[i];
        p->Start();
    }
}

void WaitGroupWorkerMgr::Stop()
{
    for(uint32_t i = 0; i < m_nWorkerCount; i++)
    {
        WaitGroupWorker *p = m_vecWorker[i];
        p->Stop();
        delete p;
    }
}

void WaitGroupWorkerMgr::AddTask(ShareptrTask pTask)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_nWorkerIndex++;
    if(m_nWorkerIndex >= m_nWorkerCount) {
        m_nWorkerIndex = 0;
    }

    WaitGroupWorker *p = m_vecWorker[m_nWorkerIndex];
    if(p == NULL)
        return;

    pTask->SetIndex(p->GetIndex());
    p->AddTask(pTask);
    m_nTaskCount++;

    if (m_pWaitGroup != nullptr) {
        m_pWaitGroup->Add();
    }
}

}