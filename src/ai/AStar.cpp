#include ".\astar.h"
#include <assert.h>
#include <stdlib.h>
#include <application\Globals.h>

AStar::AStar(void)
{
	_openNodes = NULL;
	_closedNodes = NULL;
	_destX = -1;
	_destY = -1;
}

AStar::~AStar(void)
{
}

AStar::Node *
AStar::AllocateNode()
{
	return (Node *)calloc(1, sizeof(Node));
}

void
AStar::FreeNode(Node *node)
{
	free(node);
}

long
AStar::Heuristic(int x, int y)
{
	// Straight line distance to our destination. We can add better
	// Heuristics later
	return (x-_destX)*(x-_destX)+(y-_destY)*(y-_destY);
}

Path *
AStar::FindPath(int x0, int y0, int x1, int y1)
{
	Node *bestNode = NULL;

	// Let's remember our destination
	_destX = x1;
	_destY = y1;
	_openNodes = NULL;
	_closedNodes = NULL;

	// We need to create our initial node and add it to the open
	// list
	Node *node = AllocateNode();
	node->G = 0;
	node->H = Heuristic(x0, y0);
	node->F = node->G + node->H;
	node->X = x0;
	node->Y = y0;
	_openNodes = node;
	
	for(;;)
	{
		if(NULL == _openNodes) {
			// There are no more nodes to check, so the destination
			// is unreachable.
			return NULL;
		}

		// Get the best node
		bestNode = GetBestNode();

		// Are we done yet?
		if(bestNode->X == x1 && bestNode->Y == y1)
		{
			break;
		}

		// Let's look at all the successors to this node
		GenerateSuccessors(bestNode);
	}

	// Track backwards and create our path!
	Path *path = AllocatePath();
	path->X = bestNode->X;
	path->Y = bestNode->Y;
	while(bestNode->Parent != NULL) {
		bestNode = bestNode->Parent;
		Path *p = AllocatePath();
		p->X = bestNode->X;
		p->Y = bestNode->Y;
		p->Next = path;
		path = p;
	}
	return path;
}

AStar::Node *
AStar::GetBestNode()
{
	Node *tmp;
	assert(_openNodes != NULL);
	tmp = _openNodes;
	_openNodes = _openNodes->Next;
	tmp->Next = _closedNodes;
	_closedNodes = tmp;
	return tmp;
}

void
AStar::GenerateSuccessors(Node *node)
{
	int x, y;

	// Upper left
	x = node->X-1; y = node->Y-1;
	if(CanMove(x,y)) {
		DoMove(node, x, y);
	}
	// Upper
	x = node->X; y = node->Y-1;
	if(CanMove(x,y)) {
		DoMove(node, x, y);
	}
	// Upper right
	x = node->X+1; y = node->Y-1;
	if(CanMove(x,y)) {
		DoMove(node, x, y);
	}

	// Left
	x = node->X-1; y = node->Y;
	if(CanMove(x,y)) {
		DoMove(node, x, y);
	}
	// Right
	x = node->X+1; y = node->Y;
	if(CanMove(x,y)) {
		DoMove(node, x, y);
	}

	// Lower left
	x = node->X-1; y = node->Y+1;
	if(CanMove(x,y)) {
		DoMove(node, x, y);
	}
	// Lower
	x = node->X; y = node->Y+1;
	if(CanMove(x,y)) {
		DoMove(node, x, y);
	}
	// Lower right
	x = node->X+1; y = node->Y+1;
	if(CanMove(x,y)) {
		DoMove(node, x, y);
	}
}

bool
AStar::CanMove(int x, int y)
{
	// Check the elements file to see if (x,y) is passable
	if(x < 0 || y < 0 || x >= g_Globals->World.CurrentWorld->NumTiles.x || y >= g_Globals->World.CurrentWorld->NumTiles.y) {
		return false;
	}

	// XXX/GWS: Make sure we don't go off the edge of the map!!!
	return g_Globals->World.CurrentWorld->IsPassable(x,y);
}

void
AStar::DoMove(Node *node, int x, int y)
{
	Node *oldNode;
	long g;

	// First we need to calculate the terrain cost of the new tile
	g = node->G + GetTerrainCost(x,y);

	// If our current node is on the open list and the open list one is better,
	// then we can discard and continue
	if((oldNode = Find(_openNodes, x, y)) != NULL) {
		// If our new g value is less than the old nodes g value, then
		// the old node has a new parent
		if(oldNode->G <= g) {
			// We can discard and continue
			return;
		} else {
			// We need to remove this node
			Node *n = Remove(&_openNodes, x, y);
			FreeNode(n);
		}
	}
	
	// Do the same for the closed list
	if((oldNode = Find(_closedNodes, x, y)) != NULL) {		
		// If our new g value is less than the old nodes g value, then
		// the old node has a new parent
		if(oldNode->G <= g) {
			return;
		} else {
			// We need to remove this node
			Node *n = Remove(&_closedNodes, x, y);
			FreeNode(n);
		}
	}

	// Our node was not on any of the lists, so create a new
	// node and add it as a child of the current node
	Node *newNode = AllocateNode();
	newNode->Parent = node;
	newNode->G = g;
	newNode->H = Heuristic(x,y);
	newNode->F = g + newNode->H;
	newNode->X = x;
	newNode->Y = y;
		
	// Add the new node to the open list
	Add(&_openNodes, newNode);
}

long
AStar::GetTerrainCost(int x, int y)
{
	// XXX/GWS: Fix this later. Right now all paths are equal terrain cost
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	return 1;
}

AStar::Node *
AStar::Find(Node *list, int x, int y)
{
	while(list != NULL) {
		if(list->X == x && list->Y == y) {
			return list;
		}
		list = list->Next;
	}
	return NULL;
}

AStar::Node *
AStar::Remove(Node **list, int x, int y)
{
	Node *cur = *list;
	Node *prev = NULL;

	if(NULL == cur) {
		return NULL;
	}
	
	while(cur != NULL) {
		if(cur->X == x && cur->Y == y) {
			if(prev != NULL) {
				prev->Next = cur->Next;
			} else {
				*list = cur->Next;
			}
			return cur;
		}
		prev = cur;
		cur=cur->Next;
	}
	return NULL;
}

void
AStar::Add(Node **list, Node *node)
{
	Node *src = *list;
	Node *prev = NULL;

	if(NULL == src) {
		*list = node;
		return;
	}

	// Adding to a sorted list, with F increasing
	while(src != NULL && src->F < node->F) {
		prev = src;
		src = src->Next;
	}
	if(prev != NULL) {
		prev->Next = node;
		node->Next = src;
	} else {
		node->Next = *list;
		*list = node;
	}
}

