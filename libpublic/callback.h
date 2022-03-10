/*************************************************************************
	> Date:   2020-12-08
	> Author: liangjun
	> Desc:   回调
*************************************************************************/

#ifndef _CALLBACK_H_
#define _CALLBACK_H_

typedef void (*CallbackFunc)(void *);
class Callback {
public: 
    Callback(CallbackFunc func, void *arg) { m_pFunc = func, m_pArg = arg;}
    virtual ~Callback() {};
public:
    CallbackFunc m_pFunc;
    void *m_pArg;
};

#endif // !_CALLBACK_H_