/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   回调
*************************************************************************/

#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include <functional>
#include <memory>

typedef void (*CallbackFunc)(void *);
using CallbackFunctor = std::function<void(void *)>;

class Callback {
public: 
    Callback() : m_pFunc(nullptr), m_pArg(nullptr), m_stdFunc(nullptr) { }
    Callback(CallbackFunc func, void *arg) { 
        m_pFunc = func;
        m_pArg = arg;
        m_stdFunc = nullptr; 
    }

    Callback(const CallbackFunctor func, void *arg) {
        m_stdFunc = func; 
        m_pArg = arg; 
        m_pFunc = nullptr;
    }

    virtual ~Callback() { }

    void Execute() { 
        if (m_pFunc) {
            m_pFunc(m_pArg);
        }

        if (m_stdFunc) {
            m_stdFunc(m_pArg);
        }
    }
public:
    CallbackFunc m_pFunc;
    void *m_pArg;

    // c++11
    CallbackFunctor m_stdFunc;
};

using CallbackPtr = std::shared_ptr<Callback>;

#endif // !_CALLBACK_H_