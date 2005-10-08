#pragma once

#include <misc\Array.h>

template <class T> class Stack
{
public:
	Stack(void) {}
	virtual ~Stack(void) {}

	void Push(T *c)
	{
		_list.Add(c);
	}

	T *Pop()
	{
		if(_list.Count > 0) {
			return _list.RemoveAt(_list.Count-1);
		} else {
			return NULL;
		}
	}

	T *Peek()
	{
		if(_list.Count > 0) {
			return _list.Items[_list.Count-1];
		} else {
			return NULL;
		}
	}

private:
	Array<T> _list;
};
