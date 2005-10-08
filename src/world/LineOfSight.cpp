#include ".\lineofsight.h"
#include <stdio.h>
#include <world\Map.h>

/**
 * A LOS file is a binary RLE file that codes LOS attributes for every
 * 10x10 pixel tile in the world. Run length encoding is simple,
 * and takes 3 bytes [0xFF AA BB] where 0xFF is the run length encoding
 * marker, AA in the number of repeated entries, and BB is the repeated
 * value. LOS values are in the range 0x01 - 0xFE and are a linear
 * percentage. The value 0x00 specifies the end of the scan line.
 * In the future, we might want to reduce the number
 * of LOS entries to make the filesize smaller or the amount of LOS data
 * smaller, for larger maps.
 *
 * In a map, elevations are in units of 1m increments, so an elevation
 * of 1 unit is about three feet, 2 units is 6 feet, etc.
 */
#define ELEVATION_METERS_PER_UNIT	1   /* 1 unit = 1 meter */
#define MAX_LOS			0xFE
#define MIN_LOS			0x01
#define LOS_RLE_MARKER	0xFF
#define LOS_END_MARKER	0x00
#define HEIGHT_MODIFIER	1024

LineOfSight::LineOfSight(void)
{
}

LineOfSight::~LineOfSight(void)
{
}

/**
 * Exports the line of sight calculations to another file.
 */
void
LineOfSight::Export(Map *map, char *fileName)
{
	// Get the block size of the map
	int sx, sy;
	map->GetNumTiles(&sx, &sy);

	// Create the file for exporting
	FILE *fp = fopen(fileName, "wb");

	// Let's allocate a temporary array for LOS calculations. This array
	// is large enough to hold the calculations for one tile only.
	unsigned char *buffer = (unsigned char *) calloc(sx*sy, sizeof(unsigned char));

	// Now, for each tile on the map, we need to calculate the LOS to all the other
	// tiles. 
	for(int j = 0; j < sy; ++j) {
		for(int i = 0; i < sx; ++i) {
			// We are at tile [i,j]
			CalculateLOSForTile(i, j, sx, sy, map, buffer);
		}
	}
	fclose(fp);
}

bool
LineOfSight::CalculateLOSForTile(int x1, int y1, int x2, int y2, int *outx, int *outy, int *outz, Map *map)
{
    int i, dx, dy, dz, l, m, n, x_inc, y_inc, z_inc, err_1, err_2, dx2, dy2, dz2;
    int pixel[3];
	int z3;
	int z1 = (map->GetTileElevation(x1, y1) + 2)*HEIGHT_MODIFIER;
	int z2 = (map->GetTileElevation(x2, y2))*HEIGHT_MODIFIER;

    pixel[0] = x1;
    pixel[1] = y1;
    pixel[2] = z1;
    dx = x2 - x1;
    dy = y2 - y1;
    dz = z2 - z1;
    x_inc = (dx < 0) ? -1 : 1;
    l = abs(dx);
    y_inc = (dy < 0) ? -1 : 1;
    m = abs(dy);
    z_inc = (dz < 0) ? -1 : 1;
    n = abs(dz);
    dx2 = l << 1;
    dy2 = m << 1;
    dz2 = n << 1;

    if ((l >= m) && (l >= n)) {
        err_1 = dy2 - l;
        err_2 = dz2 - l;
        for (i = 0; i < l; i++) {
			// Check the z value for this pixel
			z3 = map->GetTileElevation(pixel[0], pixel[1])*HEIGHT_MODIFIER;
			if(z3 > pixel[2] && map->IsTileBlockHeight(pixel[0], pixel[1]))
			{
				// We are blocked!!!
				*outx = pixel[0];
				*outy = pixel[1];
				*outz = pixel[2];
				return false;
			}
            if (err_1 > 0) {
                pixel[1] += y_inc;
                err_1 -= dx2;
            }
            if (err_2 > 0) {
                pixel[2] += z_inc;
                err_2 -= dx2;
            }
            err_1 += dy2;
            err_2 += dz2;
            pixel[0] += x_inc;
        }
    } else if ((m >= l) && (m >= n)) {
        err_1 = dx2 - m;
        err_2 = dz2 - m;
        for (i = 0; i < m; i++) {
			z3 = map->GetTileElevation(pixel[0], pixel[1])*HEIGHT_MODIFIER;
			if(z3 > pixel[2] && map->IsTileBlockHeight(pixel[0], pixel[1]))
			{
				// We are blocked!!!
				*outx = pixel[0];
				*outy = pixel[1];
				*outz = pixel[2];
				return false;
			}
            if (err_1 > 0) {
                pixel[0] += x_inc;
                err_1 -= dy2;
            }
            if (err_2 > 0) {
                pixel[2] += z_inc;
                err_2 -= dy2;
            }
            err_1 += dx2;
            err_2 += dz2;
            pixel[1] += y_inc;
        }
    } else {
        err_1 = dy2 - n;
        err_2 = dx2 - n;
        for (i = 0; i < n; i++) {
			z3 = map->GetTileElevation(pixel[0], pixel[1])*HEIGHT_MODIFIER;
			if(z3 > pixel[2] && map->IsTileBlockHeight(pixel[0], pixel[1]))
			{
				// We are blocked!!!
				*outx = pixel[0];
				*outy = pixel[1];
				*outz = pixel[2];
				return false;
			}
			if (err_1 > 0) {
                pixel[1] += y_inc;
                err_1 -= dz2;
            }
            if (err_2 > 0) {
                pixel[0] += x_inc;
                err_2 -= dz2;
            }
            err_1 += dy2;
            err_2 += dx2;
            pixel[2] += z_inc;
        }
    }
	// I guess we can see!
	return true;
}

void 
LineOfSight::CalculateLOSForTile(int si, int sj, int sx, int sy, Map *map, unsigned char *buffer)
{
	int dy, dx, stepx, stepy;
	int x0, y0, x1, y1;

	int currentElevation = map->GetTileElevation(si, sj);
	int elevationDiff;

	// We are at tile [si,sj]. Let's calculate the line of sight to every other
	// tile on the map
	for(int j = 0; j < sy; ++j) {
		for(int i = 0; i < sx; ++i) {
			// Let's use Bresenham's line drawing algorithm
			// to increment pixels from [si,sj] to [i,j]
			x0 = si; y0 = sj;
			x1 = i; y1 = j;
			dy = y1 - y0;
			dx = x1 - x0;
        
	        if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
		    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
			dy <<= 1; // dy is now 2*dy
	        dx <<= 1; // dx is now 2*dx

			// Set the starting pixel to the max LOS value,
			// since we can always see ourself
			buffer[j*sx+i] = MAX_LOS;

	        if (dx > dy) {
		        int fraction = dy - (dx >> 1); // same as 2*dy - dx
	            while (x0 != x1) {
		            if (fraction >= 0) {
			            y0 += stepy;
				        fraction -= dx; // same as fraction -= 2*dx
					}
	                x0 += stepx;
		            fraction += dy; // same as fraction -= 2*dy
					elevationDiff = abs(currentElevation-map->GetTileElevation(x0, y0));
					if(elevationDiff > 2) {
						buffer[j*sx+i] = MIN_LOS;
						break;
					}
				}
	        } else {
		        int fraction = dx - (dy >> 1);
			    while (y0 != y1) {
				    if (fraction >= 0) {
					    x0 += stepx;
						fraction -= dy;
					}
	                y0 += stepy;
		            fraction += dx;
					elevationDiff = abs(currentElevation-map->GetTileElevation(x0, y0));
					if(elevationDiff > 2) {
						buffer[j*sx+i] = MIN_LOS;
						break;
					}
				}
			}
		}
	}
}

