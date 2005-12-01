#pragma once

#include <misc\Structs.h>
#include <objects\Target.h>
#include <objects\Formation.h>
#include <states\ObjectActions.h>

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
		Object *TargetObject;
		// The type of the target at which we are shooting
		Target::Type TargetType;
		// The (x,y) location of the target at which we are shooting
		// (for Area targets)
		int X, Y;
	};

	// This is a data structure for finding cover
	struct FindCoverData
	{
		// A flag which indicates whether or not we have found the cover
		// we are seeking
		bool CoverFound;

		// The (i,j) tile location of the cover we are seeking
		int TileI, TileJ;
	};

	// This is a struct used for walking/running/crawling to a destination
	struct TileData
	{
		// The (i,j) location of the tile we are moving towards
		int TileI, TileJ;
	};

	struct FollowFormationData
	{
		// The object we are following
		Object *TargetObject;
		// The formation we are supposed to be in
		Formation::Type TargetFormation;
		// The formation index we are supposed to be in
		int FormationIndex;
		// The spread of the formation we are supposed to be in
		float FormationSpread;
		// The movement style used to follow
		SoldierAction::Action MovementStyle;
	};

	// This structure is used for keeping track of a wait action
	struct WaitData
	{
		long ElapsedTime;
		long WaitTime;
	};

	static bool Handle(Soldier *soldier, Action *action, long dt);

private:
	static bool AtDestination(Soldier *s, Point *p1, Point *p2);
	static bool AtDestination(Soldier *s, int i, int j);
	// Calculates the heading we need to take to get to a new tile
	static Direction CalculateNewHeading(Soldier *soldier, int i, int j);
	// Calculates a new move
	static void MoveSoldier(Soldier *soldier, long dt);
	// Functions that help us determine flocking characteristics
	static void CalculateSeparationForce(Soldier *soldier, Vector2 *force);
	static void CalculateCohesionForce(Soldier *soldier, Vector2 *force);


	// Action handlers
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
	static bool FindCoverActionHandler(Soldier *soldier, Action *action, long dt);
	static bool FollowActionHandler(Soldier *soldier, Action *action, long dt);
	static bool FollowInFormationActionHandler(Soldier *soldier, Action *action, long dt);
	static bool RunToActionHandler(Soldier *soldier, Action *action, long dt);
	static bool WalkToActionHandler(Soldier *soldier, Action *action, long dt);
	static bool WalkSlowToActionHandler(Soldier *soldier, Action *action, long dt);
	static bool CrawlToActionHandler(Soldier *soldier, Action *action, long dt);
	static bool TurnActionHandler(Soldier *soldier, Action *action, long dt);
	static bool DefendActionHandler(Soldier *soldier, Action *action, long dt);
	static bool AmbushActionHandler(Soldier *soldier, Action *action, long dt);
	static bool WaitActionHandler(Soldier *soldier, Action *action, long dt);
	
	static SoldierActionHandler _handlers[];
};
