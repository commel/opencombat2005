#include ".\map.h"
#include <assert.h>
#include <graphics\Screen.h>
#include <misc\TGA.h>
#include <world\LegacyMapLoader.h>
#include <graphics\Widget.h>
#include <world\Element.h>
#include <world\LineOfSight.h>
#include <application\Globals.h>
#include <world\MapManager.h>
#include <objects\Squad.h>
#include <world\BuildingManager.h>

/**
 * It is very important to not that individual soldiers and vehicles are stored at
 * the tile level. Thus if you select a soldier at the tile level and want to give
 * him an order, you need to get the team that the soldier belongs to and pass the
 * order to it.
 */
Map::Map(void)
{
	_originX = 0;
	_originY = 0;
}

Map::~Map(void)
{
	free(_objects);
	free(_elements);
	free(_elevations);
}

void 
Map::Render(Screen *screen, Rect *clip)
{
	assert((_originX+clip->w) < _mapImage->GetWidth());
	assert((_originY+clip->y) < _mapImage->GetHeight());

	if(g_Globals->World.bRenderBuildingOutlines)
	{
				int startX = _originX / _nPixelsPerBlockX;
		int startY = _originY / _nPixelsPerBlockY;
		int nx = clip->w / _nPixelsPerBlockX;
		int ny = clip->h / _nPixelsPerBlockY;
		Color c(255,255,255);
		for(int j = startY; j < (startY+ny); ++j) {
			for(int i = startX; i < (startX+nx); ++i) {
				if(_buildingIndices[j*_nBlocksX + i] != 0) {
					c.red = 255;
					c.green = 255;
					c.blue = 255;
					screen->FillRect((i-startX)*_nPixelsPerBlockX, (j-startY)*_nPixelsPerBlockY, _nPixelsPerBlockX, _nPixelsPerBlockY, &c);
				} else {
					c.red = 0;
					c.green = 0;
					c.blue = 0;
					screen->FillRect((i-startX)*_nPixelsPerBlockX, (j-startY)*_nPixelsPerBlockY, _nPixelsPerBlockX, _nPixelsPerBlockY, &c);
				}
			}
		}
	}
	else if(g_Globals->World.bRenderElevation) 
	{
		int startX = _originX / _nPixelsPerBlockX;
		int startY = _originY / _nPixelsPerBlockY;
		int nx = clip->w / _nPixelsPerBlockX;
		int ny = clip->h / _nPixelsPerBlockY;
		Color c(255,255,255);
		for(int j = startY; j < (startY+ny); ++j) {
			for(int i = startX; i < (startX+nx); ++i) {
				c.red = (unsigned char)((_elevations[j*_nBlocksX + i] + g_Globals->World.Elements->GetElement(_elements[j*_nBlocksX+i])->GetHeight()) << 2);
				c.green = c.red;
				c.blue = c.red;
				screen->FillRect((i-startX)*_nPixelsPerBlockX, (j-startY)*_nPixelsPerBlockY, _nPixelsPerBlockX, _nPixelsPerBlockY, &c);
			}
		}
	} 
	else 
	{
		screen->Blit(_mapImage->GetData(), clip->x, clip->y, clip->w, clip->h, _originX, _originY, _mapImage->GetWidth(), _mapImage->GetHeight(), _mapImage->GetDepth()); 

		// Render any building interiors that we need.
		int startX = _originX / _nPixelsPerBlockX;
		int startY = _originY / _nPixelsPerBlockY;
		int rx = _originX % _nPixelsPerBlockX;
		int ry = _originY % _nPixelsPerBlockY;
		int nx = clip->w / _nPixelsPerBlockX;
		int ny = clip->h / _nPixelsPerBlockY;

		Array<Building> buildingsToDraw;
		for(int j = startY; j < (startY+ny); ++j) {
			for(int i = startX; i < (startX+nx); ++i) {
				// Is there a building on this tile?
				if(_buildingIndices[j*_nBlocksX+i] > 0)
				{
					// Is there a soldier in this building?
					Building *b = _buildings.Items[_buildingIndices[j*_nBlocksX+i]-1];
					for(int k = 0; k < b->NumTiles; ++k)
					{
						if(_objects[b->Tiles[k]] != NULL)
						{
							// We have to draw this building. Make sure it is
							// not already in our list
							bool bAdd = true;
							for(int m = 0; m < buildingsToDraw.Count; ++m)
							{
								if(buildingsToDraw.Items[m] == b)
								{
									bAdd = false;
									break;
								}
							}
							if(bAdd)
							{
								buildingsToDraw.Add(b);
							}
							break;
						}
					}
				}
			}
		}

		for(int i = 0; i < buildingsToDraw.Count; ++i)
		{
			// Draw this building
			Color white(255,255,255);
			TGA *tga = _buildings.Items[i]->GetInterior();
			screen->Blit(tga->GetData(), 
					buildingsToDraw.Items[i]->Position.x-_originX,
					buildingsToDraw.Items[i]->Position.y-_originY,
					tga->GetWidth(), tga->GetHeight(), tga->GetWidth(), tga->GetHeight(),
					tga->GetDepth(), &white);
		}
	}
}

void 
Map::RenderElements(Screen *screen, Rect *clip)
{
	assert((_originX+clip->w) < _mapImage->GetWidth());
	assert((_originY+clip->y) < _mapImage->GetHeight());

	int startX = _originX / _nPixelsPerBlockX;
	int startY = _originY / _nPixelsPerBlockY;
	int rx = _originX % _nPixelsPerBlockX;
	int ry = _originY % _nPixelsPerBlockY;
	int nx = clip->w / _nPixelsPerBlockX;
	int ny = clip->h / _nPixelsPerBlockY;

	Color c(255,255,255);
	Widget *w;
	for(int j = startY; j < (startY+ny); ++j) {
		for(int i = startX; i < (startX+nx); ++i) {
			if((w = g_Globals->World.Terrain->GetWidget(_elements[j*_nBlocksX + i], false)) != NULL) {
				w->Render(screen, (i-startX)*_nPixelsPerBlockX - rx + (_nPixelsPerBlockX>>1), (j-startY)*_nPixelsPerBlockY - ry + (_nPixelsPerBlockY>>1), &c);
			}
		}
	}
}

Map *
Map::Create(char *fileName)
{
	Map *m = new Map();
	MapAttributes *attr = MapManager::Parse(fileName);

	strcpy(m->_miniName, attr->Mini);
	strcpy(m->_overlandName, attr->Overland);
	m->_mapImage = TGA::Create(attr->Background);
	LegacyMapLoader *l = new LegacyMapLoader();
	l->Load(attr->Elements);
	m->_elements = l->GetElements();
	m->_elevations = l->GetElevations();
	l->GetNumPixelsPerBlock(&(m->_nPixelsPerBlockX), &(m->_nPixelsPerBlockY));
	l->GetNumBlocks(&(m->_nBlocksX), &(m->_nBlocksY));
	m->_objects = (Object **) calloc(m->_nBlocksX*m->_nBlocksY, sizeof(Object *));
	m->_buildingIndices = (unsigned short *) calloc(m->_nBlocksX*m->_nBlocksY, sizeof(short));

	// Load the buildings into this map.
	BuildingManager::LoadBuildings(attr->Buildings, &m->_buildings);
	
	// Populate the building indices array
	m->PopulateBuildingsIndices();
	
	return m;
}

// Get's the elevation for a tile
int
Map::GetTileElevation(int i, int j)
{
	return (_elevations[j*_nBlocksX + i] + g_Globals->World.Elements->GetElement(_elements[j*_nBlocksX+i])->GetHeight());
}

bool 
Map::IsTileBlockHeight(int i, int j)
{
		return (g_Globals->World.Elements->GetElement(_elements[j*_nBlocksX+i])->GetBlocksHeight());
}

void
Map::MoveObject(Object *object, Point *from, Point *to)
{
	// Find out which tile this object used to reside on
	int oi = from->x / _nPixelsPerBlockX;
	int oj = from->y / _nPixelsPerBlockY;

	// Find out which tile the object should now reside on
	int ni = to->x / _nPixelsPerBlockX;
	int nj = to->y / _nPixelsPerBlockY;

	// If they are the same tile, we can stop
	if(oi != ni || oj != nj)
	{
		// Remove from the old pile
		if(object->PrevObject != NULL)
		{
			object->PrevObject->NextObject = object->NextObject;
			if(object->NextObject != NULL)
			{
				object->NextObject->PrevObject = object->PrevObject;
			}
		}
		else
		{
			_objects[oj*_nBlocksX+oi] = object->NextObject;
		}

		// Now add to the new pile
		object->PrevObject = NULL;
		object->NextObject = _objects[nj*_nBlocksX+ni];
		if(object->NextObject != NULL)
		{
			object->NextObject->PrevObject = object;
		}
		_objects[nj*_nBlocksX+ni] = object;
	}
}

void
Map::PlaceObject(Object *object, Point *to)
{
	// Find out which tile the object should now reside on
	int ni = to->x / _nPixelsPerBlockX;
	int nj = to->y / _nPixelsPerBlockY;

	// Now add to the new pile
	object->PrevObject = NULL;
	object->NextObject = _objects[nj*_nBlocksX+ni];
	if(object->NextObject != NULL)
	{
		object->NextObject->PrevObject = object;
	}
	_objects[nj*_nBlocksX+ni] = object;

}

void
Map::SelectObjects(int x, int y, Array<Object> *dest)
{
	// Let's look in a 3x3 sqare centered on (x,y)
	int ci = x / _nPixelsPerBlockX;
	int cj = y / _nPixelsPerBlockY;
	int si = (ci-1) >= 0 ? (ci-1) : 0;
	int sj = (cj-1) >= 0 ? (cj-1) : 0;
	int di = (ci+1) < _nBlocksX ? (ci+1) : _nBlocksX-1;
	int dj = (cj+1) < _nBlocksY ? (cj+1) : _nBlocksY-1;
	for(int j = sj; j <= dj; ++j)
	{
		for(int i = si; i <= di; ++i)
		{
			Object *object = _objects[j*_nBlocksX+i];
			while(object != NULL)
			{
				// XXX/GWS: We are going to do way too many Contains() calls
				//			in the call stack here. If you trace it down
				//			to the soldier level, i think we do two or three
				//			too many. It comes down to the squad needing
				//			to know which individual soldier was selected,
				//			so we need to add a new squad->Select() call to
				//			take that into account.
				if(object->Contains(x,y)) {
					// Get the squad that this guy belongs to
					Squad *s = object->GetSquad();
					
					// Have we already added the squad?
					bool bAdd = true;
					for(int k = 0; k < dest->Count; ++k)
					{
						if(dest->Items[k]->GetID() == s->GetID())
						{
							// Our squad has been added already, so don't do it again
							bAdd = false;
							break;
						}
					}
					
					// If we need to add it, then add it
					if(bAdd)
					{
						s->Select(x,y);
						dest->Add(s);
					}
				}
				object = object->NextObject;
			}
		}
	}
}

void
Map::PopulateBuildingsIndices()
{
	for(int i = 0; i < _buildings.Count; ++i)
	{
		int si = _buildings.Items[i]->Position.x / _nPixelsPerBlockX;
		int sj = _buildings.Items[i]->Position.y / _nPixelsPerBlockY;
		int ni = _buildings.Items[i]->GetInterior()->GetWidth() / _nPixelsPerBlockX;
		int nj = _buildings.Items[i]->GetInterior()->GetHeight() / _nPixelsPerBlockY;

		Building *building = _buildings.Items[i];
		for(int n = sj; n <= (sj+nj); ++n)
		{
			for(int m = si; m <= (si+ni); ++m)
			{
				if(Screen::PointInRegion(m*_nPixelsPerBlockX+_nPixelsPerBlockX/2,
										 n*_nPixelsPerBlockY+_nPixelsPerBlockY/2,
										 &(building->BoundaryPoints)))
				{
					_buildingIndices[n*_nBlocksX+m] = i+1;
					// Now add this tile index into the building
					if(building->NumTiles == 0)
					{
						building->Tiles = (int *)calloc(1, sizeof(int));
						building->Tiles[0] = n*_nBlocksX+m;
						building->NumTiles++;
					}
					else
					{
						building->Tiles = (int *)realloc(building->Tiles, (building->NumTiles+1)*sizeof(int));
						building->Tiles[building->NumTiles] = n*_nBlocksX+m;
						building->NumTiles++;
					}
				}
			}
		}
	}
}