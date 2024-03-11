/*************************************************************************
	> Date:   2021-01-06
	> Author: liangjun
	> Desc:   线程工厂
*************************************************************************/

#ifndef _WORKER_FACTORY_H_
#define _WORKER_FACTORY_H_

#include "worker.h"
#include "batchWorker.h"

class IWorkerFactory
{
public:
	IWorkerFactory() {}
	virtual ~IWorkerFactory() {}
	virtual Worker * GetWorker() { return nullptr; };
	virtual WorkerPtr GetWorkerPtr() { return nullptr; };
	virtual BatchWorkerPtr GetBatchWorkerPtr() { return nullptr; };
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

	virtual WorkerPtr GetWorkerPtr()
	{
		WorkerPtr p(std::make_shared<_Tp>());
		return p;
	}
};

template<typename _Tp>
class BatchWorkerFactory : public IWorkerFactory
{
public:
	BatchWorkerFactory() {}
	~BatchWorkerFactory() {}

	virtual BatchWorkerPtr GetBatchWorkerPtr()
	{
		BatchWorkerPtr p(std::make_shared<_Tp>());
		return p;
	}
};

#endif // !_WORKER_FACTORY_H_
