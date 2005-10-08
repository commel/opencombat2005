#pragma once

#include <misc\Color.h>

class TGA
{
public:
	TGA(void);
	virtual ~TGA(void);

	inline unsigned char *GetData() { return _data; }
	inline int GetDepth() { return _depth; }
	inline int GetWidth() { return _width; }
	inline int GetHeight() { return _height; }
	inline void SetOrigin(int x, int y) { _originX = x; _originY = y; }
	inline int GetOriginX() { return _originX; }
	inline int GetOriginY() { return _originY; }
	inline void SetTransparentColor(unsigned char r, unsigned char g, unsigned char b) { _transparentColor.red=r;_transparentColor.green=g;_transparentColor.blue=b; }
	inline Color *GetTransparentColor() { return &_transparentColor; }

	static TGA *Create(char *fileName);

private:
	unsigned char *_data;
	long _idx;
	int _width;
	int _height;
	int _depth;
	int _originX, _originY;
	Color _transparentColor;
};
