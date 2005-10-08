#pragma once
#include "order.h"

class PauseOrder :
	public Order
{
public:
	PauseOrder(long pauseTime, int oldState, int pauseState);
	virtual ~PauseOrder(void);

	inline long GetPauseTime() { return _pauseTime; }
	inline long GetTotalTime() { return _totalTime; }
	inline void IncrementTotalTime(long dt) { _totalTime += dt; }
	inline int GetOldState() { return _oldState; }
	inline int GetPauseState() { return _pauseState; }

protected:
	long _pauseTime;
	long _totalTime;
	int _oldState;
	int _pauseState;
};
