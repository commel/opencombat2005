#pragma once

#include <misc\Color.h>

#define MAX_COLORS 1024

class ColorManager
{
public:
	ColorManager();
	virtual ~ColorManager(void);

	void Load(char *configFile);
	void CopyColor(char *name, Color *dest);

private:
	Color _colors[MAX_COLORS];
	char _names[MAX_COLORS][32];
	int _nColors;
};
