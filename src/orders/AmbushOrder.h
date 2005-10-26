#pragma once
#include <orders\Order.h>
#include <misc\Structs.h>

class AmbushOrder :
	public Order
{
public:
	AmbushOrder(Direction dir);
	~AmbushOrder(void);

	Direction Heading;
};
