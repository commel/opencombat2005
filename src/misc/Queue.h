#pragma once

#include <stdlib.h>
#include <misc\Array.h>

template <class T> class Queue
{
public:
	Queue()
	{
	}

	virtual ~Queue()
	{
	}

	void Enqueue(T *c)
	{
		_array.Add(c);
	}

	T *Dequeue()
	{
		if(_array.Count > 0) {
			T *rv = _array.Items[0];
			_array.RemoveAt(0);
			return rv;
		}
		return NULL;
	}

	T *Peek()
	{
		if(_array.Count > 0) {
			T *rv = _array.Items[0];
			return rv;
		}
		return NULL;
	}

	void Insert(T *t, int i) 
	{
		_array.AddAt(t, i);
	}

	void Clear()
	{
		while(_array.Count > 0) {
			_array.RemoveAt(0);
		}
	}

	int Count()
	{
		return _array.Count;
	}

private:
	Array<T> _array;
};