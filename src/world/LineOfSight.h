#pragma once

class Map;

class LineOfSight
{
public:
	LineOfSight(void);
	virtual ~LineOfSight(void);

	// Exports line of sight information to a file
	void Export(Map *map, char *fileName);

	// Calculates the LOS from (x1,y1) to (x2,y2)
	bool CalculateLOSForTile(int x1, int y1, int x2, int y2, int *ox, int *oy, int *oz, Map *map);

protected:
	// Calculates the LOS for the tile [i,j] to every other tile
	void CalculateLOSForTile(int i, int j, int sx, int sy, Map *map, unsigned char *buffer);
	
};
