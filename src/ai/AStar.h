#pragma once

#include <ai\Path.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <world\Element.h>

/**
 * In our implementation of the A* algorithm, we use a hash table for the
 * Closed list, because we want fast Insertions, Removals, and Queries. For
 * the open list, we use a priority queue for fast insertion and removals from
 * a binary heap, and we also use a hash table for fast queries.
 */
class AStar
{
public:
	AStar(void);
	virtual ~AStar(void);

	// Finds a path from (x0,y0) to (x1,y1), using the method
	// (prone, med, etc) found in level
	//
	// XXX/SGW: Add a flag to optimize for speed/cover/danger etc
	Path *FindPath(int x0, int y0, int x1, int y1, Element::Level level);

protected:
	// This is the node structure that determines our path
	struct Node
	{
		int X, Y;

		// The A* heuristics
		float F,G,H;
	
		// Pointers to my parent and my children. We are a rectangular
		// tile based game so we have potentially 8 children
		struct Node *Parent;

		// The index of this node in the binary heap
		int HeapIndex;
	
		// Pointers to next and previous hash nodes
		struct Node *HashNext, *HashPrev;

		// Pointer to the next node in the memory list
		struct Node *MemNext;
	};

	// Returns the best node off our open list
	Node *GetBestNode();

	// Generates candidate nodes from the given node
	void GenerateSuccessors(Node *node);

	// Can we move to a particular tile?
	bool CanMove(int x, int y);

	// Generate a trial move to a particular tile
	void DoMove(Node *node, int x, int y);

	// Gets the cost of moving to tile (x,y)
	float GetTerrainCost(int x, int y);

	// Generates a heuristic for the cost remaining to
	// (_destx, _destY)
	float Heuristic(int x, int y);

	// Allocates and frees nodes
	Node *AllocateNode();
	void FreeNode(Node *node);

	class Hash
	{
	public:
		Hash(int size, AStar *astar)
		{
			_astar = astar;
			_size = size;
			_nodes = (Node **) calloc(_size, sizeof(Node *));
		}

		~Hash()
		{
		}

		void Insert(Node *node)
		{
			unsigned int idx = HashFunction(node);
			node->HashNext = _nodes[idx];

			if(node->HashNext != NULL)
			{
				node->HashNext->HashPrev = node;
			}
			_nodes[idx] = node;
		}

		Node *Remove(int x, int y)
		{
			unsigned int idx = HashFunction(x,y);
			Node *p = _nodes[idx];
			while(p != NULL)
			{
				if(p->X == x && p->Y == y)
				{
					if(p->HashPrev != NULL)
					{
						p->HashPrev->HashNext = p->HashNext;
						if(p->HashNext != NULL)
						{
							p->HashNext->HashPrev = p->HashPrev;
						}
					}
					else
					{
						_nodes[idx] = p->HashNext;
						if(p->HashNext != NULL)
						{
							p->HashNext->HashPrev = NULL;
						}
					}
					return p;
				}
				p = p->HashNext;
			}
			return NULL;
		}

		Node *Find(int x, int y)
		{
			unsigned int idx = HashFunction(x,y);
			Node *p = _nodes[idx];
			while(p != NULL)
			{
				if(p->X == x && p->Y == y)
				{
					return p;
				}
				p = p->HashNext;
			}
			return NULL;
		}

		void Clear()
		{
			memset(_nodes, 0, _size*sizeof(Node *));
		}

		void DeepClear()
		{
			for(int i = 0; i < _size; ++i)
			{
				while(_nodes[i] != NULL)
				{
					Node *n = _nodes[i];
					_nodes[i] = _nodes[i]->HashNext;
					_astar->FreeNode(n);
				}
			}
		}

		unsigned int HashFunction(Node *n)
		{
			return (n->X << 16 | n->Y) % _size;
		}

		unsigned int HashFunction(int x, int y)
		{
			return (x << 16 | y) % _size;
		}

	private:
		Node **_nodes;
		int _size;
		AStar *_astar;
	};

	// A MinHeap class that we can use
	class MinHeap
	{
	public:
		
		// Some definitions for child checking
		#define Left(i)		((i) << 1)
		#define Right(i)	(((i) << 1) + 1)
		#define Parent(i)	((i) >> 1)

		// Creates a new heap of the given size
		MinHeap(int maxSize, AStar *astar)
		{
			assert (0 < maxSize);
			_astar = astar;
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
			_hash = new Hash(60013,astar);
			_freeHeapNodes = NULL;
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
		int Insert(Node *data, float priority)
		{
			// Create a new node for this data
			HeapNode *newNode = AllocateHeapNode();
			newNode->Priority = priority;
			newNode->Data = data;

			++_last;
  
			// Is the heap full?
			// XXX/GWS: I have written the code here, however, I do not
			//			want these to be dynamic heaps, so I have also
			//			added an assert to make sure we never hit this case
			assert(_last <= _size);
#ifdef USE_DYNAMIC_HEAP		
			if(_last > _size) 
			{
				// Resize heap
				_heapNodes = (HeapNode **)realloc(_heapNodes, sizeof(HeapNode *)*(_size*2 + 1));
				
				if(NULL == _heapNodes) 
				{
					throw "Could not reallocate heap"
				}
				_size = _size*2;
			}
#endif

			int i = _last;
			while((i > 1) && (_heapNodes[Parent(i)]->Priority >= newNode->Priority)) 
			{
				_heapNodes[i] = _heapNodes[Parent(i)];
				_heapNodes[i]->Data->HeapIndex = i;
				i = Parent(i);
			}
		
			_heapNodes[i] = newNode;
			_heapNodes[i]->Data->HeapIndex = i;
			_hash->Insert(data);
			return i;
		}

		// Gets the number of nodes in the heap
		int GetNumNodes()
		{
			return _last;
		}

		// Extracts the node with the max priority from the heap
		Node *ExtractMin()
		{
			if(0 >= _last) {
				throw "Heap is empty";
			}
	
			HeapNode *max = _heapNodes[1];
			_heapNodes[1] = _heapNodes[_last];
			_heapNodes[1]->Data->HeapIndex = 1;
			--_last;
			SiftDown(1);
			_hash->Remove(max->Data->X, max->Data->Y);

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
			Node *n = max->Data;
			FreeHeapNode(max);
			return n;
		}

		// Removes a given index from the heap
		Node *RemoveIndex(int i)
		{
			if(0 >= _last) {
				throw "Heap is empty";
			} else if(i > _last) {
				throw "Index out of bounds";
			}

			HeapNode *rv = _heapNodes[i];
			_heapNodes[i] = _heapNodes[_last];
			_heapNodes[i]->Data->HeapIndex = i;
			--_last;
			SiftDown(i);
			_hash->Remove(rv->Data->X, rv->Data->Y);
			return rv->Data;
		}

		Node *Find(int x, int y)
		{
			return _hash->Find(x, y);
		}

		void Clear()
		{
			// Need to release all of our heap nodes
			for(int i = 1; i <= _last; ++i)
			{
				_astar->FreeNode(_heapNodes[i]->Data);
				_heapNodes[i]->Data = NULL;
				FreeHeapNode(_heapNodes[i]);
			}
			_last = 0;
			_hash->Clear();
		}

	private:	
		struct HeapNode
		{
			float Priority;
			Node *Data;
			HeapNode *MemNext;
		};

		HeapNode *AllocateHeapNode()
		{
			if(_freeHeapNodes == NULL)
			{
				return (HeapNode *) calloc(1, sizeof(HeapNode));
			}
			else
			{
				HeapNode *n = _freeHeapNodes;
				_freeHeapNodes = _freeHeapNodes->MemNext; 
				return n;
			}
		}

		void FreeHeapNode(HeapNode *node)
		{
			node->MemNext = _freeHeapNodes;
			_freeHeapNodes = node;
		}

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
				if((Left(j) <= n) && (_heapNodes[Left(j)]->Priority <= _heapNodes[k]->Priority)) 
				{
					k = Left(j);
				}
				if((Right(j) <= n) && (_heapNodes[Right(j)]->Priority <= _heapNodes[k]->Priority)) 
				{
					k = Right(j);
				}
				_heapNodes[0] = _heapNodes[j];
				_heapNodes[0]->Data->HeapIndex = 0;
				_heapNodes[j] = _heapNodes[k];
				_heapNodes[j]->Data->HeapIndex = j;
				_heapNodes[k] = _heapNodes[0];
				_heapNodes[k]->Data->HeapIndex = k;
			} while (j != k);
		}

		Hash *_hash;
		int _size;
		int _last;
		HeapNode **_heapNodes;
		HeapNode *_freeHeapNodes;
		AStar *_astar;
	}; // end class MinHeap


	// Our list of OPEN nodes. This is a sorted list with respect
	// to f
	MinHeap *_openNodes;

	// Out list of CLOSED nodes
	Hash *_closedNodes;

	// Our destination nodes
	int _destX, _destY;

	// Keep track of free Nodes
	Node *_freeNodes;
	long _nFreeNodes, _nAllocatedNodes;

	// The current level we are using
	Element::Level _level;
};
