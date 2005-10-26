#pragma once

#include <math.h>

#ifndef MAX_NAME
#define MAX_NAME	256
#endif

enum Direction {
	South=0,
	SouthWest,
	West,
	NorthWest,
	North,
	NorthEast,
	East,
	SouthEast,
	NumDirections
};

struct Point
{
	Point() {x=0;y=0;}
	Point(int _x, int _y) {x=_x;y=_y;}
	int x;
	int y;
};

struct Point16
{
	unsigned short int x;
	unsigned short int y;
};

struct Size
{
	int w;
	int h;
};

struct Rect
{
	int x;
	int y;
	int w;
	int h;
};

struct Bounds
{
	int x0, y0, x1, y1;
};

struct Vertex
{
    float x;
    float y;
    float z;
    float rhw;
    unsigned long colour;
    float u;
    float v;
};

struct Vector2
{
	float x;
	float y;

	Vector2() { x = 0.0f; y = 0.0f; }
	inline float Magnitude() { return sqrt(x*x + y*y); }
	inline void Normalize() { float mag = Magnitude(); x = x / mag; y = y / mag; }
	inline void Multiply(float scalar) { x = x*scalar; y = y*scalar; }
};

struct Region
{
	Point points[4];
};
