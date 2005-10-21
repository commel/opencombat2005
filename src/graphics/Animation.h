#pragma once

#include <misc\Array.h>
#include <misc\Color.h>
#include <misc\Structs.h>
#include <graphics\Frame.h>

class Screen;

/**
 * An animation consists of a number of frames, with a display time for each frame.
 * Each frame is from the same TGA file, and so requires a source (x,y) to pull it out.
 * Also, each frame has certain attributes which define certain colors like transparent,
 * highlighted, etc.
 */
class Animation
{
public:
	Animation(char *name);
	virtual ~Animation(void);

	// Adds a frame to this animation
	void AddFrame(Frame *f, Direction dir);

	// Returns the name of this animation set
	inline char *GetName() { return _name; }

	// Clones this object
	Animation *Clone();

	// Renders this animations to the screen at a given point
	virtual void Render(Screen *screen, Direction heading, int x, int y, bool hilit, Color *hilitColor, int camouflageIdx);

	// Updates the given animations
	virtual void Update(long dt);

	// Gets the exists of the current frame. Returned in relation
	// to the input coords, useful for determining if this object
	// is selected
	virtual void GetExtents(Direction heading, int x, int y, Region *r);

	// Returns the current frame number
	virtual int GetCurrentFrameNumber(Direction heading);

	// Resets the animation
	virtual void Reset();

	// Sets the reverse flag
	void SetReverse(bool b) { _reverse = b; }

protected:
	// The list of frames in this animation. This includes the different directions
	// of each frame as well
	Array<Frame> _frames[NumDirections];

	// The name of this animation set
	char _name[MAX_NAME];

	// The current animation frame number
	int _currentFrameNums[NumDirections];

	// The total time for the current animation
	long _totalTimes[NumDirections];

	// The incremental time for this animation
	long _incrementalTimes[NumDirections];

	// Play this animation in reverse
	bool _reverse;
};
