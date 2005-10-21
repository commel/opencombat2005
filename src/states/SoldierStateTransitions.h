#pragma once

typedef  void(*SoldierStateTransitionFunction) ();

struct SoldierStateTransitions
{
	// This is a 2D array [NumStates, NumStates] of transition
	// functions
	SoldierStateTransitionFunction **TransitionFunctions;
	int NumStates;
};
