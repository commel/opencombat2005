#pragma once

#include <Misc\Structs.h>

struct IDirect3DDevice9;
class Color;
struct Region;
struct ID3DXLine;

class Screen
{
public:
	Screen(void);
	virtual ~Screen(void);

	// Cleans up resources associated with this screen
	virtual void Cleanup();

	// Set's the device dependent screen access
	virtual void SetDevice(void *device);

	// Set's this device's capabilities
	virtual void SetCapabilities(unsigned char *bits, int width, int height, int format, int pitch);

	// Clears the screen to the given color
	virtual void Clear(Color *c);

	// Retrieves given capabilities
	virtual int GetWidth() { return _width; }
	virtual int GetHeight() { return _height; }

	// Blits a rectangle to the screen
	virtual void Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sw, int sh, int sbytes_per_pixel);
	virtual void Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sw, int sh, int sbytes_per_pixel, Color *transparentColor);
	virtual void Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, int sbytes_per_pixel);
	virtual void Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, Color *transparentColor, Color *shadowColor, Color *hilitColor, Color *hilitShadowColor, bool bHilit, int sbytes_per_pixel);
	virtual void Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, int sbytes_per_pixel, bool useAlpha);
	virtual void Blit(unsigned char *src, unsigned char *mask, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, bool bHilit, Color *hilitColor, int sbytes_per_pixel, int colorModifierIdx);
	virtual void Blit(unsigned char *src, int dx, int dy, int dw, int dh, int sw, int sh, int sbytes_per_pixel, Color *transparentColor, int rotx, int roty, double angle);

	// Draws a line
	virtual void DrawLine(int sx, int sy, int dx, int dy, int width, Color *c);

	// Draw a rectangle
	virtual void DrawRect(int x, int y, int w, int h, int width, Color *c);

	// Fills a rectangle
	virtual void FillRect(int x, int y, int w, int h, Color *c);

	// Retrieves the current position of the cursor
	int GetCursorX() { return _cursorX; }
	int GetCursorY() { return _cursorY; }

	// Sets the position of the cursor
	void SetCursorPosition(int x, int y) { _cursorX = x; _cursorY = y; }

	// Set's the origin for the screen. This is used to offset items that
	// are on the screen
	Point Origin;
	inline void SetOrigin(int x, int y) { Origin.x = x; Origin.y = y; }

	// A utility function to test if a point lies in a polygonal region
	static bool PointInRegion(int x, int y, Region *r);

protected:
	IDirect3DDevice9 *_device;
	ID3DXLine *_line;
	int _width, _height, _format, _bytes_per_pixel, _pitch;
	int _cursorX, _cursorY;
	int _originX, _originY;
	unsigned char *_bits;
};
