#pragma once
#include <graphics\frame.h>

class MaskFrame :
	public Frame
{
public:
	MaskFrame(TGA *source, TGA *mask, int displayTime, int width, int height, int sourceX, int sourceY, Color *transparentColor);
	virtual ~MaskFrame(void);

	// Clones this frame
	virtual Frame *Clone();

	// Renders this frame, centered around (x,y)
	virtual void Render(Screen *screen, int x, int y, bool hilit, Color *hilitColor, int camouflageIdx);

	// Gets the extents of this frame, relative to the position
	// (x,y) eg centered around (x,y)
	virtual void GetExtents(int x, int y, Region *r);

protected:

	// The tga of the mask image
	TGA *_mtga;

private:
	// Private constructor
	MaskFrame(MaskFrame *src);
};
