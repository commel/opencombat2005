#include ".\object.h"
#include <string.h> // for memset()
#include <orders\Order.h>
#include <graphics\Screen.h>
#include <application\Globals.h>

static long g_id = 0;

Object::Object()
{
	NextObject = NULL;
	PrevObject = NULL;
	Position.x = Position.y = 0;
	_isSelected = false;
	_bHighlight = false;
	memset(&_minBounds, 0, sizeof(Bounds));
	_id = g_id++;
	_type = Target::NoTarget;
	_health = HEALTH_MAX;
	_bSquadLeader = false;
	_moving = false;
}

Object::~Object(void)
{
}

bool
Object::Select(int x, int y)
{
	return Contains(x,y);
}

bool
Object::Contains(int x, int y)
{
	Region r;
	r.points[0].x = _minBounds.x0; r.points[0].y = _minBounds.y0;
	r.points[1].x = _minBounds.x1; r.points[1].y = _minBounds.y0;
	r.points[2].x = _minBounds.x1; r.points[2].y = _minBounds.y1;
	r.points[3].x = _minBounds.x0; r.points[3].y = _minBounds.y1;
	return Screen::PointInRegion(x, y, &r);
}

void
Object::AddOrder(Order *o)
{
	o->IncrementRefCount();
	_orders.Enqueue(o);
}

void
Object::InsertOrder(Order *o, int i) 
{
	o->IncrementRefCount();
	_orders.Insert(o, i);
}

void
Object::ClearOrders()
{
	Order *o;

	while((o = _orders.Peek()) != NULL) {
		_orders.Dequeue();
		o->Release();
	}
}

void
Object::SetPosition(int x, int y)
{
	Position.x = x;
	Position.y = y;
}

bool
Object::IsStopped()
{
	return !_moving;
}
