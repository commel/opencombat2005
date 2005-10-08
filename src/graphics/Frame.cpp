#include ".\frame.h"
#include <graphics\Screen.h>
#include <application\Globals.h>

Frame::Frame(TGA *source, int displayTime, int width, int height, int sourceX, int sourceY, Color *transparentColor)
{
	_tga = source;
	_displayTime = displayTime;
	_width = width;
	_height = height;
	_sourceX = sourceX;
	_sourceY = sourceY;
	_transparentColor = *transparentColor;

	// XXX/GWS: Fix this, remove hardcoding
	_shadowColor.red = 0;
	_shadowColor.green = 127;
	_shadowColor.blue = 0;
	_hilitColor.red = 0;
	_hilitColor.green = 200;
	_hilitColor.blue = 0;
	_hilitShadowColor.red = 31;
	_hilitShadowColor.green = 255;
	_hilitShadowColor.blue = 31;

	// Now calculate the minimum bounds for this frame
	int xorigin=_sourceX+_width, yorigin=_sourceY+_height;
	int xend=0, yend=0;
	unsigned char *bits = _tga->GetData();
	for(int j = _sourceY; j < _sourceY+_height; ++j) {
		for(int i = _sourceX; i < _sourceX+_width; ++i) {
			if((bits[j*_tga->GetWidth()*_tga->GetDepth() + i*_tga->GetDepth() + 2] != _transparentColor.red 
				|| bits[j*_tga->GetWidth()*_tga->GetDepth() + i*_tga->GetDepth() + 1] != _transparentColor.green
				|| bits[j*_tga->GetWidth()*_tga->GetDepth() + i*_tga->GetDepth() + 0] != _transparentColor.blue)
				&&
				(bits[j*_tga->GetWidth()*_tga->GetDepth() + i*_tga->GetDepth() + 2] != _shadowColor.red 
				|| bits[j*_tga->GetWidth()*_tga->GetDepth() + i*_tga->GetDepth() + 1] != _shadowColor.green
				|| bits[j*_tga->GetWidth()*_tga->GetDepth() + i*_tga->GetDepth() + 0] != _shadowColor.blue)
				)
			{
				if(i < xorigin) {
					xorigin = i-1;
				}
				if(i > xend) {
					xend = i+1;
				}
				if(j < yorigin) {
					yorigin = j-1;
				}
				if(j > yend) {
					yend = j+1;
				}
			}
		}
	}
	_minBounds.x0 = xorigin;
	_minBounds.y0 = yorigin;
	_minBounds.x1 = xend;
	_minBounds.y1 = yend;
}

Frame::~Frame(void)
{
}

Frame *
Frame::Clone()
{
	Frame *f = new Frame(_tga, _displayTime, _width, _height, _sourceX, _sourceY, &_transparentColor);
	return f;
}

void
Frame::Render(Screen *screen, int x, int y, bool hilit, Color *hilitColor, int camouflageIdx)
{
	UNREFERENCED_PARAMETER(camouflageIdx);
	UNREFERENCED_PARAMETER(hilitColor);
	screen->Blit(_tga->GetData(), x-((_sourceX+(_width>>1))-_minBounds.x0), 
		y-((_sourceY+(_height>>1))-_minBounds.y0), 
		_minBounds.x1-_minBounds.x0, _minBounds.y1-_minBounds.y0, 
		_minBounds.x0, _minBounds.y0, _tga->GetWidth(), _tga->GetHeight(), 
		&_transparentColor, &_shadowColor, &_hilitColor, &_hilitShadowColor, 
		hilit, _tga->GetDepth());

	// Draw a bounding box around this frame
#if 0
	Color c(255,0,0);
	Region r;
	GetExtents(x, y, &r);
	screen->DrawLine(r.points[0].x, r.points[0].y, r.points[1].x, r.points[1].y, 1, &c);
	screen->DrawLine(r.points[1].x, r.points[1].y, r.points[2].x, r.points[2].y, 1, &c);
	screen->DrawLine(r.points[2].x, r.points[2].y, r.points[3].x, r.points[3].y, 1, &c);
	screen->DrawLine(r.points[3].x, r.points[3].y, r.points[0].x, r.points[0].y, 1, &c);
#endif
}

void
Frame::GetExtents(int x, int y, Region *r)
{
	r->points[0].x = x-((_sourceX+(_width>>1))-_minBounds.x0);
	r->points[0].y = y-((_sourceY+(_height>>1))-_minBounds.y0);
	r->points[1].x = r->points[0].x + _minBounds.x1 - _minBounds.x0;
	r->points[1].y = r->points[0].y;
	r->points[2].x = r->points[1].x;
	r->points[2].y = r->points[1].y + _minBounds.y1-_minBounds.y0;
	r->points[3].x = r->points[0].x;
	r->points[3].y = r->points[2].y;
}
