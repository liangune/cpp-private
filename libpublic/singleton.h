#ifndef _SIGLETON_H_
#define _SIGLETON_H_
#include <mutex>

template<typename T>
class CSingleton
{
public:
	CSingleton() {}
	virtual ~CSingleton() {}

    static T *GetInstance();
    static void Destory();
private:
    static T *m_pInstance;
    static std::mutex m_mutex;
};

template<typename T>
T *CSingleton<T>::m_pInstance = nullptr;

template <typename T>
std::mutex CSingleton<T>::m_mutex;

template<typename T>
T *CSingleton<T>::GetInstance() 
{
    m_mutex.lock();
    if (nullptr == m_pInstance) {
        m_pInstance = new T();
    }
    m_mutex.unlock();
    return m_pInstance;
}

template<typename T>
void CSingleton<T>::Destory() 
{
    if (m_pInstance) {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

#endif // !_SIGLETON_H_