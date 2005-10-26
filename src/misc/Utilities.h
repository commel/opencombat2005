#pragma once

#include <misc\Structs.h>

class Utilities
{
public:
	Utilities(void);
	~Utilities(void);

	// Finds a heading from (x0,y0) - (x1,y1)
	static Direction FindHeading(int x0, int y0, int x1, int y1);
	static Direction FindHeading(Vector2 *dir);

	static float FindAngle(int x0, int y0, int x1, int y1);
	static float FindAngle(Vector2 *dir);
	static float FindAngle(Direction dir);

	// Rotate a point
	static void Rotate(Point *dst, const Point *src, float angle);

protected:
	static Direction ConvertAngle(float angle);

};
