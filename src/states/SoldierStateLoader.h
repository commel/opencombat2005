#pragma once

#include <states\ObjectStates.h>

class SoldierStateLoader
{
public:
	static void Load(char *fileName, ObjectStates *states);
};
