#pragma once

namespace Orders {
	enum OrderType {
		Move,
		MoveFast,
		Sneak,
		Fire,
		Ambush,
		Hide,
		Smoke,
		Destination,
		Stop,
		Pause
	};
}

class Order
{
public:
	Order(void);
	virtual ~Order(void);

	inline Orders::OrderType GetType() { return _orderType; }
	
	// Following two functions are for reference counting
	inline void Release() { --_refCount; if(_refCount <= 0) { delete this; } }
	inline void IncrementRefCount() { ++_refCount; }

protected:
	Orders::OrderType _orderType;

private:
	int _refCount;
};
