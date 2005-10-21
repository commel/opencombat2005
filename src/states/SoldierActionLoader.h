#pragma once

#include <states\ObjectActions.h>

class SoldierActionLoader
{
public:
	static void Load(char *fileName, ObjectActions *actions);
};