#include ".\fireorder.h"
#include <objects\Object.h>

FireOrder::FireOrder(int x, int y) : Order()
{
	X = x;
	Y = y;
	TargetType = Target::Area;
	Target = 0;
	_orderType = Orders::Fire;
}

FireOrder::FireOrder(Object *target, Target::Type type) : Order()
{
	TargetType = type;
	Target = target;
	_orderType = Orders::Fire;
	X = target->Position.x;
	Y = target->Position.y;
}

FireOrder::~FireOrder(void)
{
}
