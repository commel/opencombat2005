#pragma once

#include <assert.h>

// This class implements the min heap code as described in CLRS "Introduction To
// Algorithms".
template <class T> class MinHeap
{
public:
	// Creates a new heap of the given size
	MinHeap(int maxSize)
	{
		assert (0 < maxSize);
		_heapNodes = (HeapNode **) calloc (maxSize+1, sizeof(HeapNode *));
		if(NULL == _heapNodes) 
		{
			_size = 0;
			_last = 0;
		}
		else 
		{
			_size = maxSize;
			_last = 0;
		}
	}

	// Destroys this heap
	~MinHeap()
	{
		if(NULL != _heapNodes) {
			while(0 < _last) {
				if(NULL != _heapNodes[_last]) {
					free(_heapNodes[_last]);
				}
				--_last;
			}
			free(_heapNodes);
		}
	}

	// Inserts a new node into the heap. Returns the index at which
	// the node was inserted
	int Insert(T data, int priority)
	{
		// Create a new node for this data
		HeapNode *newNode = (HeapNode *) calloc(1, sizeof(HeapNode));
		newNode->Priority = priority;
		newNode->Data = data;

		++_last;
  
		// Is the heap full?
		// XXX/GWS: I have written the code here, however, I do not
		//			want these to be dynamic heaps, so I have also
		//			added an assert to make sure we never hit this case
#ifdef USE_DYNAMIC_HEAP		
		if(_last > _size) 
		{
			// Resize heap
			_heapNodes = (HeapNode **)realloc(_heapNodes, sizeof(HeapNode *)*(_size*2 + 1));
			if(NULL == _heapNodes) 
			{
				fprintf(stderr, "Could not insert into heap, memory reallocation error.\n");
				return -1;
			}
			_size = _size*2;
		}
#endif

		int i = _last;
		while((i > 1) && (_heapNodes[Parent(i)]->Priority > newNode->Priority)) 
		{
			_heapNodes[i] = _heapNodes[Parent(i)];
			i = Parent(i);
		}
		
		_heapNodes[i] = newNode;
		return i;
	}

	// Gets the number of nodes in the heap
	int GetNumNodes()
	{
		return _last;
	}

	// Extracts the node with the max priority from the heap
	T ExtractMin()
	{
		if(0 >= _last) {
			throw "Heap is empty";
		}

		HeapNode *max = _heapNodes[1];
		_heapNodes[1] = _heapNodes[_last];
		--_last;
		SiftDown(1);
  
		// Resize the heap if it is less than a quarter full.
		// XXX/GWS: I don't want to use this part of the code.
		//			so I have assert'd it out for now. In the future,
		//			I might relax these constraints and allow it
		//			to occur
#ifdef USE_DYNAMIC_HEAP
		if((_last*4) < _size) 
		{
			// Resize heap to a half
			_heapNodes = (HeapNode **) realloc(_heapNodes, (1 + _size/2)*sizeof(HeapNode *));
			if(NULL == _heapNodes) 
			{
				throw "Error resizing heap";
			}
		}
#endif
		return max->Data;
	}

	// Removes a given index from the heap
	T RemoveIndex(int i)
	{
		if(0 >= _last) {
			throw "Heap is empty";
		} else if(i > _last) {
			throw "Index out of bounds";
		}

		HeapNode *rv = _heapNodes[i];
		_heapNodes[i] = _heapNodes[_last];
		--_last;
		SiftDown(i);
		return rv->Data;
	}

private:
	// Some definitions for child checking
	#define Left(i)		((i) << 1)
	#define Right(i)	(((i) << 1) + 1)
	#define Parent(i)	((i) >> 1)
	
	struct HeapNode
	{
		long Priority;
		T Data;
	};

	// This function is the iterative version of Heapify()
	void SiftDown(int i)
	{
		int k, j, n;

		assert (1 <= i);

		k = i;
		n = _last;
		do 
		{
			j = k;
			if((Left(j) <= n) && (_heapNodes[Left(j)]->Priority < _heapNodes[k]->Priority)) 
			{
				k = Left(j);
			}
			if((Right(j) <= n) && (_heapNodes[Right(j)]->Priority < _heapNodes[k]->Priority)) 
			{
				k = Right(j);
			}
			_heapNodes[0] = _heapNodes[j];
			_heapNodes[j] = _heapNodes[k];
			_heapNodes[k] = _heapNodes[0];
		} while (j != k);
	}

	int _size;
	int _last;
	HeapNode **_heapNodes;
};
