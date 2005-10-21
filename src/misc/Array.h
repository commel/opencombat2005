#pragma once

#include <stdlib.h>
#include <string.h>
#include <assert.h>

template <class T> class Array
{
public:
	T **Items;
	int Count;

	Array()
	{
		Count = 0;
		_maxItems = 2;
		Items = (T **) calloc(_maxItems, sizeof(T *));
	}

	virtual ~Array()
	{
		free(Items);
		Count = 0;
		_maxItems = 0;
	}

	void Add(T *c)
	{
		if(Count >= _maxItems) {
			// We need to grow the array
			_maxItems *= 2;
			Items = (T **) realloc(Items, _maxItems*sizeof(T *));
			for(int i = Count; i < _maxItems; ++i) {
				Items[i] = NULL;
			}
		}
		Items[Count++] = c;
	}

	void AddAt(T *c, int i)
	{
		if(Count >= _maxItems) {
			// We need to grow the array
			_maxItems *= 2;
			Items = (T **) realloc(Items, _maxItems*sizeof(T *));
			for(int i = Count; i < _maxItems; ++i) {
				Items[i] = NULL;
			}
		}
		// Move everything down
		for(int j = Count; j > i; --j) {
			Items[j] = Items[j-1];
		}
		Items[i] = c;
		++Count;
	}

	T *RemoveAt(int idx)
	{
		if(idx < Count && idx >= 0 && Count > 0) {
			T *rv = Items[idx];
			Items[idx] = NULL;
		
			// Move everything down
			if(idx < (Count-1)) {
				memmove(Items + idx, Items + idx + 1, (Count - (idx + 1))*sizeof(T *));
				Items[Count-1] = NULL;
			}
			--Count;
			return rv;
		}
		return NULL;
	}

	T *Remove(T *c)
	{
		for(int i = 0; i < Count; ++i) {
			if(Items[i]->Equals(c)) {
				return RemoveAt(i);
			}
		}
		return NULL;
	}

	void Clear()
	{
		while(Count > 0) 
		{
			RemoveAt(0);
		}
		assert(Count == 0);
	}

private:
	int _maxItems;
};