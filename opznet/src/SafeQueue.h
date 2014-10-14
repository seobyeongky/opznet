#pragma once

#include "Thread.h"

#include <deque>

namespace opznet
{
/*��Ƽ ������ ȯ�濡�� �����ϰ� ��� ������ ť�Դϴ�.*/
template <typename T>
class opzSafeQueue
{
public:
	opzSafeQueue();
	~opzSafeQueue();

	void	Push(const T &chunk);
	bool	Pop(T *chunkPtr);
	bool	Peek(T *chunkPtr) const;

	bool	CheckEmpty() const {return _deque.empty(); }
	UINT	GetSize() const {return _deque.size(); };

private:
	std::deque<T>	_deque;
	opzMutex		_mutex;
};

template <typename T>
opzSafeQueue<T>::opzSafeQueue()
	: _deque(),
	_mutex()
{
}

template <typename T>
opzSafeQueue<T>::~opzSafeQueue()
{
}

template <typename T>
void opzSafeQueue<T>::Push(const T& chunk)
{
	opzLock lock(_mutex);
	_deque.push_front(chunk);	
}

template <typename T>
bool opzSafeQueue<T>::Pop(T *chunkPtr)
{
	if(Peek(chunkPtr) == false)
		return false;
	else
	{
		opzLock lock(_mutex);
		_deque.pop_back();
		return true;
	}
}

template <typename T>
bool opzSafeQueue<T>::Peek(T *chunkPtr) const
{
	opzLock lock(const_cast<opzMutex &>(_mutex));

	if(_deque.empty())
	{
		assert(!L"Queue is empty!! Peek�� ����");
		return false;
	}

	assert(chunkPtr != nullptr);

	*chunkPtr = _deque[_deque.size() - 1];

	return true;
}
}