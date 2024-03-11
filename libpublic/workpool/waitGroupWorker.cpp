
#include <thread> 
#include "waitGroupWorker.h"

namespace Sync {

WaitGroupWorker::WaitGroupWorker()
{
   m_isEnable = false;
   m_isClosed = false;
   m_pHandle = nullptr;
   m_taskCompleteStatusFunc = nullptr;
}

WaitGroupWorker::~WaitGroupWorker()
{

}

void WaitGroupWorker::Run()
{
    while (m_isEnable.load())
    {
        Handle();
    }

    m_isClosed.store(true);
}

void WaitGroupWorker::Handle()
{  
    ShareptrTask ptrTask = std::move(m_queue.pop());
    ptrTask->Execute(this);

    if (m_taskCompleteStatusFunc != nullptr) {
        m_taskCompleteStatusFunc(ptrTask->GetStatus());
    }
}

void WaitGroupWorker::AddTask(ShareptrTask pTask)
{
    m_queue.push(pTask);
}

void WaitGroupWorker::Start()
{
    m_isEnable.store(true);
    std::thread th(&WaitGroupWorker::Run, std::ref(*this));
    th.detach();
}

void WaitGroupWorker::Stop()
{
    m_isEnable.store(false);
}

bool WaitGroupWorker::IsClose()
{
    return m_isClosed.load();
}

void WaitGroupWorker::SetIndex(const uint32_t nIndex)
{
    m_nIndex = nIndex;
}

uint32_t WaitGroupWorker::GetIndex()
{
    return m_nIndex;
}

bool WaitGroupWorker::IsQueueEmpty()
{
    return m_queue.empty();
}

size_t WaitGroupWorker::GetQueueSize()
{
    return m_queue.size();
}

void WaitGroupWorker::SetHandle(HandleInterface *handPtr)
{
    HandleInterfacePtr it(handPtr);
    m_pHandle = it;
}

HandleInterface *WaitGroupWorker::GetHandle()
{
    return m_pHandle.get();
}

void WaitGroupWorker::SetHandlePtr(HandleInterfacePtr handPtr)
{
    m_pHandle = handPtr;
}

HandleInterfacePtr WaitGroupWorker::GetHandlePtr()
{
    return m_pHandle;
}

void WaitGroupWorker::SetTaskCompleteStatusFunc(TaskCompleteStatusFunc func)
{
    m_taskCompleteStatusFunc = func;
}

}