#include <thread>
#include "batchWorker.h"
#include "../datetime.h"
#include "batchTaskListInterface.h"

BatchWorker::BatchWorker()
{
   m_isEnable = false;
   m_isClosed = false;
   m_nMaxBatch = DefaultMaxBatch;
   m_nWaitTimeout = DefaultBatchWaitTimeout;
}

BatchWorker::~BatchWorker()
{

}

void BatchWorker::Run()
{
    while (m_isEnable.load())
    {
        Handle();
    }

    m_isClosed.store(true);
}

void BatchWorker::Handle()
{
    if (IsQueueEmpty()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(DefaultThreadSleepMillisecond));
		return;
	}

    if(m_taskListInterface == nullptr) {
		return;
	}

    CDatetime startTime;
    uint64_t nStarTime = startTime.getMillisecond();
    uint64_t nEndTime = nStarTime;
	while (nEndTime - nStarTime < m_nWaitTimeout && m_taskListInterface->Size() < m_nMaxBatch)
	{
        BatchTaskInterfacePtr ptrObject;
		bool isRet = m_queue.tryPop(ptrObject);
		if (isRet) {
      		m_taskListInterface->AddBatchTask(ptrObject);
		} else {
			std::this_thread::sleep_for(std::chrono::microseconds(DefaultThreadSleepMicrosecond));
		}
		
        CDatetime endTime;
        nEndTime = endTime.getMillisecond();
    }

    size_t nSize = m_taskListInterface->Size();
	if(nSize > 0) {
		std::string sJson;
		m_taskListInterface->ExecuteBatch(this);

        for(size_t i = 0; i < nSize; i++) {
            if (m_taskCompleteFunc != nullptr) {
                m_taskCompleteFunc();
            }
        }
        
        m_taskListInterface->Clear();
	}
}

void BatchWorker::AddTask(BatchTaskInterfacePtr pTask)
{
    m_queue.push(pTask);
}

void BatchWorker::Start()
{
	m_isEnable.store(true);
    std::thread th(&BatchWorker::Run, std::ref(*this));
    th.detach();
}

void BatchWorker::Stop()
{
    m_isEnable.store(false);
}

void BatchWorker::SetIndex(const uint32_t nIndex)
{
    m_nIndex = nIndex;
}

uint32_t BatchWorker::GetIndex()
{
    return m_nIndex;
}

bool BatchWorker::IsQueueEmpty()
{
    return m_queue.empty();
}

size_t BatchWorker::GetQueueSize()
{
    return m_queue.size();
}

void BatchWorker::SetMaxBatch(uint32_t nMaxBatch)
{
    if (nMaxBatch <= 0) {
        return;
    }
    m_nMaxBatch = nMaxBatch;
}

uint32_t BatchWorker::GetMaxBatch() {
    return m_nMaxBatch;
}

void BatchWorker::SetWaitTimeout(uint32_t nWaitTimeout)
{
    if (nWaitTimeout <= 0) {
        return;
    }
    m_nWaitTimeout = nWaitTimeout;
}

uint32_t BatchWorker::GetWaitTimeout()
{
    return m_nWaitTimeout;
}

void BatchWorker::SetHandlePtr(HandleInterfacePtr handPtr)
{
    m_pHandle = handPtr;
}

HandleInterfacePtr BatchWorker::GetHandlePtr()
{
    return m_pHandle;
}

void BatchWorker::SetTaskCompleteFunc(TaskCompleteFunc func)
{
    m_taskCompleteFunc = func;
}

void BatchWorker::SetBatchTaskListPtr(BatchTaskListInterfacePtr ptr)
{
    m_taskListInterface = ptr;
}

BatchTaskListInterfacePtr BatchWorker::GetBatchTaskListPtr()
{
    return m_taskListInterface;
}