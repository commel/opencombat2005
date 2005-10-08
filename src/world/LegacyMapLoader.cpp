#include ".\legacymaploader.h"
#include <misc\readline.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * Legacy maps are defined in the Close Combat txt files. 
 * Legacy maps are broken up into 10x10 blocks, in 4x4
 * sections. So if the map size is 2400x2280 pixels, then in
 * the map file there will be 60x57 = 3420 indices. There are 16
 * entries per index line, which means each line represents 
 * a 4x4 block of 10x10 pixels.
 */
LegacyMapLoader::LegacyMapLoader(void)
{
	_nPixelsPerBlockX = 10;
	_nPixelsPerBlockY = 10;
	_nBlocksPerMacroblockX = 4;
	_nBlocksPerMacroblockY = 4;
}

LegacyMapLoader::~LegacyMapLoader(void)
{
}

/**
 * Load a legacy map file.
 */
void
LegacyMapLoader::Load(char *mapFile)
{
	FILE *fp;
	const int bufferLen = 1024;
	char buffer[bufferLen];
	int nread = 0;
	int mapIdx = 0;

	fp = fopen(mapFile, "r");

	// Let's read the map index
	nread = readline(fp, buffer, bufferLen);
	mapIdx = atoi(buffer);

	// Let's read a blank line
	readline(fp, buffer, bufferLen);

	// Let's read the macroblocks in the x direction
	readline(fp, buffer, bufferLen);
	_nMacroblocksX = atoi(buffer);

	// Let's read the macroblocks in the y direction
	readline(fp, buffer, bufferLen);
	_nMacroblocksY = atoi(buffer);

	// Now read two lines of I don't know what they are yet
	readline(fp, buffer, bufferLen);
	readline(fp, buffer, bufferLen);
	
	// And a line of column headers
	readline(fp, buffer, bufferLen);
	
	// And the '&'
	readline(fp, buffer, bufferLen);
	
	// Allocate the elements and elevation
	_elements = (unsigned short *) calloc(_nMacroblocksX*_nMacroblocksY*_nBlocksPerMacroblockX*_nBlocksPerMacroblockY, sizeof(short));
	_elevation = (unsigned char *) calloc(_nMacroblocksX*_nMacroblocksY*_nBlocksPerMacroblockX*_nBlocksPerMacroblockY, sizeof(char));

	// Now we are ready to start reading in data
	for(int j = 0; j < _nMacroblocksY; ++j) 
	{
		for(int i = 0; i < _nMacroblocksX; ++i) {
			// Let's read in this line
			readline(fp, buffer, bufferLen);
		
			// Now let's tokenize it based on '\t' characters
			// and extract all of the elements out first. Our first token
			// is the index
			char *token = strtok(buffer, "\t\r\n");
			assert(token != NULL);
			
			token = strtok(NULL, "\t\r\n");
			for(int n = 0; n < _nBlocksPerMacroblockY; ++n) 
			{
				for(int m = 0; m < _nBlocksPerMacroblockX; ++m) 
				{
					assert(token != NULL);
					_elements[(n+j*_nBlocksPerMacroblockY)*_nMacroblocksX*_nBlocksPerMacroblockX + m + i*_nBlocksPerMacroblockX] = (unsigned short)atoi(token);
					token = strtok(NULL, "\t\r\n");
				}
			}

			// Now read in the elevations
			for(int n = 0; n < _nBlocksPerMacroblockY; ++n) 
			{
				for(int m = 0; m < _nBlocksPerMacroblockX; ++m) 
				{
					assert(token != NULL);
					_elevation[(n+j*_nBlocksPerMacroblockY)*_nMacroblocksX*_nBlocksPerMacroblockX + m + i*_nBlocksPerMacroblockX] = (unsigned char)atoi(token);
					token = strtok(NULL, "\t\r\n");
				}
			}

			// Now make sure we are at the end of the line
			assert(token == NULL);
		}
	}

	fclose(fp);
}
