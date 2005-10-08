#pragma once

#include <Misc\Color.h>

#define MAX_COLOR_MODIFIERS 1024

class ColorModifierManager
{
public:
	ColorModifierManager();
	virtual ~ColorModifierManager(void);

	void Load(char *configFile);
};
