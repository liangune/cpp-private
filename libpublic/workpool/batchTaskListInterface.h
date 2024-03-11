#ifndef _BATCH_TASK_LIST_INTERFACE_H_
#define _BATCH_TASK_LIST_INTERFACE_H_

#include <vector>
#include "batchTaskInterface.h"

class BatchTaskListInterface {
public:
    BatchTaskListInterface() {}
    virtual ~BatchTaskListInterface() {}

    virtual void AddBatchTask(BatchTaskInterfacePtr task) {
        m_vecBatchTaskObject.emplace_back(std::move(task));
    }

    size_t Size() {
        return m_vecBatchTaskObject.size();
    }

    void Clear() {
        m_vecBatchTaskObject.clear();
    }

    virtual void ExecuteBatch(void *argPtr) {};

protected:
	std::vector<BatchTaskInterfacePtr> m_vecBatchTaskObject;
};

typedef std::shared_ptr<BatchTaskListInterface> BatchTaskListInterfacePtr;

#endif // !_BATCH_TASK_LIST_INTERFACE_H_