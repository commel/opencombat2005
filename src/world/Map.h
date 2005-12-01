#pragma once

#include <misc\Array.h>
#include <misc\StrictArray.h>
#include <misc\Structs.h>
#include <misc\TGA.h>
#include <World\Building.h>
#include <world\Element.h>
#include <world\VictoryLocation.h>
#include <world\Nationality.h>

class Object;
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
	void ConvertMegaTileToPosition(int mi, int mj, int *x, int *y) { *x = mi*_nPixelsPerBlockX*12+((_nPixelsPerBlockX*12)>>1), *y=mj*_nPixelsPerBlockY*12+((_nPixelsPerBlockY*12)>>1);}
	
	// Get's the elevation for a tile
	int GetTileElevation(int i, int j);
	bool IsTileBlockHeight(int i, int j);

	// Retrieves the tile element
	Element *GetTileElement(int i, int j);

	// Gets the mini map name
	char *GetMiniName() { return _miniName; }

	// Gets the overland name
	char *GetOverlandName() { return _overlandName; }

	// Moves an object from one tile to another
	void MoveObject(Object *object, Point *from, Point *to);

	// Places an object on a tile
	void PlaceObject(Object *object, Point *to);

	// Tries to select objects at (x,y) and place them in the array argument
	void SelectObjects(int x, int y, Array<Object> *dest);

	// Gets the number of victory locations
	int GetNumVictoryLocations() { return _victoryLocations.Count; }
	void GetVictoryLocation(int idx, int *x, int *y, Nationality **nationality);

protected:
	// Populates the buildings index array
	void PopulateBuildingsIndices();

	// The map image
	TGA *_mapImage;

	// The origin of the map
	int _originX, _originY;

	// XXX/GWS: The following are all things that can exist on a tile,
	//			which includes things like elements, elevations, units,
	//			etc. We could use a struct to encapsulate all of this things,
	//			but because this is C++, that would use a little bit more
	//			extra memory than we require. And since we want maps to
	//			be insanely huge, better safe than sorry here.

	// The elements in the map, a map of {_bBlocksX, _nBlocksY}
	unsigned short *_elements;

	// The elevations in the map, a map of {_bBlocksX, _nBlocksY}
	unsigned char *_elevations;

	// The objects that are on each tile. Again, a map of {_blocksX, _blocksY}
	Object **_objects;

	// This array keeps track of which building indices are on each
	// tile. It is an array of {_nBlocksX, _nBlocksY} and is 1-origin,
	// so that a value of zero means no building
	unsigned short *_buildingIndices;

	// An array of the buildings that are on this map
	Array<Building> _buildings;

	// The number of blocks for the elements and elevation files.
	// For legacy files, each block is 10x10 pixels
	int _nBlocksX, _nBlocksY;

	// The number of pixels per block
	int _nPixelsPerBlockX, _nPixelsPerBlockY;

	// Victory locations on this map
	StrictArray<VictoryLocation *> _victoryLocations;

	char _miniName[MAX_NAME];
	char _overlandName[MAX_NAME];
};
