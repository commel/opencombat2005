#pragma once

#include <misc\Structs.h>
#include <objects\Target.h>

class Object;
class Soldier;
struct Action;

class SoldierActionHandlers
{
public:
	// Let's define the callback function for an action handler
	typedef bool (*SoldierActionHandler) (Soldier *soldier, Action *action, long dt);

	// Let's define some structs that we will use to pass data around
	struct FireActionData
	{
		// The object at which we are firing
		Object *Target;
		// The type of the target at which we are shooting
		Target::Type TargetType;
		// The (x,y) location of the target at which we are shooting
		// (for Area targets)
		int X, Y;
	};

	static bool Handle(Soldier *soldier, Action *action, long dt);

private:
	static bool AtDestination(Soldier *s, Point *p1, Point *p2);
	static bool StandingFireActionHandler(Soldier *soldier, Action *action, long dt);
	static bool ProneFireActionHandler(Soldier *soldier, Action *action, long dt);
	static bool RunActionHandler(Soldier *soldier, Action *action, long dt);
	static bool WalkActionHandler(Soldier *soldier, Action *action, long dt);
	static bool WalkSlowActionHandler(Soldier *soldier, Action *action, long dt);
	static bool CrawlActionHandler(Soldier *soldier, Action *action, long dt);
	static bool StandActionHandler(Soldier *soldier, Action *action, long dt);
	static bool LieDownActionHandler(Soldier *soldier, Action *action, long dt);
	static bool StopActionHandler(Soldier *soldier, Action *action, long dt);
	static bool DestinationReachedActionHandler(Soldier *soldier, Action *action, long dt);
	static bool ReloadActionHandler(Soldier *soldier, Action *action, long dt);

	static SoldierActionHandler _handlers[];
};
