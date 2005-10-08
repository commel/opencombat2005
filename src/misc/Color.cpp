#include ".\color.h"

Color::Color(void)
{
	alpha = 0xff;
	red = green = blue = 0;
}

Color::Color(int r, int g, int b)
{
	alpha = 0xFF;
	red = (unsigned char)r;
	green = (unsigned char)g;
	blue = (unsigned char)b;
}

Color::~Color(void)
{
}

void
Color::Parse(unsigned int c)
{
	alpha = (unsigned char)((0xFF000000 & c) >> 24);
	red = (unsigned char)((0x00FF0000 & c) >> 16);
	green = (unsigned char)((0x0000FF00 & c) >> 8);
	blue = (unsigned char)((0x000000FF & c));
}