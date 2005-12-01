#pragma once

#include <states\State.h>

class ObjectActions
{
public:
	typedef unsigned int StateIdx;
	typedef int ActionIdx;

	struct Action
	{
		char *Name;
		char *Group;
		long Time;
		StateIdx *Requirements;
		int NumRequirements;
		StateIdx *Adds;
		int NumAdds;
		StateIdx *Subtracts;
		int NumSubtracts;
	};

	Action *Actions;
	int NumActions;

	// Checks our requirements for a given action. Returns -1 if we satisfy
	// our requirements, otherwise the first action index we are missing
	ActionIdx CheckRequirements(ActionIdx actionID, State *srcState);

	// Updates our states
	void UpdateState(ActionIdx actionID, State *state);
};

// Let's enum all the actions a soldier can take. Again, this corresponds
// to the SoldierActions.txt file and should be updated accordingly
// XXX/GWS: Read the note above. Also need to update the soldier actions handler
//			array in SoldierActionHandlers.cpp file.
namespace SoldierAction
{
	enum Action
	{
		StandingFire=0,
		ProneFire,
		Run,
		Walk,
		WalkSlow,
		Crawl,
		Stand,
		LieDown,
		Stop,
		DestinationReached,
		Reload,
		FindCover,
		Follow,
		FollowInFormation,
		WalkTo,
		RunTo,
		WalkSlowTo,
		CrawlTo,
		Turn,
		Defend,
		Ambush,
		Wait,
		NumActions // Must be last
	};
};
