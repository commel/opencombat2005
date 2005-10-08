#include ".\fontmanager.h"
#include <directx\D3DFont.h>
#include <misc\Color.h>
#include <graphics\Screen.h>

FontManager::FontManager(void)
{
	_font = NULL;
}

FontManager::~FontManager(void)
{
	if(_font != NULL) {
		delete _font;
	}
}

void 
FontManager::Initialize(void *data)
{
	_font = new CD3DFont("Arial", 7, 0);
	LPDIRECT3DDEVICE9 device = (LPDIRECT3DDEVICE9)data;
	_font->InitDeviceObjects(device);
}

void 
FontManager::Cleanup()
{
	_font->DeleteDeviceObjects();
}

void
FontManager::Restore()
{
	_font->RestoreDeviceObjects();
}

void
FontManager::Invalidate()
{
	_font->InvalidateDeviceObjects();
}

void 
FontManager::Render(Screen *screen, char *msg, int x, int y, Color *c)
{
	UNREFERENCED_PARAMETER(screen);
	_font->DrawText( (float)x, (float)y, D3DCOLOR_ARGB(255,c->red,c->green,c->blue), msg );
}

