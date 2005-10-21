#pragma once

#include <stdlib.h>
#include <string.h>
#include <assert.h>

// This class defines a queue of actions, where each action is defined
// by an integer index
class ActionQueue
{
public:
	ActionQueue() 
	{
		_length = 1;
		_data = (int *)calloc(_length, sizeof(int));
		_firstItem = 0;
		_lastItem = 0;
		_nItems = 0;
	}

	~ActionQueue() 
	{
		if(_data != 0)
		{
			free(_data);
			_data = 0;
		}
	}

	// Enqueues a new action idx
	void Enqueue(int actionIdx)
	{
		// Let's see if we have enough space to add another index
		if(_firstItem == _lastItem && _nItems > 0)
		{
			// We need more space
			int oldLength = _length;
			int newLength = _length << 1;
			int toMove = oldLength-_lastItem-1;
			_data = (int *)realloc(_data, newLength*sizeof(int));
			if(toMove > 0)
			{
				memmove(_data+newLength-toMove, _data+_lastItem+1, toMove*sizeof(int));
			}
			_firstItem = newLength-toMove-1;
			_length = newLength;
		}
		_data[_firstItem] = actionIdx;
		--_firstItem;
		if(_firstItem < 0)
		{
			_firstItem += _length;
		}
		++_nItems;
	}

	// Dequeues the first element
	bool Dequeue(int *rv)
	{
		if(_nItems > 0)
		{
			*rv = _data[_lastItem];
			--_lastItem;
			if(_lastItem < 0)
			{
				_lastItem += _length;
			}
			--_nItems;
			return true;
		}
		else
		{
			return false;
		}
	}

	// Peeks at the first element
	bool Peek(int *rv)
	{
		if(_nItems < 0)
		{
			*rv = _data[_lastItem];
			return true;
		}
		else 
		{
			return false;
		}
	}

	// Perform a self test of this module
	static bool SelfTest()
	{
		ActionQueue q1;

		for(int i = 0; i < 10; ++i)
		{
			q1.Enqueue(i);
		}
		
		int j = 0;
		int rv = -1;
		while(q1.Dequeue(&rv))
		{
			assert(rv == (j++));
		}
		assert(j == 10);

		// Now do it again
		assert(q1._nItems == 0);
		for(int i = 0; i < 17; ++i)
		{
			q1.Enqueue(i);
		}
		
		j = 0;
		rv = -1;
		while(q1.Dequeue(&rv))
		{
			assert(rv == (j++));
		}
		assert(j == 17);
		assert(q1._nItems == 0);
		return true;
	}

protected:
	int *_data;
	int _length;
	int _firstItem;
	int _lastItem;
	int _nItems;
};
