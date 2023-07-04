#ifndef _WAIT_GROUP_H_
#define _WAIT_GROUP_H_

#include <mutex>
#include <atomic>
#include <condition_variable>

namespace Sync {

class WaitGroup {

public:
    WaitGroup() {
        m_nCounter = 0;
    }

    void Add(int count = 1) {
        m_nCounter += count;
    }

    void Done() {
        m_nCounter--;
        if (m_nCounter.load() <= 0) {
            m_cond.notify_all();
        }
    }

    int GetCount() {
        return m_nCounter.load();
    }

    void Wait() {
        if (m_nCounter.load() <= 0) {
            return;
        }
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [&]() {return this->m_nCounter.load() <= 0;});
    }

private:
    std::mutex m_mutex;
    std::atomic<int> m_nCounter;
    std::condition_variable m_cond;
};

}

#endif // !_WAIT_GROUP_H_