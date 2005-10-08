#pragma once

#include "Order.h"

class MoveOrder : public Order
{
public:
	MoveOrder(int x, int y, Orders::OrderType type);
	virtual ~MoveOrder(void);

	int X; int Y;
};
