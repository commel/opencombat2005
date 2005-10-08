#pragma once

class CD3DFont;
class Color;
class Screen;

class FontManager
{
public:
	FontManager(void);
	virtual ~FontManager(void);

	// First time initialization
	void Initialize(void *data);
	// Final cleanup
	void Cleanup();
	// Reset
	void Restore();
	// Invalidate 
	void Invalidate();

	void Render(Screen *screen, char *msg, int x, int y, Color *c);

protected:
    CD3DFont* _font; // Font for drawing text
};
