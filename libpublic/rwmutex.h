/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   读写锁
*************************************************************************/

/* 
 * 读写锁 c++14以上使用std::shared_mutex
*/

#ifndef _RW_MUTEX_H_
#define _RW_MUTEX_H_

/*******************************************************************/
/*
#include <mutex>  // For std::unique_lock
#include <shared_mutex>
 
class ThreadSafeCounter {
 public:
  ThreadSafeCounter() = default;
 
  // Multiple threads/readers can read the counter's value at the same time.
  unsigned int get() const {
    std::shared_lock lock(mutex_);
    return value_;
  }
 
  // Only one thread/writer can increment/write the counter's value.
  void increment() {
    std::unique_lock lock(mutex_);
    value_++;
  }
 
  // Only one thread/writer can reset/write the counter's value.
  void reset() {
    std::unique_lock lock(mutex_);
    value_ = 0;
  }
 
 private:
  mutable std::shared_mutex mutex_;
  unsigned int value_ = 0;
};
*/
/*******************************************************************/

#include <mutex>
#include <condition_variable>
#include <map>

class RWMutex 
{
public:
    RWMutex() = default;
    ~RWMutex() = default;

    void rLock() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condRead.wait(lock, [this]()-> bool { return m_writeCount == 0; });
        ++m_readCount;
    }

    void rUnLock() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (--m_readCount == 0 && m_writeCount > 0) {
            m_condWrite.notify_one();
        }
    }

    void lock() {
        std::unique_lock<std::mutex> lock(m_mutex);
        ++m_writeCount;
        m_condWrite.wait(lock, [this]()-> bool { return m_readCount == 0 && !m_writing; });
        m_writing = true;
    }

    void unLock() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (--m_writeCount == 0) {
            m_condRead.notify_all();
        } else {
            m_condWrite.notify_one();
        }
        m_writing = false;
    }

private:
    volatile size_t m_readCount = 0;
    volatile size_t m_writeCount = 0;
    volatile bool m_writing = false;
    std::mutex m_mutex;
    std::condition_variable m_condRead;
    std::condition_variable m_condWrite;
};

#ifdef READ_WRITE_LOCK_CONFLICT
namespace rwlock {
#endif
template<typename _RWMutex>
class ReadLock
{
public:
    explicit ReadLock(_RWMutex &rwMutex) : m_pmtx(&rwMutex) {
        m_pmtx->rLock();
    }

    ~ReadLock() {
        if (m_pmtx) {
            m_pmtx->rUnLock();
        }
    }
private:
    _RWMutex *m_pmtx = nullptr;
};

template<typename _RWMutex>
class WriteLock
{
public:
    explicit WriteLock(_RWMutex &rwMutex) : m_pmtx(&rwMutex) {
        m_pmtx->lock();
    }

    ~WriteLock() {
        if (m_pmtx) {
            m_pmtx->unLock();
        }
    }
private:
    _RWMutex * m_pmtx = nullptr;
};
#ifdef READ_WRITE_LOCK_CONFLICT
}
#endif

#endif // !_RW_MUTEX_H_