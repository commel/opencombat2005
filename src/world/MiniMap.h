#pragma once

#include <Misc\Structs.h>
#include <Misc\TGA.h>
class Screen;
class World;

class MiniMap
{
public:
	MiniMap(void);
	virtual ~MiniMap(void);

	// The position of this minimap
	Point Position;
	virtual void SetPosition(int x, int y) { Position.x = x; Position.y = y; }

	// Renders this minimap
	virtual void Render(Screen *screen);

	// Create a new mini map
	static MiniMap *Create(char *fileName, World *parentWorld);

	// Gets the width and height of this mini-map
	inline int GetWidth() { return _tga->GetWidth() + 4; }
	inline int GetHeight() { return _tga->GetHeight() + 4; }

	// Mouse handling routines
	virtual void LeftMouseUp(int x, int y);
	virtual void LeftMouseDown(int x, int y);
	virtual void LeftMouseDrag(int x, int y);

	// Does this mini map contain this point?
	virtual bool Contains(int x, int y);

	// Updates the mini map based on a new world origin
	virtual void Update();

protected:
	// The graphic for this minimap
	TGA *_tga;

	// The parent map for this mini-map
	World *_parentWorld;

	// The extents of the yellow rectangle inside the mini-map,
	// denoting the area we are looking at
	int _zoomWidth, _zoomHeight;

	// The percentages we used to calculate the zoom extents
	float _widthPct, _heightPct;

	// The screen size we used to calculate the zoom extents
	int _calcWidth, _calcHeight;

	// The (x,y) location of the zoom rectangle
	int _x, _y;
};
