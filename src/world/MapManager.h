#pragma once

#include <misc\Structs.h>
#include <world\VictoryLocation.h>
#include <misc\StrictArray.h>

struct MapAttributes
{
	char Name[MAX_NAME];
	char Background[MAX_NAME];
	char Overland[MAX_NAME];
	char Mini[MAX_NAME];
	char Elements[MAX_NAME];
	char Buildings[MAX_NAME];
	StrictArray<VictoryLocation *> VictoryLocations;
};

class MapManager
{
public:
	MapManager(void);
	virtual ~MapManager(void);

	// Creates a map template from the configuration
	// file
	static MapAttributes *Parse(char *configFile);
};
