#pragma once

#include <Misc\Array.h>

class TGA;
class Building;

class BuildingManager
{
public:
	// Loads a group of widgets into this widget manager
	static void LoadBuildings(char *fileName, Array<Building> *buildings);
};
