#pragma once

class LegacyMapLoader
{
public:
	LegacyMapLoader(void);
	virtual ~LegacyMapLoader(void);

	/* Loads a legacy map file */
	virtual void Load(char *mapFile);

	/* Retrieves the elements array */
	virtual unsigned short *GetElements() { return _elements; }

	/* Retrieves the elevation */
	virtual unsigned char *GetElevations() { return _elevation; }

	inline void GetNumPixelsPerBlock(int *x, int *y) { *x = _nPixelsPerBlockX; *y = _nPixelsPerBlockY; }
	inline void GetNumBlocks(int *x, int *y) { *x = _nMacroblocksX*_nBlocksPerMacroblockX; *y = _nMacroblocksY*_nBlocksPerMacroblockY; }

protected:
	int _nPixelsPerBlockX, _nPixelsPerBlockY;
	int _nBlocksPerMacroblockX, _nBlocksPerMacroblockY;
	int _nMacroblocksX, _nMacroblocksY;
	int _nPixelsX, _nPixelsY;

	// The elements in the map
	unsigned short *_elements;

	// The elevations in the map
	unsigned char *_elevation;

};
