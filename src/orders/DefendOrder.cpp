#include ".\defendorder.h"

DefendOrder::DefendOrder(Direction dir)
{
	_orderType = Orders::Defend;
	Heading = dir;
}

DefendOrder::~DefendOrder(void)
{
}
