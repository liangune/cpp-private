/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   同步队列
*************************************************************************/

#ifndef _SYNC_QUEUE_H_
#define _SYNC_QUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename _Ty>
class SyncQueue {
public:
    //using queue_type = std::queue<_Ty>;
    //using value_type = typename queue_type::value_type;
    SyncQueue()=default;
    SyncQueue(const SyncQueue&)=delete;
    SyncQueue& operator=(const SyncQueue&)=delete;

    // 从队列里面弹出一个元素,队列为空则阻塞
    _Ty pop() 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queueCond.wait(lock, [this]{return !this->m_queue.empty();});
        
        auto value = std::move(m_queue.front());
        m_queue.pop();
        return value;
    }

    // 从队列里面弹出一个元素,队列为空则返回false
    bool tryPop(_Ty &value) 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_queue.empty())
            return false;
        
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    void push(const _Ty &val) 
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        m_queue.push(std::move(val));
        m_queueCond.notify_one();
    }

    bool empty()
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        return m_queue.empty();
    }

    size_t size()
    {
        std::unique_lock<std::mutex> guard(m_mutex);
        return m_queue.size();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_queueCond;
    std::queue<_Ty> m_queue;
};

#endif // !_SYNC_QUEUE_H_