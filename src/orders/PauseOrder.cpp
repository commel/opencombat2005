#include ".\pauseorder.h"

PauseOrder::PauseOrder(long pauseTime, int oldState, int pauseState)
{
	_pauseTime = pauseTime;
	_totalTime = 0;
	_orderType = Orders::Pause;
	_oldState = oldState;
	_pauseState = pauseState;
}

PauseOrder::~PauseOrder(void)
{
}
