#pragma once

#include <ai\Path.h>

class AStar
{
public:
	AStar(void);
	virtual ~AStar(void);

	// Finds a path from (x0,y0) to (x1,y1)
	Path *FindPath(int x0, int y0, int x1, int y1);

protected:
	// This is the node structure that determines our path
	struct Node
	{
		int X, Y;

		// The A* heuristics
		long F,G,H;
	
		// Pointers to my parent and my children. We are a rectangular
		// tile based game so we have potentially 8 children
		struct Node *Parent;

		// Pointer to the next node in the list
		struct Node *Next;
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
	long GetTerrainCost(int x, int y);

	// Generates a heuristic for the cost remaining to
	// (_destx, _destY)
	long Heuristic(int x, int y);

	// Adds a node to a sorted list
	void Add(Node **list, Node *node);

	// Finds a node on a list
	Node *Find(Node *list, int x, int y);

	// Removes a node from a list
	Node *Remove(Node **list, int x, int y);

	// Allocates and frees nodes
	Node *AllocateNode();
	void FreeNode(Node *node);

	// Our list of OPEN nodes. This is a sorted list with respect
	// to f
	Node *_openNodes;

	// Out list of CLOSED nodes
	Node *_closedNodes;

	// Our destination nodes
	int _destX, _destY;
};
