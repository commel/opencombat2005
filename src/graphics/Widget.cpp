#include ".\widget.h"
#include <string.h>
#include <graphics\Screen.h>
#include <misc\TGA.h>
#include <application\Globals.h>

Widget::Widget(char *name, TGA *tga)
{
	strcpy(_name, name);
	_tga = tga;
}

Widget::~Widget(void)
{
}

void
Widget::Render(Screen *screen, int x, int y, Rect *clip)
{
	UNREFERENCED_PARAMETER(clip);
	screen->Blit(_tga->GetData(), x-_tga->GetOriginX(), y-_tga->GetOriginY(), _tga->GetWidth(), _tga->GetHeight(), _tga->GetWidth(), _tga->GetHeight(), _tga->GetDepth());
}

void
Widget::Render(Screen *screen, int x, int y)
{
	screen->Blit(_tga->GetData(), x-_tga->GetOriginX(), y-_tga->GetOriginY(), _tga->GetWidth(), _tga->GetHeight(), _tga->GetWidth(), _tga->GetHeight(), _tga->GetDepth());
}

void
Widget::Render(Screen *screen, int x, int y, int w, int h)
{
	screen->Blit(_tga->GetData(), x-_tga->GetOriginX(), y-_tga->GetOriginY(), w, h, _tga->GetWidth(), _tga->GetHeight(), _tga->GetDepth());
}

void
Widget::Render(Screen *screen, int x, int y, int w, int h, bool useAlpha)
{
	screen->Blit(_tga->GetData(), x-_tga->GetOriginX(), y-_tga->GetOriginY(), w, h, 0, 0, _tga->GetWidth(), _tga->GetHeight(), _tga->GetDepth(), useAlpha);
}

void 
Widget::Render(Screen *screen, int x, int y, Color *transparentColor)
{
	screen->Blit(_tga->GetData(), x-_tga->GetOriginX(), y-_tga->GetOriginY(), _tga->GetWidth(), _tga->GetHeight(), _tga->GetWidth(), _tga->GetHeight(), _tga->GetDepth(), transparentColor);
}

Widget *
Widget::Clone()
{
	return new Widget(_name, _tga);
}
