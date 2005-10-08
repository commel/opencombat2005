#pragma once
#include <orders\order.h>
#include <objects\Target.h>

class Object;

class FireOrder :
	public Order
{
public:
	FireOrder(Object *target, Target::Type targetType);
	FireOrder(int x, int y);
	virtual ~FireOrder(void);

	Object *Target;
	Target::Type TargetType;
	int X, Y;
};
