#pragma once

struct ColorModifier {
	int Red;
	int Green;
	int Blue;
};

struct ColorModifiers {
	char Name[256];
	ColorModifier Body;
	ColorModifier Legs;
	ColorModifier Head;
	ColorModifier Belt;
	ColorModifier Boots;
	ColorModifier Weapon;
};

extern ColorModifiers g_ColorModifiers[];
extern int g_NumColorModifiers;

class Color
{
public:
	Color(void);
	Color(int r, int g, int b);
	virtual ~Color(void);

	void Parse(unsigned int c);

	unsigned char alpha,red,green,blue;
};
