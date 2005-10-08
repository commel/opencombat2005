#pragma once

#include <Misc\Array.h>
#include <Misc\Structs.h>
#include <Misc\TGA.h>

class Screen;

/**
 * Map
 *
 * This class defines an abstract map. A map is made up of the following parts.
 *
 */
class Map
{
public:
	Map(void);
	virtual ~Map(void);

	virtual void Render(Screen *screen, Rect *clip);
	virtual void RenderElements(Screen *screen, Rect *clip);
	static Map *Create(char *fileName);

	// Gets the width and height of the map
	inline int GetWidth() { return _mapImage->GetWidth(); }
	inline int GetHeight() { return _mapImage->GetHeight(); }

	// Sets the origin of the map
	inline void SetOrigin(int x, int y) { _originX = x; _originY = y; }

	// Gets the tile sizes
	inline void GetNumTiles(int *nx, int *ny) { *nx = _nBlocksX; *ny = _nBlocksY; }
	inline void GetTileSize(int *w, int *h) { *w = _nPixelsPerBlockX; *h = _nPixelsPerBlockY; }

	// Get's the elevation for a tile
	int GetTileElevation(int i, int j);
	bool IsTileBlockHeight(int i, int j);

	// Gets the mini map name
	char *GetMiniName() { return _miniName; }

	// Gets the overland name
	char *GetOverlandName() { return _overlandName; }

protected:
	TGA *_mapImage;

	// The origin of the map
	int _originX, _originY;

	// The elements in the map
	unsigned short *_elements;

	// The elevations in the map
	unsigned char *_elevations;

	// The number of blocks for the elements and elevation files.
	// For legacy files, each block is 10x10 pixels
	int _nBlocksX, _nBlocksY;

	// The number of pixels per block
	int _nPixelsPerBlockX, _nPixelsPerBlockY;

	char _miniName[MAX_NAME];
	char _overlandName[MAX_NAME];
};
