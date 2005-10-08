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

Map::Map(void)
{
	_originX = 0;
	_originY = 0;
}

Map::~Map(void)
{
}

void 
Map::Render(Screen *screen, Rect *clip)
{
	assert((_originX+clip->w) < _mapImage->GetWidth());
	assert((_originY+clip->y) < _mapImage->GetHeight());
	
	if(g_Globals->World.bRenderElevation) {
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
	} else {
		screen->Blit(_mapImage->GetData(), clip->x, clip->y, clip->w, clip->h, _originX, _originY, _mapImage->GetWidth(), _mapImage->GetHeight(), _mapImage->GetDepth()); 
	}
}

void 
Map::RenderElements(Screen *screen, Rect *clip)
{
	assert((_originX+clip->w) < _mapImage->GetWidth());
	assert((_originY+clip->y) < _mapImage->GetHeight());

	// Let's render our elevations in black and white
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

