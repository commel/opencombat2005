#include ".\animation.h"
#include <string.h>
#include <graphics\Screen.h>

Animation::Animation(char *name)
{
	strcpy(_name, name);
	for(int i = 0; i < NumDirections; ++i) {
		_currentFrameNums[i] = 0;
		_totalTimes[i] = 0;
		_incrementalTimes[i] = 0;
	}
}

Animation::~Animation(void)
{
}

void
Animation::AddFrame(Frame *f, Direction dir)
{
	_frames[dir].Add(f);
}

Animation *
Animation::Clone()
{
	Animation *a = new Animation(_name);
	for(int dir = 0; dir < NumDirections; ++dir) {
		for(int i = 0; i < _frames[dir].Count; ++i) {
			a->_frames[dir].Add(_frames[dir].Items[i]->Clone());
		}
	}

	for(int i = 0; i < NumDirections; ++i) {
		a->_incrementalTimes[i] = _incrementalTimes[i];
		a->_totalTimes[i] = _totalTimes[i];
	}
	return a;
}

void
Animation::Render(Screen *screen, Direction heading, int x, int y, bool hilit, Color *hilitColor, int camouflageIdx)
{
	_frames[heading].Items[_currentFrameNums[heading]]->Render(screen, x, y, hilit, hilitColor, camouflageIdx);
}


void
Animation::Update(long dt)
{
	for(int heading = 0; heading < NumDirections; ++ heading) 
	{
		_incrementalTimes[heading] += dt;
		_totalTimes[heading] += dt;

		if(_totalTimes[heading] >= _frames[heading].Items[_currentFrameNums[heading]]->GetDisplayTime()) {
			// Update the timers
			_totalTimes[heading] = 0;
			_incrementalTimes[heading] = 0;
			// Increment the current frame number
			_currentFrameNums[heading] = (_currentFrameNums[heading]+1) % _frames[heading].Count;
		}
	}
}

void
Animation::GetExtents(Direction heading, int x, int y, Region *r)
{
	_frames[heading].Items[_currentFrameNums[heading]]->GetExtents(x, y, r);
}

int
Animation::GetCurrentFrameNumber(Direction heading)
{
	return _currentFrameNums[heading];
}

void
Animation::Reset()
{
	for(int i = 0; i < NumDirections; ++i) {
		_totalTimes[i] = 0;
		_incrementalTimes[i] = 0;
		_currentFrameNums[i] = 0;
	}
}
