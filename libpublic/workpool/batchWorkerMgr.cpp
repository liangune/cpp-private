#include "batchWorkerMgr.h"
#include "workerFactory.h"
#include "handleFactory.h"
#include "batchTaskListFactory.h"

BatchWorkerMgr::BatchWorkerMgr()
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

BatchWorkerMgr::~BatchWorkerMgr()
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
    m_vecWorker.clear();
}

void BatchWorkerMgr::Init(IWorkerFactory *pWorkerFactory, uint32_t nWorkerCnt, IBatchTaskListFactory *pBatchTaskListFactory, IHandleFactory *pHandleFactory)
{
    m_nWorkerCount = nWorkerCnt;
    m_pWorkerFactory = pWorkerFactory;
    m_pBatchTaskListFactory = pBatchTaskListFactory;
    m_pHandleFactory = pHandleFactory;
}

void BatchWorkerMgr::Start()
{
    for(uint32_t i = 0; i < m_nWorkerCount; i++)
    {
        BatchWorkerPtr p = m_pWorkerFactory->GetBatchWorkerPtr();
        p->SetIndex(i);
        
        if(m_pHandleFactory != nullptr) {
            HandleInterfacePtr handPtr = m_pHandleFactory->GetHandlePtr();
            handPtr->Init();
            p->SetHandlePtr(handPtr);
        }

        if (m_isEnableMaxTaskCount) {
            auto f = std::bind(&BatchWorkerMgr::TaskComplete, this);
            p->SetTaskCompleteFunc(f);
        }

        if(m_pBatchTaskListFactory != nullptr) {
            BatchTaskListInterfacePtr taskListPtr = m_pBatchTaskListFactory->GetBatchTaskListPtr();
            p->SetBatchTaskListPtr(taskListPtr);
        }
        p->SetMaxBatch(m_nMaxBatch);
        p->SetWaitTimeout(m_nWaitTimeout);
        
        p->Start();
        m_vecWorker.push_back(p);
    }
}

void BatchWorkerMgr::Stop()
{
    for(uint32_t i = 0; i < m_nWorkerCount; i++)
    {
        BatchWorkerPtr p = m_vecWorker[i];
        p->Stop();
    }
}

uint32_t BatchWorkerMgr::GetWorkerCount()
{
    return m_nWorkerCount;
}

void BatchWorkerMgr::SetWorkerCount(const uint32_t nCount)
{
    m_nWorkerCount = nCount;
}

uint32_t BatchWorkerMgr::GetWorkerMaxTaskCount()
{
    return m_nWorkerMaxTaskCount;
}

void BatchWorkerMgr::SetWorkerMaxTaskCount(const uint32_t nCount)
{
    m_nWorkerMaxTaskCount = nCount;
}

void BatchWorkerMgr::EnableMaxTaskCount()
{
    m_isEnableMaxTaskCount = true;
}

uint32_t BatchWorkerMgr::GetTaskCount()
{
    return m_nTaskCount.load();
}

void BatchWorkerMgr::SetTaskHashFunc(TaskHashFunc func)
{
    m_taskHashFunc = func;
}

void BatchWorkerMgr::TaskComplete()
{
    m_nTaskCount--;
    if (m_isEnableMaxTaskCount) {  
        m_queueCond.notify_one();
    }
}

void BatchWorkerMgr::AddTask(BatchTaskInterfacePtr pTask)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_nWorkerIndex++;
    if(m_nWorkerIndex >= m_nWorkerCount) {
        m_nWorkerIndex = 0;
    }

    BatchWorkerPtr p = m_vecWorker[m_nWorkerIndex];
    if(p == NULL)
        return;

    if (m_isEnableMaxTaskCount) {    
        m_queueCond.wait(lock, [this]{return !(this->m_nTaskCount.load() > m_nWorkerMaxTaskCount * m_nWorkerCount);});
        m_nTaskCount++;
    }

    pTask->SetIndex(p->GetIndex());
    p->AddTask(pTask);
}

void BatchWorkerMgr::AddTask(const std::string sHashKey, BatchTaskInterfacePtr pTask)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    BatchWorkerPtr p = nullptr;
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



void BatchWorkerMgr::SetMaxBatch(uint32_t nMaxBatch)
{
    m_nMaxBatch = nMaxBatch;
}

uint32_t BatchWorkerMgr::GetMaxBatch() {
    return m_nMaxBatch;
}

void BatchWorkerMgr::SetWaitTimeout(uint32_t nWaitTimeout)
{
    m_nWaitTimeout = nWaitTimeout;
}

uint32_t BatchWorkerMgr::GetWaitTimeout()
{
    return m_nWaitTimeout;
}
