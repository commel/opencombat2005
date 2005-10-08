#include ".\moveorder.h"

MoveOrder::MoveOrder(int x, int y, Orders::OrderType type) : Order()
{
	_orderType = type;
	X = x;
	Y = y;
}

MoveOrder::~MoveOrder(void)
{
}
