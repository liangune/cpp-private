#ifndef _BATCH_TASK_LIST_FACTORY_H_
#define _BATCH_TASK_LIST_FACTORY_H_

#include "batchTaskListInterface.h"

class IBatchTaskListFactory
{
public:
	IBatchTaskListFactory() {}
	virtual ~IBatchTaskListFactory() {}
	virtual BatchTaskListInterfacePtr GetBatchTaskListPtr() = 0;
};

template<typename _Tp>
class BatchTaskListFactory : public IBatchTaskListFactory
{
public:
	BatchTaskListFactory() {}
	~BatchTaskListFactory() {}

	virtual BatchTaskListInterfacePtr GetBatchTaskListPtr()
	{
		BatchTaskListInterfacePtr ptr(std::make_shared<_Tp>());
		return ptr;
	}
};

#endif // !_WORK_POOL_HANDLE_FACTORY_H_
