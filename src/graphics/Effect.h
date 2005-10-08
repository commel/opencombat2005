#pragma once

#include <misc\Array.h>
#include <misc\Structs.h>

class Screen;
class TGA;

class Effect 
{
public:
	Effect(char *name);
	virtual ~Effect(void);

	// Clone's this effect
	Effect *Clone();

	// Gets the completed status of this effect
	inline bool IsCompleted() { return _completed; }

	// Adds a frame to this effect
	void AddFrame(TGA *tga, long frameHoldTime);

	// Renders this effect to the screen
	virtual void Render(Screen *screen);

	// The position of this effect in the world
	Point Position;

	// Sets the position of the effect
	inline void SetPosition(int x, int y) { Position.x = x; Position.y = y; }

	// Simulates this effect
	virtual void Simulate(long dt);

	// Gets the name of this effect
	inline char *GetName() { return _name; }

	// Set's the sound for this effect
	void SetSound(char *name);

	// Returns true if this is a dynamic effect (the origin is an offset)
	bool IsDynamic() { return _dynamic; }
	inline void SetDynamic(bool d) { _dynamic = d; }

	// Returns true if this effect is placed on a turret
	inline bool IsPlaceOnTurret() { return _bPlaceOnTurret; }
	inline void SetPlaceOnTurret(bool p) { _bPlaceOnTurret = p; }

protected:
	// The name of this effect
	char _name[MAX_NAME];

	// The sound for this effect
	char _sound[MAX_NAME];

	// The time to display a frame for
	long _frameHoldTime;

	// The total time this effect has been in existence
	long _totalTime;

	// The time between frames
	long _incrementalTime;

	// A flag that tells whether or not this effect is completed
	bool _completed;

	// The list of frames in this effect
	Array<TGA> _frames;

	// The current frame number
	int _currentFrameNumber;

	// Dynamic flag - true means the origin is an offset
	bool _dynamic;

	// Do i place this effect on a turret?
	bool _bPlaceOnTurret;
};
