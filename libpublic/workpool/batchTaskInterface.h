#ifndef _BATCH_TASK_INTERFACE_H_
#define _BATCH_TASK_INTERFACE_H_

#include <memory>

class BatchTaskInterface {
public:
    BatchTaskInterface() {}
    virtual ~BatchTaskInterface() {}

    int32_t GetType() {
        return nType;
    }

	void SetIndex(uint32_t nIndex) { 
		m_nIndex = nIndex; 
	}
	uint32_t GetIndex() { 
		return m_nIndex; 
	}

protected:
    int32_t nType;
	uint32_t m_nIndex;
};

typedef std::shared_ptr<BatchTaskInterface> BatchTaskInterfacePtr;

#endif // _BATCH_TASK_INTERFACE_H_