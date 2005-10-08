#pragma once

#include <misc\TGA.h>
#include <misc\Structs.h>
#include <misc\Color.h>

class Screen;

/**
 * Defines a single frame of an animation. Each frame is from the same TGA file, 
 * and so requires a source (x,y) to pull it out.
 * Also, each frame has certain attributes which define certain colors like transparent,
 * highlighted, etc.
 */
class Frame
{
public:
	Frame() {}
	Frame(TGA *source, int displayTime, int width, int height, int sourceX, int sourceY, Color *transparentColor);
	virtual ~Frame(void);

	// Clones this frame
	virtual Frame *Clone();

	// Renders this frame, centered around (x,y)
	virtual void Render(Screen *screen, int x, int y, bool hilit, Color *hilitColor, int camouflageIdx);

	// Get the display time for this frame
	inline int GetDisplayTime() { return _displayTime; }

	// Gets the extents of this frame, relative to the position
	// (x,y) eg centered around (x,y)
	virtual void GetExtents(int x, int y, Region *r);

protected:
	// The display time for this frame
	int _displayTime;

	// The source image data for this frame. This picture might contain
	// many other animation frames, so we need to describe the location
	// of this frame in the source image
	TGA *_tga;

	// The width and height of this frame
	int _width, _height;

	// The (x,y) location of the start of this frame in the source image
	int _sourceX, _sourceY;

	// The minimum displayable bounds of this frame (the bounds of the frame
	// not including any transparent pixels)
	Bounds _minBounds;

	// The transparent color for this frame
	Color _transparentColor;

	// The shadow color for this frame
	Color _shadowColor;

	// The hiliting color for this frame
	Color _hilitColor;

	// These pixels are both hilited and shadowed
	Color _hilitShadowColor;
};
