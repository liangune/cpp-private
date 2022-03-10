/*************************************************************************
	> Date:   2021-01-06
	> Author: liangjun
	> Desc:   线程工厂
*************************************************************************/

#ifndef _WORKER_FACTORY_H_
#define _WORKER_FACTORY_H_

#include "worker.h"

class IWorkerFactory
{
public:
	IWorkerFactory() {}
	virtual ~IWorkerFactory() {}
	virtual Worker * GetWorker() = 0;
};

template<typename _Tp>
class WorkerFactory : public IWorkerFactory
{
public:
	WorkerFactory() {}
	~WorkerFactory() {}
	virtual Worker * GetWorker()
	{
		Worker *p = new _Tp();
		return p;
	}
};

#endif // !_WORKER_FACTORY_H_
