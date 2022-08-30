/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   工作线程
*************************************************************************/
#include <chrono>
#include <thread> 
#include "worker.h"
#include "typedef.h"

Worker::Worker()
{
   m_isEnable = false;
   m_isClosed = false;
   m_pHandle = nullptr;
}

Worker::~Worker()
{

}

void Worker::Run()
{
    while (m_isEnable.load())
    {
        Handle();
    }

    m_isClosed.store(true);
}

void Worker::Handle()
{
    ShareptrTask ptrTask = std::move(m_queue.pop());
    ptrTask->Execute(this);
}

void Worker::AddTask(ShareptrTask pTask)
{
    m_queue.push(pTask);
}

void Worker::Start()
{
	m_isEnable.store(true);
    std::thread th(&Worker::Run, std::ref(*this));
    th.detach();
}

void Worker::Stop()
{
    m_isEnable.store(false);
}

void Worker::SetIndex(uint32_t nIndex)
{
    m_nIndex = nIndex;
}

uint32_t Worker::GetIndex()
{
    return m_nIndex;
}

bool Worker::IsQueueEmpty()
{
    return m_queue.empty();
}

void Worker::SetHandle(HandleInterface *handPtr)
{
    m_pHandle = handPtr;
}

HandleInterface *Worker::GetHandle()
{
    return m_pHandle;
}