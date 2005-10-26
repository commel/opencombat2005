#include ".\astar.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <application\Globals.h>

AStar::AStar(void)
{
	_openNodes = new AStar::MinHeap(300*300, this);
	_closedNodes = new Hash(60013,this);
	_destX = -1;
	_destY = -1;

	// Let's allocate a bunch of free nodes that we
	// can use for our path calculations
	_freeNodes = NULL;
	assert(sizeof(Node) == 40);
	for(int i = 0; i < 60000; ++i)
	{
		Node *n = (Node *)calloc(1, sizeof(Node));
		n->MemNext = _freeNodes;
		_freeNodes = n;
	}
	_nAllocatedNodes = 60000;
	_nFreeNodes = 60000;
}

AStar::~AStar(void)
{
	// XXX/GWS: Need to do a deep delete
	delete _openNodes;
	delete _closedNodes;
}

AStar::Node *
AStar::AllocateNode()
{
	Node *n = _freeNodes;
	if(_freeNodes != NULL)
	{
		_freeNodes = _freeNodes->MemNext;
		--_nFreeNodes;
	}
	else
	{
		// We are out of free nodes, so it looks like we need
		// to allocate some more
		n = (Node *)calloc(1, sizeof(Node));
		++_nAllocatedNodes;
	}
	assert(_nFreeNodes >= 0);
	return n;
}

void
AStar::FreeNode(Node *node)
{
	memset(node, 0, sizeof(Node));
	node->MemNext = _freeNodes;
	_freeNodes = node;
	++_nFreeNodes;
}

float
AStar::Heuristic(int x, int y)
{
	// Weight diagonals slightly more
	float diag = (float)(__min(abs(x-_destX), abs(y-_destY)));
	float straight = (float)((abs(x-_destX) + abs(y-_destY)));
	return sqrt(2.0f)*diag + (straight - 2.0f*diag);
}

Path *
AStar::FindPath(int x0, int y0, int x1, int y1)
{
	Node *bestNode = NULL;

	// Let's remember our destination
	_destX = x1;
	_destY = y1;

	_closedNodes->DeepClear();
	_openNodes->Clear();

	// We need to create our initial node and add it to the open
	// list
	Node *node = AllocateNode();
	node->G = 0;
	node->H = Heuristic(x0, y0);
	node->F = node->G + node->H;
	node->X = x0;
	node->Y = y0;
	node->HeapIndex = _openNodes->Insert(node, node->F);

	for(;;)
	{
		if(_openNodes->GetNumNodes() <= 0) {
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
	Path *path = NULL;
	while(bestNode != NULL) {
		Path *p = AllocatePath();
		p->X = bestNode->X;
		p->Y = bestNode->Y;
		p->Next = path;
		path = p;
		bestNode = bestNode->Parent;
		
	}
	return path;
}

AStar::Node *
AStar::GetBestNode()
{
	Node *tmp;
	assert(_openNodes->GetNumNodes() > 0);
	tmp = _openNodes->ExtractMin();
	_closedNodes->Insert(tmp);
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
	float g;

	// First we need to calculate the terrain cost of the new tile
	g = node->G + GetTerrainCost(x,y);

	// If our current node is on the open list and the open list one is better,
	// then we can discard and continue
	if((oldNode = _openNodes->Find(x, y)) != NULL) {
		// If our new g value is less than the old nodes g value, then
		// the old node has a new parent
		if(oldNode->G <= g) {
			// We can discard and continue
			return;
		} else {
			// Let's modify this node in place
			_openNodes->RemoveIndex(oldNode->HeapIndex);
			FreeNode(oldNode);
		}
	}
	
	// Do the same for the closed list
	if((oldNode = _closedNodes->Find(x, y)) != NULL) {		
		// If our new g value is less than the old nodes g value, then
		// the old node has a new parent
		if(oldNode->G <= g) {
			return;
		} else {
			// We need to remove this node because we are going to add it
			// back onto the open list
			Node *n = _closedNodes->Remove(x,y);
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
	_openNodes->Insert(newNode, newNode->F);
}

float
AStar::GetTerrainCost(int x, int y)
{
	// XXX/GWS: Fix this later. Right now all paths are equal terrain cost
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	return 1.0f;
}
