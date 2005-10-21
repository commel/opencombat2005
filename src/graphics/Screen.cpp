#include ".\screen.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <assert.h>
#include <misc\Color.h>
#include <misc\Structs.h>
#include <graphics\SoldierMasks.h>

Screen::Screen(void)
{
	_device = NULL;
	_line = NULL;
	Origin.x = 0;
	Origin.y = 0;
}

Screen::~Screen(void)
{
	Cleanup();
}

void
Screen::Cleanup()
{
	if(_line != NULL) {
		_line->Release();
		_line = NULL;
	}
}

void
Screen::SetDevice(void *device)
{
	if(device != _device) {
		_device = (IDirect3DDevice9 *)device;
		if(_line != NULL) {
			_line->Release();
			_line = NULL;
		}
		D3DXCreateLine(_device, &_line);
		_line->SetAntialias(true);
	}
}

void
Screen::SetCapabilities(unsigned char *bits, int width, int height, int format, int pitch)
{
	_bits = bits;
	_width = width;
	_height = height;
	_format = format;
	_pitch = pitch;

	switch(_format) {
		case D3DFMT_A2R10G10B10:
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			_bytes_per_pixel = 4;
			break;
		case D3DFMT_A1R5G5B5:
		case D3DFMT_X1R5G5B5:
		case D3DFMT_R5G6B5:
			_bytes_per_pixel = 2;
			break;
		default:
			assert(false);
	}
}

void
Screen::Clear(Color *c)
{
	// This is going to be really slow, but whatever
	for(int j = 0; j < _height; ++j) {
		for(int i = 0; i < _width; ++i) {
			_bits[j*_pitch + i*_bytes_per_pixel + 0] = 0;
			_bits[j*_pitch + i*_bytes_per_pixel + 1] = c->red;
			_bits[j*_pitch + i*_bytes_per_pixel + 2] = c->green;
			_bits[j*_pitch + i*_bytes_per_pixel + 3] = c->blue;
		}
	}
}

// XXX/GWS: All of the blitting routines in here need to be optimized!!!
void
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sw, int sh, int sbytes_per_pixel)
{
	UNREFERENCED_PARAMETER(sh);
	assert(_bytes_per_pixel == sbytes_per_pixel);
	for(int j = 0; j < dh; ++j) {
		memcpy(&(_bits[(j+dy)*_pitch+dx*_bytes_per_pixel]), &(src[j*sw*sbytes_per_pixel]), dw*_bytes_per_pixel);
	}
}

void 
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sw, int sh, int sbytes_per_pixel, Color *transparentColor)
{
	UNREFERENCED_PARAMETER(sh);
	unsigned char r,g,b;
	assert(_bytes_per_pixel == sbytes_per_pixel);
	for(int j = 0; j < dh; ++j) {
		for(int i = 0; i < dw; ++i) {
			r = src[(j)*sw*sbytes_per_pixel + (i)*sbytes_per_pixel + 2];
			g =	src[(j)*sw*sbytes_per_pixel + (i)*sbytes_per_pixel + 1];
			b = src[(j)*sw*sbytes_per_pixel + (i)*sbytes_per_pixel + 0];
			
			// First look at transparency
			if(r != transparentColor->red || g != transparentColor->green || b != transparentColor->blue) {
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = b;
			}
		}
	}
}

// Blits with rotation
// The basic rotation matrix is given by:
//
// [x', y'] = [ x*cos(theta) + y*sin(theta) , y*cos(theta) - x*sin(theta) ]
void 
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sw, int sh, int sbytes_per_pixel, Color *transparentColor, int rotx, int roty, double angle)
{
	int xp, yp;
	double cosT = cos(angle);
	double sinT = sin(angle);
	unsigned char r,g,b;
	assert(_bytes_per_pixel == sbytes_per_pixel);
	UNREFERENCED_PARAMETER(sh);

	for(int j = 0; j < dh; ++j) {
		for(int i = 0; i < dw; ++i) {
			xp = (int)((i-rotx)*cosT + (j-roty)*sinT);
			yp = (int)((j-roty)*cosT - (i-rotx)*sinT);
			r = src[(j)*sw*sbytes_per_pixel + (i)*sbytes_per_pixel + 2];
			g =	src[(j)*sw*sbytes_per_pixel + (i)*sbytes_per_pixel + 1];
			b = src[(j)*sw*sbytes_per_pixel + (i)*sbytes_per_pixel + 0];

			if(r != transparentColor->red || g != transparentColor->green || b != transparentColor->blue) {
				_bits[(yp+dy+roty)*_pitch+(xp+dx+rotx)*_bytes_per_pixel + 2] = r;
				_bits[(yp+dy+roty)*_pitch+(xp+dx+rotx)*_bytes_per_pixel + 1] = g;
				_bits[(yp+dy+roty)*_pitch+(xp+dx+rotx)*_bytes_per_pixel + 0] = b;
			}
		}
	}
}

void
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, int sbytes_per_pixel)
{
	UNREFERENCED_PARAMETER(sh);
	assert(_bytes_per_pixel == sbytes_per_pixel);
	for(int j = 0; j < dh; ++j) {
		memcpy(&(_bits[(j+dy)*_pitch+dx*_bytes_per_pixel]), &(src[(j+sy)*sw*sbytes_per_pixel + sx*sbytes_per_pixel]), dw*_bytes_per_pixel);
	}
}

void
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, int sbytes_per_pixel, bool useAlpha)
{
	UNREFERENCED_PARAMETER(sh);
	if(!useAlpha) {
		Blit(src, dx,dy,dw,dh,sx,sy,sw,sh,sbytes_per_pixel);
	} else {
		unsigned char r,g,b,a,or,og,ob;

		for(int j = 0; j < dh; ++j) {
			for(int i = 0; i < dw; ++i) {
				// new pixel = (alpha)(pixel A color) + (1 - alpha)(pixel B color)
				a = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 3];
				r = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 2];
				g =	src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 1];
				b = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 0];
				 
				or = _bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2];
				og = _bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1];
				ob = _bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0];
	
				// XXX/GWS: Need to speed this up!!!
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (a*(or-r) >> 8) + r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (a*(og-g) >> 8) + g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (a*(ob-b) >> 8) + b;
			}
		}
	}
}

// XXX/GWS: We can speed this one up a lot
void 
Screen::Blit(unsigned char *src, int dx, int dy, int dw, int dh, 
			 int sx, int sy, int sw, int sh, 
			 Color *transparentColor, Color *shadowColor, Color *hilitColor, Color *hilitShadowColor,
			 bool bHilit, int sbytes_per_pixel)
{
	UNREFERENCED_PARAMETER(sh);
	unsigned char r,g,b;
	for(int j = 0; j < dh; ++j) {
		for(int i = 0; i < dw; ++i) {
			r = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 2];
			g =	src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 1];
			b = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 0];
			
			// First look at transparency
			if(r != transparentColor->red || g != transparentColor->green || b != transparentColor->blue) {
				// Next look at shadow
				if(r == shadowColor->red && g == shadowColor->green && b == shadowColor->blue) {
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] >>= 2) *= 3;
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] >>= 2) *= 3;
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] >>= 2) *= 3;					
				} else {
					// Look at hiliting
					if(bHilit) {
						// Draw normal, with hiliting
						if(r == hilitShadowColor->red && g == hilitShadowColor->green && b == hilitShadowColor->blue) {
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = hilitColor->red;
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = hilitColor->green;
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = hilitColor->blue;
						} else {
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = r;
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = g;
							_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = b;
						}
					} else {
						// Remove hiliting color
						if(r != hilitColor->red || g != hilitColor->green || b != hilitColor->blue) {
							if(r == hilitShadowColor->red && g == hilitShadowColor->green && b == hilitShadowColor->blue) {
								(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] >>= 2) *= 3;
								(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] >>= 2) *= 3;
								(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] >>= 2) *= 3;
							} else {
								_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = r;
								_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = g;
								_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = b;
							}
						}
					}
				}
			}
		}
	}
}

void 
Screen::Blit(unsigned char *src, unsigned char *mask, 
			 int dx, int dy, int dw, int dh, 
			 int sx, int sy, int sw, int sh, 
			 bool bHilit, Color *hilitColor, int sbytes_per_pixel, int modifierIdx)
{
	unsigned int *isrc = (unsigned int *)src;
	unsigned int *imask = (unsigned int *) mask;
	unsigned int pixel;
	unsigned int msk;
	int r,g,b;
	UNREFERENCED_PARAMETER(sh);

	assert(sbytes_per_pixel == 4);
	assert(_bytes_per_pixel == 4);

	for(int j = 0; j < dh; ++j) {
		for(int i = 0; i < dw; ++i) {
			// Look at all the masks
			pixel = isrc[(sy+j)*sw + sx+i];
			msk = (imask[(sy+j)*sw + sx+i]) & 0xFFFFFF;

			r = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 2];
			g =	src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 1];
			b = src[(sy+j)*sw*sbytes_per_pixel + (sx+i)*sbytes_per_pixel + 0];
			
			if(MASK_SHADOW == msk) {
				(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] >>= 2) *= 3;
				(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] >>= 2) *= 3;
				(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] >>= 2) *= 3;
			} else if(MASK_SHADOW_EDGE == msk) {
				if(bHilit) {
					_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = hilitColor->red;
					_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = hilitColor->green;
					_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = hilitColor->blue;
				} else {
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] >>= 2) *= 3;
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] >>= 2) *= 3;
					(_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] >>= 2) *= 3;
				}
			} else if(MASK_EDGE == msk && bHilit) {
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = hilitColor->red;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = hilitColor->green;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = hilitColor->blue;
			} else if(MASK_BODY == msk) {
				r += g_ColorModifiers[modifierIdx].Body.Red;
				g += g_ColorModifiers[modifierIdx].Body.Green;
				b += g_ColorModifiers[modifierIdx].Body.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_LEGS == msk) {
				r += g_ColorModifiers[modifierIdx].Legs.Red;
				g += g_ColorModifiers[modifierIdx].Legs.Green;
				b += g_ColorModifiers[modifierIdx].Legs.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_HEAD == msk) {
				r += g_ColorModifiers[modifierIdx].Head.Red;
				g += g_ColorModifiers[modifierIdx].Head.Green;
				b += g_ColorModifiers[modifierIdx].Head.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_BELT == msk) {
				r += g_ColorModifiers[modifierIdx].Belt.Red;
				g += g_ColorModifiers[modifierIdx].Belt.Green;
				b += g_ColorModifiers[modifierIdx].Belt.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_BOOTS == msk) {
				r += g_ColorModifiers[modifierIdx].Boots.Red;
				g += g_ColorModifiers[modifierIdx].Boots.Green;
				b += g_ColorModifiers[modifierIdx].Boots.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_WEAPON == msk) {
				r += g_ColorModifiers[modifierIdx].Weapon.Red;
				g += g_ColorModifiers[modifierIdx].Weapon.Green;
				b += g_ColorModifiers[modifierIdx].Weapon.Blue;
				r = (r < 0) ? 0 : r;
				r = (r > 0xFF) ? 0xFF : r;
				g = (g < 0) ? 0 : g;
				g = (g > 0xFF) ? 0xFF : g;
				b = (b < 0) ? 0 : b;
				b = (b > 0xFF) ? 0xFF : b;
				
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			} else if(MASK_TRANSPARENT != msk && MASK_EDGE != msk) {
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 2] = (unsigned char)r;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 1] = (unsigned char)g;
				_bits[(dy+j)*_pitch+(dx+i)*_bytes_per_pixel + 0] = (unsigned char)b;
			}
		}
	}
}

bool
Screen::PointInRegion(int x, int y, Region *r)
{
	int i, j=0;
	bool oddNodes=false;
	int polySides = 4;

	for(i=0; i<polySides; i++) {
		j++; if (j==polySides) j=0;
		if (r->points[i].y < y && r->points[j].y >= y 
			|| r->points[j].y < y && r->points[i].y >= y) 
		{
			if (r->points[i].x + (y-r->points[i].y)/(r->points[j].y-r->points[i].y)*(r->points[j].x-r->points[i].x) < x) 
			{
				oddNodes=!oddNodes;
			}
		}
	}
	return oddNodes;
}

void 
Screen::DrawLine(int sx, int sy, int dx, int dy, int width, Color *c)
{
	if(_line == NULL) {
		D3DXCreateLine(_device, &_line);
		_line->SetAntialias(true);
	}

	D3DXVECTOR2 vertices[2];
	vertices[0].x = (float)sx; vertices[0].y = (float)sy;
	vertices[1].x = (float)dx; vertices[1].y = (float)dy;
	_line->SetWidth((float)width);
	_line->Draw(vertices, 2, D3DCOLOR_ARGB(c->alpha, c->red, c->green, c->blue));
}

void
Screen::DrawRect(int x, int y, int w, int h, int width, Color *c)
{
	DrawLine(x, y, x+w, y, width, c);
	DrawLine(x+w, y, x+w, y+h, width, c);
	DrawLine(x+w, y+h, x, y+h, width, c);
	DrawLine(x, y+h, x, y, width, c);
}

void
Screen::FillRect(int x, int y, int w, int h, Color *c)
{
	for(int j = 0; j < h; ++j) {
		for(int i = 0; i < w; ++i) {
			_bits[(j+y)*_pitch+(i+x)*_bytes_per_pixel + 0] = c->blue;
			_bits[(j+y)*_pitch+(i+x)*_bytes_per_pixel + 1] = c->green;
			_bits[(j+y)*_pitch+(i+x)*_bytes_per_pixel + 2] = c->red;
		}
	}
}

bool
Screen::PointInRegion(int x, int y, int rx, int ry, int rw, int rh)
{
	Region r;
	r.points[0].x = rx;		r.points[0].y = ry;
	r.points[1].x = rx+rw;	r.points[1].y = ry;
	r.points[2].x = rx+rw;	r.points[2].y = ry+rh;
	r.points[3].x = rx;		r.points[3].y = ry+rh;
	return PointInRegion(x,y,&r);
}

bool
Screen::PointInRegion(int x, int y, Array<Point> *points)
{
	bool c = false;
	int i, j;
    for (i = 0, j = points->Count-1; i < points->Count; j = i++) {
		if ((((points->Items[i]->y <= y) && (y < points->Items[j]->y)) ||
             ((points->Items[j]->y <= y) && (y < points->Items[i]->y))) &&
            (x < (points->Items[j]->x - points->Items[i]->x) * (y - points->Items[i]->y) / (points->Items[j]->y - points->Items[i]->y) + points->Items[i]->x))
		{
			c = !c;
		}
	}
    return c;
}

bool
Screen::SelfTest()
{
	// Let's test the PointInRegion functionality.
	// First, let's make a rhomboid type thing
	Array<Point> points;
	points.Add(new Point(0,0));
	points.Add(new Point(100,0));
	points.Add(new Point(200, 100));
	points.Add(new Point(100, 100));
	assert(!PointInRegion(0,50,&points));
	assert(PointInRegion(100,50,&points));
	return true;
}
