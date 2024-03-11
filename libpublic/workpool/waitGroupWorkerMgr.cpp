
#include "waitGroupWorkerMgr.h"

namespace Sync {

WaitGroupWorkerMgr::WaitGroupWorkerMgr()
{
    m_nWorkerIndex = 0;
    m_nWorkerCount = 1;
    m_isEnableMaxTaskCount = false;
    m_nWorkerMaxTaskCount = DefaultWorkerMaxTaskCount;
    m_nTaskCount = 0;
    m_nTaskFailedCount = 0;
    m_pHandleFactory = nullptr;
    m_waitGroupPtr = nullptr;
}

WaitGroupWorkerMgr::~WaitGroupWorkerMgr()
{
    if (m_pHandleFactory != nullptr) {
        delete m_pHandleFactory;
        m_pHandleFactory = nullptr;
    }
    Stop();
}

void WaitGroupWorkerMgr::Init(const uint32_t nWorkerCnt, IHandleFactory *pHandleFactory)
{
    m_nWorkerCount = nWorkerCnt;
    m_pHandleFactory = pHandleFactory;
    m_waitGroupPtr = std::make_shared<WaitGroup>();
    InitWaitGroupWorker();
}

uint32_t WaitGroupWorkerMgr::GetWorkerCount()
{
    return m_nWorkerCount;
}

void WaitGroupWorkerMgr::SetWorkerCount(const uint32_t nCount)
{
    m_nWorkerCount = nCount;
}

uint32_t WaitGroupWorkerMgr::GetWorkerMaxTaskCount()
{
    return m_nWorkerMaxTaskCount;
}

void WaitGroupWorkerMgr::SetWorkerMaxTaskCount(const uint32_t nCount)
{
    m_nWorkerMaxTaskCount = nCount;
}

void WaitGroupWorkerMgr::EnableMaxTaskCount()
{
    m_isEnableMaxTaskCount = true;
}

uint32_t WaitGroupWorkerMgr::GetTaskCount()
{
    return m_nTaskCount.load();
}

uint32_t WaitGroupWorkerMgr::GetTaskFailedCount()
{
    return m_nTaskFailedCount.load();
}

WaitGroupWorkerPtr WaitGroupWorkerMgr::CreateWorker()
{
	WaitGroupWorkerPtr p = std::make_shared<WaitGroupWorker>();
	return p;
}

void WaitGroupWorkerMgr::InitWaitGroupWorker()
{
    for(uint32_t i = 0; i < m_nWorkerCount; i++)
    {
        WaitGroupWorkerPtr p = CreateWorker();
        p->SetIndex(i);
        
        if(m_pHandleFactory != nullptr) {
            HandleInterfacePtr handPtr = m_pHandleFactory->GetHandlePtr();
            handPtr->Init();
            p->SetHandlePtr(handPtr);
        }

        auto f = std::bind(&WaitGroupWorkerMgr::TaskCompleteStatus, this, std::placeholders::_1);
        p->SetTaskCompleteStatusFunc(f);
        m_vecWorker.push_back(p);
    }
}

void WaitGroupWorkerMgr::Start()
{
    for(uint32_t i = 0; i < m_nWorkerCount; i++)
    {
        WaitGroupWorkerPtr p = m_vecWorker[i];
        p->Start();
    }
}

void WaitGroupWorkerMgr::Stop()
{
    for(auto it = m_vecWorker.begin(); it != m_vecWorker.end();)
    {
        WaitGroupWorkerPtr p = *it;
        p->Stop();
        it = m_vecWorker.erase(it); 
    }
}

void WaitGroupWorkerMgr::AddTask(ShareptrTask pTask)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_nWorkerIndex++;
    if(m_nWorkerIndex >= m_nWorkerCount) {
        m_nWorkerIndex = 0;
    }

    WaitGroupWorkerPtr p = m_vecWorker[m_nWorkerIndex];
    if(p == NULL)
        return;

    if (m_isEnableMaxTaskCount) {    
        m_queueCond.wait(lock, [this]{return !(this->m_nTaskCount.load() > m_nWorkerMaxTaskCount * m_nWorkerCount);});
    }

    pTask->SetIndex(p->GetIndex());
    p->AddTask(pTask);
    m_nTaskCount++;

    if (m_waitGroupPtr != nullptr) {
        m_waitGroupPtr->Add();
    }
}

void WaitGroupWorkerMgr::TaskCompleteStatus(TaskStatus nStatus)
{
    m_nTaskCount--;

    if (m_waitGroupPtr != nullptr) {
        m_waitGroupPtr->Done();
    }

    if (nStatus == TaskStatusFailed) {
        m_nTaskFailedCount++;
    }

    if (m_isEnableMaxTaskCount) {   
        m_queueCond.notify_one();
    }
}

void WaitGroupWorkerMgr::Reset()
{
    ResetTaskFailedCount();
    ResetTaskCount();
    m_waitGroupPtr->Reset();
}

void WaitGroupWorkerMgr::ResetTaskFailedCount()
{
    m_nTaskFailedCount = 0;
}

void WaitGroupWorkerMgr::ResetTaskCount()
{
    m_nTaskCount = 0;
}

void WaitGroupWorkerMgr::Wait()
{
    if (m_waitGroupPtr != nullptr) {
        m_waitGroupPtr->Wait();
    }
}

}