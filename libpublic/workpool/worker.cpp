/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   工作线程
*************************************************************************/
#include <chrono>
#include <thread> 
#include "worker.h"
#include "typedef.h"
#include <iostream>

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
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_INTERVAL));
    }

    m_isClosed.store(true);
}

void Worker::Handle()
{
    while(!IsQueueEmpty() && m_isEnable.load()) {
        ShareptrTask ptrTask;
        bool isRet = m_queue.tryPop(ptrTask);
        if(isRet) {
            ptrTask->Execute(this);
        }
    }
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
    if(m_isClosed.load()) {
        while(!IsQueueEmpty()) {
            m_queue.pop();
        }
    }
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