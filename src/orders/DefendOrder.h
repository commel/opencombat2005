#pragma once
#include <orders\Order.h>
#include <misc\Structs.h>

class DefendOrder :
	public Order
{
public:
	DefendOrder(Direction dir);
	~DefendOrder(void);

	Direction Heading;
};
