#include ".\minimap.h"
#include <misc\Color.h>
#include <objects\Object.h>
#include <world\World.h>
#include <graphics\Screen.h>
#include <application\Globals.h>

MiniMap::MiniMap(void)
{
	_zoomWidth = -1;
	_zoomHeight = -1;
	_x = 0;
	_y = 0;
}

MiniMap::~MiniMap(void)
{
}

void
MiniMap::Render(Screen *screen)
{
	Color white(255,255,255);
	Color black(0,0,0);

	screen->DrawRect(Position.x, Position.y, _tga->GetWidth()+4, _tga->GetHeight()+4, 1, &black);
	screen->DrawRect(Position.x+1, Position.y+1, _tga->GetWidth()+2, _tga->GetHeight()+2, 1, &white);
	screen->Blit(_tga->GetData(), Position.x+2, Position.y+2, _tga->GetWidth(), _tga->GetHeight(), _tga->GetWidth(), _tga->GetHeight(), _tga->GetDepth());

	// We need to mark off the position of all the moving objects
	// in the world on the mini map
	Color blue(0,0,255);
	Array<Object> *objs = _parentWorld->GetObjects();
	for(int i = 0; i < objs->Count; ++i) {
		// Draw a small circle
		float xpct = ((float) objs->Items[i]->Position.x / (float) _parentWorld->GetWidth());
		float ypct = ((float) objs->Items[i]->Position.y / (float) _parentWorld->GetHeight());
		screen->FillRect(Position.x + 2 + (int)(xpct*(float)_tga->GetWidth()), Position.y + 2 + (int)(ypct*(float)_tga->GetHeight()), 5, 5, &blue);
	}

	// Now we need to draw the yellow line. If we have not determined our extents yet,
	// then find them
	if(_zoomWidth < 0 || _calcWidth != screen->GetWidth()) {
		// We need to figure out how big our box should be
		_widthPct = (float) screen->GetWidth() / (float) _parentWorld->GetWidth();
		_heightPct = (float) screen->GetHeight() / (float) _parentWorld->GetHeight();
		_calcWidth = screen->GetWidth();
		_calcHeight = screen->GetHeight();

		// Now figure out our zoom width
		_zoomWidth = (int)(_widthPct * (float) _tga->GetWidth());
		_zoomHeight= (int)(_heightPct * (float) _tga->GetHeight());
	} else {
		Color yellow(255,255,0);
		screen->DrawRect(Position.x+2+_x, Position.y+2+_y, _zoomWidth, _zoomHeight, 1, &yellow);
	}
}

MiniMap *
MiniMap::Create(char *fileName, World *parentWorld)
{
	MiniMap *mm = new MiniMap();
	mm->_tga = TGA::Create(fileName);
	mm->_parentWorld = parentWorld;
	return mm;
}

bool
MiniMap::Contains(int x, int y) 
{
	Region r;
	r.points[0].x = Position.x+2; r.points[0].y = Position.y+2;
	r.points[1].x = Position.x+2+_tga->GetWidth(); r.points[1].y = Position.y+2;
	r.points[2].x = Position.x+2+_tga->GetWidth(); r.points[2].y = Position.y+2+_tga->GetHeight();
	r.points[3].x = Position.x+2; r.points[3].y = Position.y+2+_tga->GetHeight();
	return Screen::PointInRegion(x, y, &r);
}

void
MiniMap::LeftMouseDown(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void 
MiniMap::LeftMouseUp(int x, int y)
{
	_x = x - (Position.x+2) - _zoomWidth/2;
	_y = y - (Position.y+2) - _zoomHeight/2;

	if(_x < 0) { _x = 0; }
	if(_y < 0) { _y = 0; }
	if(_x > (_tga->GetWidth()-_zoomWidth)) { _x = _tga->GetWidth() - _zoomWidth; }
	if(_y > (_tga->GetHeight()-_zoomHeight)) { _y = _tga->GetHeight() - _zoomHeight; }
}

void
MiniMap::LeftMouseDrag(int x, int y)
{
	_x = x - (Position.x+2) - _zoomWidth/2;
	_y = y - (Position.y+2) - _zoomHeight/2;

	if(_x < 0) { _x = 0; }
	if(_y < 0) { _y = 0; }
	if(_x > (_tga->GetWidth()-_zoomWidth)) { _x = _tga->GetWidth() - _zoomWidth; }
	if(_y > (_tga->GetHeight()-_zoomHeight)) { _y = _tga->GetHeight() - _zoomHeight; }

	// Now we need to update where the world is!
	int ox = (int)((float)_parentWorld->GetWidth() * ((float)_x/(float)_tga->GetWidth()));
	int oy = (int)((float)_parentWorld->GetHeight() * ((float)_y/(float)_tga->GetHeight()));
	if((ox+_calcWidth) >= _parentWorld->GetWidth()) {
		ox = _parentWorld->GetWidth() - _calcWidth - 1;
	}
	if((oy+_calcHeight) >= _parentWorld->GetHeight()) {
		oy = _parentWorld->GetHeight() - _calcHeight - 1;
	}

	_parentWorld->SetOrigin(ox, oy);
}
