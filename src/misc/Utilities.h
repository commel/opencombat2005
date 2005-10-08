#pragma once

#include <misc\Structs.h>

class Utilities
{
public:
	Utilities(void);
	~Utilities(void);

	// Finds a heading from (x0,y0) - (x1,y1)
	static Direction FindHeading(int x0, int y0, int x1, int y1);
	static float FindAngle(int x0, int y0, int x1, int y1);

	// Rotate a point
	static void Rotate(Point *dst, const Point *src, float angle);

protected:
	static Direction ConvertAngle(float angle);

};
