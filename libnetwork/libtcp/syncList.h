/*************************************************************************
	> Date:   2020-12-24
	> Author: liangjun
	> Desc:   线程同步链表
*************************************************************************/

#ifndef _SYNC_LIST_H_
#define _SYNC_LIST_H_
#include <stdint.h>
#include <mutex>

template<typename _Tp>
class CSyncList
{
public:
	struct ListNode
	{
		_Tp * pData;
		ListNode *next;
		ListNode *prev;
	};
public:
	CSyncList(): m_first(NULL), m_last(NULL), m_nCount(0)
	{

	}
	~CSyncList()
	{
	}

	void AddTail(ListNode * newNode)
	{
		AddTail((ListNode *)newNode);
	}

	void AddTail(_Tp * newNode)
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		if( m_first )
		{
			m_last->next = newNode;
			newNode->prev = m_last;
		}
		else
		{
			newNode->prev = NULL;
			m_first = newNode;
		}
		newNode->next = NULL;
		m_last = newNode;
		++m_nCount;
	}

	_Tp * GetHead()
	{
		_Tp * pRet = NULL;
		std::lock_guard<std::mutex> guard(m_mutex);
		pRet = m_first;
		return pRet;
	}

	_Tp * GetTail()
	{
		_Tp * pRet = NULL;
		std::lock_guard<std::mutex> guard(m_mutex);
		pRet = m_last;
		return pRet;
	}

	_Tp * GetHeadAndRemove()
	{
		_Tp * pRet = NULL;

		std::lock_guard<std::mutex> guard(m_mutex);
		if( m_first )
		{
			pRet = m_first;
			if( m_first == m_last )
				m_last = NULL;
			else
			{
				pRet->next->prev = NULL;
			}
			m_first = pRet->next;
			--m_nCount;
		}
		return pRet;
	}

	/**
	 * @param istNode [在该节点之前插入新节点]
	 * @param newNode [需要插入的新节点]
	 */
	void Insert(_Tp *istNode, _Tp *newNode)
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		if( m_first == istNode)
		{
			newNode->prev = NULL;
			if( m_first )
			{
				newNode->next = m_first;
				m_first->prev = newNode;
			}
			else
			{
				newNode->next = NULL;
				m_last = newNode;
			}
			m_first = newNode;
		}
		else if( NULL == istNode)
		{
			if( m_first )
			{
				m_last->next = newNode;
				newNode->prev = m_last;
			}
			else
			{
				newNode->prev = NULL;
				m_first = newNode;
			}
			newNode->next = NULL;
			m_last = newNode;
		}
		else
		{
			newNode->next = istNode;
			newNode->prev = istNode->prev;
			istNode->prev->next = newNode;
			istNode->prev = newNode;
		}
		++m_nCount;
	}

	size_t GetCount()
	{
		size_t nCount = 0;
		std::lock_guard<std::mutex> guard(m_mutex);
		nCount = m_nCount;
		return nCount;
	}

	void Clear()
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		m_first = NULL;
		m_last  = NULL;
		m_nCount = 0;
	}

private:
	_Tp * m_first;
	_Tp * m_last;
	size_t m_nCount;
	std::mutex m_mutex;
};
#endif /* _SYNC_LIST_H_ */

