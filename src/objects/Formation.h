#pragma once

#include <misc\Structs.h>

class Formation
{
public:
	enum Type
	{
		Column,
		File,
		Line,
		/*Scatter,*/
		NumFormations
	};

	static void GetFormationPosition(Type formationType, int formationIdx, float formationSpread, int *x, int *y);
	static void GetFormationPosition(Type formationType, int formationIdx, float formationSpread, Point *src, Direction heading, int *x, int *y);
};
