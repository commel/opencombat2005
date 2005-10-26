#include ".\soldieractionhandlers.h"
#include <application\Globals.h>
#include <objects\Squad.h>
#include <misc\Structs.h>
#include <sound\Sound.h>
#include <objects\Weapon.h>
#include <misc\Utilities.h>

static float _currentHeadingAngles[8] = { 0.0f, 1.0f*2.0f*(float)M_PI/8.0f, 2.0f*2.0f*(float)M_PI/8.0f, 3.0f*2.0f*(float)M_PI/8.0f, 4.0f*2.0f*(float)M_PI/8.0f, 5.0f*2.0f*(float)M_PI/8.0f, 6.0f*2.0f*(float)M_PI/8.0f, 7.0f*2.0f*(float)M_PI/8.0f};

// The following are used for figuring out formations and aligmnent
const static float _formations[][9][2] = 
{
	/* Wedge */  { {0.0f,0.0f}, {-7.0f,7.0f}, {-7.0f,-7.0f}, {-12.0f,0.0f}, {-10.0f,-12.0f}, {-16.0f,0.0f}, {-20.0f,7.0f}, {-20.0f,-7.0f}, {-25.0f,0.0f} },
	/* Column */ { {0.0f,0.0f}, {-5.0f,0.0f}, {-10.0f,0.0f},{-15.0f,0.0f}, {-20.0f,0.0f}, {-25.0f,0.0f}, {-30.0f,0.0f}, {-35.0f,0.0f} , {-40.0f,0.0f} },
    /* Line */   { {0.0f,0.0f},{3.0f,7.0f},{3.0f,-7.0f},{6.0f,10.0f},{6.0f,-10.0f},{10.0f,15.0f},{10.0f,-15.0f},{15.0f,10.0f},{15.0f,-10.0f} }
};

// Weights used for our various formation calculations
const static float _cohesionWeight = 1.5f;
const static float _separationWeight = 1.0f;
const static float _alignmentWeight = 1.0f;
const static float _formationWeight = 3.0f;

SoldierActionHandlers::SoldierActionHandler SoldierActionHandlers::_handlers[SoldierAction::NumActions] =
{
	SoldierActionHandlers::StandingFireActionHandler,
	SoldierActionHandlers::ProneFireActionHandler,
	SoldierActionHandlers::RunActionHandler,
	SoldierActionHandlers::WalkActionHandler,
	SoldierActionHandlers::WalkSlowActionHandler,
	SoldierActionHandlers::CrawlActionHandler,
	SoldierActionHandlers::StandActionHandler,
	SoldierActionHandlers::LieDownActionHandler,
	SoldierActionHandlers::StopActionHandler,
	SoldierActionHandlers::DestinationReachedActionHandler,
	SoldierActionHandlers::ReloadActionHandler,
	SoldierActionHandlers::FindCoverActionHandler,
	SoldierActionHandlers::FollowActionHandler,
	SoldierActionHandlers::FollowInFormationActionHandler,
	SoldierActionHandlers::WalkToActionHandler,
	SoldierActionHandlers::RunToActionHandler,
	SoldierActionHandlers::WalkSlowToActionHandler,
	SoldierActionHandlers::CrawlToActionHandler,
	SoldierActionHandlers::TurnActionHandler,
	SoldierActionHandlers::DefendActionHandler,
	SoldierActionHandlers::AmbushActionHandler,

};

bool
SoldierActionHandlers::AtDestination(Soldier *s, Point *p1, Point *p2)
{
	Vector2 dest;
	dest.x = (float)(p1->x - p2->x);
	dest.y = (float)(p1->y - p2->y);
	return (dest.Magnitude() < 3.0f);
}

bool
SoldierActionHandlers::AtDestination(Soldier *s, int i, int j)
{
	// Are we at the tile we are supposed to be at?
	int si=0,sj=0;
	g_Globals->World.CurrentWorld->ConvertPositionToTile(s->Position.x, s->Position.y, &si, &sj);
	return si==i && sj==j;
}

bool 
SoldierActionHandlers::Handle(Soldier *soldier, Action *action, long dt)
{
	int actionIdx = g_Globals->World.Actions.Soldiers.CheckRequirements(action->Index, &(soldier->_currentState));
	if(actionIdx >= 0) {
		// We need to perform this action before we can even attempt
		// the one we are trying to do
		soldier->_actionQueue.Insert(new Action(actionIdx, NULL), 0);
		return false;
	}

	// Now call the individual handler
	return _handlers[action->Index](soldier, action, dt);
}

bool 
SoldierActionHandlers::StandingFireActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(soldier);
	UNREFERENCED_PARAMETER(action);
	UNREFERENCED_PARAMETER(dt);
	return true;
}

bool 
SoldierActionHandlers::ProneFireActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(dt);
	FireActionData *fireData = (FireActionData *)action->Data;

	// We change our attributes each time we fire
	g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
	soldier->_currentAnimationState = Soldier::AnimationState::ProneFiring;
	
	// Continue doing our firing thing. If our weapon is ready,
	// then go ahead and shoot. Otherwise, we need to wait for our weapon to
	// become ready
	if(soldier->_weapons[soldier->_currentWeaponIdx]->CanFire()) 
	{
		// We are able to fire our weapon, so let's go ahead and fire it.
		// We need to make sure that our target is valid, because another person
		// could have come in and killed it already
		switch(fireData->TargetType) 
		{
		case Target::Soldier:
			if(fireData->TargetObject != NULL) {
				// Make sure my target is not already dead or dying!
				Object *o = (Object *) fireData->TargetObject;
				if(o->IsDead())
				{
					// Our target is dead, so we need to find a new one
					fireData->TargetObject = o->GetSquad();
					fireData->TargetType = Target::Squad;
					return false;
				}

				// Update our heading to point at the target
				soldier->_currentHeading = Utilities::FindHeading(soldier->Position.x, soldier->Position.y, fireData->TargetObject->Position.x, fireData->TargetObject->Position.y);
				
				// Take our shot
				((Soldier *)(fireData->TargetObject))->CalculateShot(soldier, soldier->_weapons[soldier->_currentWeaponIdx]);
			}
			break;
		case Target::Squad:
			// Find a target in the squad
			fireData->TargetObject = soldier->FindTarget((Squad *) fireData->TargetObject);
			fireData->TargetType = Target::Soldier;
			if(fireData->TargetObject == NULL) {
				// We don't have any more targets, so we need to stop
				soldier->_currentState.Set(SoldierState::NoTarget);
				
				// We need to stop doing whatever we were doing
				Action *a = new Action(SoldierAction::Stop, NULL);

				// Insert it after our current one
				soldier->_actionQueue.Insert(a, 1);
				return true;
			}
			break;
		case Target::Area:
			// Set my heading
			soldier->_currentHeading = Utilities::FindHeading(soldier->Position.x, soldier->Position.y, fireData->X, fireData->Y);
			break;
		}

		soldier->_weapons[soldier->_currentWeaponIdx]->Fire();
		soldier->_currentAction = Unit::Firing;
		soldier->_effects.Add(g_Globals->World.Effects->GetEffect(soldier->_weapons[soldier->_currentWeaponIdx]->GetEffect(soldier->_currentHeading)));
	} 
	else if(soldier->_weapons[soldier->_currentWeaponIdx]->IsEmpty()) 
	{
		// We need to reload our weapon!
		soldier->_currentState.UnSet(SoldierState::Reloaded);
	}
	return false;
}

bool 
SoldierActionHandlers::RunActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(dt);
	if(!soldier->_currentState.IsSet(SoldierState::Running))
	{
		// This is the first time we are calling this handler
		g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
		soldier->_moving = true;
	
		// Get my current path from my squad
		//soldier->_currentPath = soldier->_currentSquad->GetCurrentPath();
	
		// Set the current animation state
		soldier->_currentAnimationState = Soldier::AnimationState::Running;

		// Start at zero velocity
		soldier->_velocity.x = 0; // Stop moving!
		soldier->_velocity.y = 0;
		return false;
	}
	else
	{
		// Check to see if we are at our destination. If we are,
		// then we can stop this action
		Point *p = (Point *)action->Data;
		if(AtDestination(soldier, p, &(soldier->Position))) 
		{
			delete p;
			// Add a stop action
			Action *a = new Action(SoldierAction::DestinationReached, NULL);
			// Insert it after our current one
			soldier->_actionQueue.Insert(a, 1);
			return true;
		}
	}
	return false;
}

bool 
SoldierActionHandlers::WalkActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(dt);

	if(!soldier->_currentState.IsSet(SoldierState::Walking))
	{
		g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
		soldier->_moving = true;
	
		// Get my current path from my squad
		//soldier->_currentPath = soldier->_currentSquad->GetCurrentPath();
	
		// Set the current animation state
		soldier->_currentAnimationState = Soldier::AnimationState::Walking;

		// Start at zero velocity
		soldier->_velocity.x = 0; // Stop moving!
		soldier->_velocity.y = 0;
		return false;
	}
	else
	{
		// Check to see if we are at our destination. If we are,
		// then we can stop this action
		Point *p = (Point *)action->Data;
		if(AtDestination(soldier, p, &(soldier->Position))) 
		{
			delete p;
			// Add a stop action
			Action *a = new Action(SoldierAction::DestinationReached, NULL);
			// Insert it after our current one
			soldier->_actionQueue.Insert(a, 1);
			return true;
		}
	}
	return false;
}

bool 
SoldierActionHandlers::WalkSlowActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(dt);
	UNREFERENCED_PARAMETER(soldier);
	UNREFERENCED_PARAMETER(action);
	return true;
}

bool 
SoldierActionHandlers::CrawlActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(dt);

	// Let's do some crawling stuff
	if(!soldier->_currentState.IsSet(SoldierState::Crawling))
	{
		g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
		soldier->_moving = true;
	
		// Get my current path from my squad
		//soldier->_currentPath = soldier->_currentSquad->GetCurrentPath();
	
		// Set the current animation state
		soldier->_currentAnimationState = Soldier::AnimationState::Sneaking;

		// Start at zero velocity
		soldier->_velocity.x = 0; // Stop moving!
		soldier->_velocity.y = 0;
		return false;
	}
	else
	{
		// Check to see if we are at our destination. If we are,
		// then we can stop this action
		Point *p = (Point *)action->Data;
		if(AtDestination(soldier, p, &(soldier->Position))) 
		{
			delete p;
			// Add a stop action
			Action *a = new Action(SoldierAction::DestinationReached, NULL);
			// Insert it after our current one
			soldier->_actionQueue.Insert(a, 1);
			return true;
		}
	}
	return false;
}

bool 
SoldierActionHandlers::StandActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(dt);

	// We have all of the right states, so let's set
	// our animation states
	if(Soldier::AnimationState::StandingUp == soldier->_currentAnimationState)
	{
		// Are we done with this animation?
		int currentFrameNumber = soldier->_animations[soldier->_currentHeading]->GetCurrentFrameNumber(soldier->_currentHeading);
		if(soldier->_currentFrameCurrentState == currentFrameNumber && !soldier->_currentAnimationMarker)
		{
			// We are done with this action
			g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
			soldier->_currentAnimationState = Soldier::AnimationState::Standing;
			return true;
		}
		else
		{
			if(soldier->_currentFrameCurrentState != soldier->_animations[soldier->_currentHeading]->GetCurrentFrameNumber(soldier->_currentHeading))
			{
				soldier->_currentAnimationMarker = false;
			}
		}
	}
	else
	{
		soldier->_currentAnimationState = Soldier::AnimationState::StandingUp;
		soldier->_animations[soldier->_currentAnimationState]->Reset();
		soldier->_currentFrameCurrentState = soldier->_animations[soldier->_currentAnimationState]->GetCurrentFrameNumber(soldier->_currentHeading);
		soldier->_currentAnimationMarker = true;
	}
	return false;
}

bool 
SoldierActionHandlers::LieDownActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(dt);

	// We have all of the right states, so let's set
	// our animation states
	if(Soldier::AnimationState::LyingDown == soldier->_currentAnimationState)
	{
		// Are we done with this animation?
		int currentFrameNumber = soldier->_animations[soldier->_currentHeading]->GetCurrentFrameNumber(soldier->_currentHeading);
		if(soldier->_currentFrameCurrentState == currentFrameNumber && !soldier->_currentAnimationMarker)
		{
			// We are done with this action
			g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
			soldier->_currentAnimationState = Soldier::AnimationState::Prone;
			return true;
		}
		else
		{
			if(soldier->_currentFrameCurrentState != soldier->_animations[soldier->_currentHeading]->GetCurrentFrameNumber(soldier->_currentHeading))
			{
				soldier->_currentAnimationMarker = false;
			}
		}
	}
	else
	{
		soldier->_currentAnimationState = Soldier::AnimationState::LyingDown;
		soldier->_animations[soldier->_currentAnimationState]->Reset();
		soldier->_currentFrameCurrentState = soldier->_animations[soldier->_currentAnimationState]->GetCurrentFrameNumber(soldier->_currentHeading);
		soldier->_currentAnimationMarker = true;
	}
	return false;
}

bool 
SoldierActionHandlers::StopActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(dt);

	soldier->_moving = false;

	// We are done with this action
	g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);

	if(soldier->_currentState.IsSet(SoldierState::Standing))
	{
		soldier->_currentAnimationState = Soldier::AnimationState::Standing;
	}
	else if(soldier->_currentState.IsSet(SoldierState::Prone))
	{
		soldier->_currentAnimationState = Soldier::AnimationState::Prone;
	}

	soldier->_currentAction = Unit::Defending;
	soldier->_velocity.x = 0; // Stop moving!
	soldier->_velocity.y = 0;
	return true;
}

bool 
SoldierActionHandlers::DestinationReachedActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(dt);
	UNREFERENCED_PARAMETER(action);

	if(soldier->IsSquadLeader())
	{
		if(soldier->_currentSquad != NULL)
		{
			soldier->_currentSquad->ClearMarks();
		}
		g_Globals->World.Voices->GetSound("move completed")->Play();	
	}

	return true;
}

bool 
SoldierActionHandlers::ReloadActionHandler(Soldier *soldier, Action *action, long dt)
{
	UNREFERENCED_PARAMETER(dt);

	if(soldier->_currentState.IsSet(SoldierState::Reloading))
	{
		if(!soldier->_weapons[soldier->_currentWeaponIdx]->IsReloading())
		{
			// We are done reloading, so clean up
			g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
			soldier->_currentState.UnSet(SoldierState::Reloading);
			return true;
		}
		return false;
	}
	else
	{
		// We need to start reloading this weapon
		if(soldier->_weapons[soldier->_currentWeaponIdx]->IsEmpty())
		{
			if(soldier->_weaponsNumClips[soldier->_currentWeaponIdx] > 0) {
				soldier->_currentState.Set(SoldierState::Reloading);
				soldier->_weapons[soldier->_currentWeaponIdx]->Reload();
				--soldier->_weaponsNumClips[soldier->_currentWeaponIdx];
				soldier->_currentAction = Unit::Reloading;

				if(soldier->_currentState.IsSet(SoldierState::Standing))
				{
					soldier->_currentAnimationState = Soldier::AnimationState::StandingReloading;
				}
				else if(soldier->_currentState.IsSet(SoldierState::Prone))
				{
					soldier->_currentAnimationState = Soldier::AnimationState::ProneReloading;
				}
			}
			else
			{
				// We cannot reload this weapon, because we are out of ammo
				soldier->_currentState.Set(SoldierState::OutOfAmmo);
				
				// We need to stop doing whatever we were doing
				Action *a = new Action(SoldierAction::Stop, NULL);
				// Insert it after our current one
				soldier->_actionQueue.Insert(a, 1);
				return true;
			}
		}
		else
		{
			// The weapon is not empty, so we do not need to reload
			g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
			soldier->_currentState.UnSet(SoldierState::Reloading);
			return true;
		}
	}
	return false;
}

bool 
SoldierActionHandlers::FindCoverActionHandler(Soldier *soldier, Action *action, long dt)
{
	FindCoverData *data = (FindCoverData *) action->Data;

	if(data->CoverFound)
	{
		// We have found the cover tile we are looking for, so let's
		// see if we are there yet. If we aren't, then we need to move there.
	}
	else
	{
		// We have not yet found the tile we are looking for, so let's
		// pick a tile to give us good cover and move there.
	}
	return true;
}

bool 
SoldierActionHandlers::RunToActionHandler(Soldier *soldier, Action *action, long dt)
{
	TileData *data = (TileData *) action->Data;

	// Make sure our moving flag is set
	soldier->_moving = true;

	// Set the current animation state
	soldier->_currentAnimationState = Soldier::AnimationState::Running;

	// Update our state
	g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);

	// Are we at our destination yet?
	if(AtDestination(soldier, data->TileI, data->TileJ))
	{
		// We are at our destination, so let's stop
		delete data;
		action->Data = NULL;
		return true;
	}

	// We need to find the direction we are supposed to be pointing
	Direction newHeading = CalculateNewHeading(soldier, data->TileI, data->TileJ);
	Direction oldHeading = soldier->_currentHeading;

	// If our heading needs to change, then do it
	if(oldHeading != newHeading)
	{
		soldier->_velocity.x = 0.0f;
		soldier->_velocity.y = 0.0f;
		soldier->_currentHeading = newHeading;
	}

	// Continue on our path
	MoveSoldier(soldier, dt);
	return false;
}

bool 
SoldierActionHandlers::WalkToActionHandler(Soldier *soldier, Action *action, long dt)
{
	TileData *data = (TileData *) action->Data;

	// Make sure our moving flag is set
	soldier->_moving = true;

	// Set the current animation state
	soldier->_currentAnimationState = Soldier::AnimationState::Walking;

	// Update our state
	g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);

	// Are we at our destination yet?
	if(AtDestination(soldier, data->TileI, data->TileJ))
	{
		// We are at our destination, so let's stop
		delete data;
		action->Data = NULL;
		return true;
	}

	// We need to find the direction we are supposed to be pointing
	Direction newHeading = CalculateNewHeading(soldier, data->TileI, data->TileJ);
	Direction oldHeading = soldier->_currentHeading;

	// If our heading needs to change, then do it
	if(oldHeading != newHeading)
	{
		soldier->_velocity.x = 0.0f;
		soldier->_velocity.y = 0.0f;
		soldier->_currentHeading = newHeading;
	}

	// Continue on our path
	MoveSoldier(soldier, dt);
	return false;
}

bool 
SoldierActionHandlers::WalkSlowToActionHandler(Soldier *soldier, Action *action, long dt)
{
	return true;
}

bool 
SoldierActionHandlers::CrawlToActionHandler(Soldier *soldier, Action *action, long dt)
{
	TileData *data = (TileData *) action->Data;

	// Make sure our moving flag is set
	soldier->_moving = true;

	// Set the current animation state
	soldier->_currentAnimationState = Soldier::AnimationState::Sneaking;

	// Update our state
	g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);

	// Are we at our destination yet?
	if(AtDestination(soldier, data->TileI, data->TileJ))
	{
		// We are at our destination, so let's stop
		delete data;
		action->Data = NULL;
		return true;
	}

	// We need to find the direction we are supposed to be pointing
	Direction newHeading = CalculateNewHeading(soldier, data->TileI, data->TileJ);
	Direction oldHeading = soldier->_currentHeading;

	// If our heading needs to change, then do it
	if(oldHeading != newHeading)
	{
		soldier->_velocity.x = 0.0f;
		soldier->_velocity.y = 0.0f;
		soldier->_currentHeading = newHeading;
	}

	// Continue on our path
	MoveSoldier(soldier, dt);
	return false;
}

bool 
SoldierActionHandlers::FollowActionHandler(Soldier *soldier, Action *action, long dt)
{
	return true;
}

bool 
SoldierActionHandlers::FollowInFormationActionHandler(Soldier *soldier, Action *action, long dt)
{
	FollowFormationData *data = (FollowFormationData *) action->Data;

	// We need to check the requirements for the movement style we are going to use
	int actionIdx = g_Globals->World.Actions.Soldiers.CheckRequirements(data->MovementStyle, &(soldier->_currentState));
	if(actionIdx >= 0) {
		// We need to perform this action before we can even attempt
		// the one we are trying to do
		soldier->_actionQueue.Insert(new Action(actionIdx, NULL), 0);
		return false;
	}

	// Update our state
	g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);

	// Now update the state of the movement style we are using
	g_Globals->World.Actions.Soldiers.UpdateState(data->MovementStyle, &soldier->_currentState);

	// Set the current animation state
	if(soldier->_currentState.IsSet(SoldierState::Running))
	{
		soldier->_currentAnimationState = Soldier::AnimationState::Running;
	}
	else if(soldier->_currentState.IsSet(SoldierState::Walking))
	{
		soldier->_currentAnimationState = Soldier::AnimationState::Walking;
	}
	else if(soldier->_currentState.IsSet(SoldierState::WalkingSlow))
	{
		// XXX/GWS: This needs to be corrected!
		soldier->_currentAnimationState = Soldier::AnimationState::Walking;
	}
	else if(soldier->_currentState.IsSet(SoldierState::Crawling))
	{
		soldier->_currentAnimationState = Soldier::AnimationState::Sneaking;
	}

	// Let's first steer for alignment
	Vector2 alignmentForce;
	data->TargetObject->GetGeneralHeading(&alignmentForce);

	// Let's steer for separation
	Vector2 separationForce;
	//CalculateSeparationForce(&separationForce);

	// Let's steer for cohesion
	Vector2 cohesionForce;
	//CalculateCohesionForce(&cohesionForce);

	// Calculate formation
	Vector2 formationForce;
	
	// Rotate the formation thing
	Point point, formation;
	point.x = _formations[data->TargetFormation][data->FormationIndex][0]*data->FormationSpread;
	point.y = _formations[data->TargetFormation][data->FormationIndex][1]*data->FormationSpread;

	// Now if our alignment force is zero, then we
	// need to line up based on our point man's heading
	if(alignmentForce.Magnitude() == 0.0f)
	{
		// XXX/GWS: Our y values seem to be flipped here!
		Utilities::Rotate(&formation, &point, Utilities::FindAngle(data->TargetObject->GetHeading()));
	}
	else
	{
		// XXX/GWS: Worry about divide by zero!!!
		Utilities::Rotate(&formation, &point, atan(alignmentForce.y / alignmentForce.x));
	}

	formationForce.x = data->TargetObject->Position.x + formation.x - soldier->Position.x;
	formationForce.y = data->TargetObject->Position.y - formation.y - soldier->Position.y;
	
	// If we are already in position and our point man is at the end
	// of his path, then we can stop. Eventually, we are going to want 
	// to find cover here
	if(data->TargetObject->IsStopped())
	{
		// Let's make sure we get to the position we need to get to
		Action *newAction = new Action(data->MovementStyle, NULL);
		TileData *tileData = new TileData();
		g_Globals->World.CurrentWorld->ConvertPositionToTile(data->TargetObject->Position.x + formation.x, data->TargetObject->Position.y - formation.y, &tileData->TileI, &tileData->TileJ);
		newAction->Data = tileData;
		soldier->_actionQueue.Insert(newAction, 1);

		// Make sure we are all facing in the right direction
		newAction = new Action(SoldierAction::Turn, (void *) data->TargetObject->GetHeading());
		soldier->_actionQueue.Insert(newAction, 2);

		delete data;
		return true;
	}

	// Otherwise normalize our force
	formationForce.Normalize();

	// Apply our weights
	Vector2 totalForce;
	totalForce.x = _formationWeight*formationForce.x+_separationWeight*separationForce.x + _cohesionWeight*cohesionForce.x + _alignmentWeight*alignmentForce.x;
	totalForce.y = _formationWeight*formationForce.y+_separationWeight*separationForce.y + _cohesionWeight*cohesionForce.y + _alignmentWeight*alignmentForce.y;
	totalForce.Normalize();

	// This should be our general direction as well
	Direction heading = Utilities::FindHeading(&totalForce);

	if(heading != soldier->_currentHeading)
	{
		soldier->_velocity.x = 0.0f;
		soldier->_velocity.y = 0.0f;
		soldier->_currentHeading = heading;
	}

	// Now do our movement
	MoveSoldier(soldier, dt);
	return false;
}

bool 
SoldierActionHandlers::TurnActionHandler(Soldier *soldier, Action *action, long dt)
{
	int direction = (int)action->Data;
	soldier->_currentHeading = (Direction)direction;
	return true;
}

bool 
SoldierActionHandlers::DefendActionHandler(Soldier *soldier, Action *action, long dt)
{
	// Update our state
	g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);

	// XXX/GWS: Perform whatever AI we need to do this action!	
	return false;
}

bool 
SoldierActionHandlers::AmbushActionHandler(Soldier *soldier, Action *action, long dt)
{
	// Update our state
	g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);

	// XXX/GWS: Perform whatever AI we need to do this action!
	return false;
}

// Calculates the heading we need to take to get to a new tile
Direction 
SoldierActionHandlers::CalculateNewHeading(Soldier *soldier, int i, int j)
{
	Direction heading = South;
	int x=0,y=0;

	g_Globals->World.CurrentWorld->ConvertTileToPosition(i, j, &x, &y);

	if(soldier->Position.x < x) {
		if(soldier->Position.y < y) {
			heading = SouthEast;
		} else if(soldier->Position.y == y) {
			heading = East;
		} else {
			heading = NorthEast;
		}
	} else if(soldier->Position.x == x) {
		if(soldier->Position.y < y) {
			heading = South;
		} else if(soldier->Position.y == y) {
			// This case should never happen, because
			// if it did we would be at our destination!
			assert(false);
		} else {
			heading = North;
		}
	} else {
		if(soldier->Position.y < y) {
			heading = SouthWest;
		} else if(soldier->Position.y == y) {
			heading = West;
		} else {
			heading = NorthWest;
		}
	}
	return heading;
}

// Calculates a new move
void 
SoldierActionHandlers::MoveSoldier(Soldier *soldier, long dt)
{
	float accel = 0.0f;
	float maxSpeed = 0.0f;

	if(soldier->_currentState.IsSet(SoldierState::Running))
	{
		accel = soldier->_runningAccel;
		maxSpeed = soldier->_maxRunningSpeed;
	}
	else if(soldier->_currentState.IsSet(SoldierState::Walking))
	{
		accel = soldier->_walkingAccel;
		maxSpeed = soldier->_maxWalkingSpeed;
	}
	else if(soldier->_currentState.IsSet(SoldierState::WalkingSlow))
	{
		accel = soldier->_walkingSlowAccel;
		maxSpeed = soldier->_maxWalkingSlowSpeed;
	}
	else if(soldier->_currentState.IsSet(SoldierState::Crawling))
	{
		accel = soldier->_crawlingAccel;
		maxSpeed = soldier->_maxCrawlingSpeed;
	}

	soldier->_velocity.x -= accel*dt*sin(_currentHeadingAngles[soldier->_currentHeading])/1000.0f;
	soldier->_velocity.y += accel*dt*cos(_currentHeadingAngles[soldier->_currentHeading])/1000.0f;

	if(soldier->_velocity.Magnitude() > maxSpeed) { 
		soldier->_velocity.Normalize();
		soldier->_velocity.Multiply(maxSpeed);
	}

	// Now we need to try moving this object to its new position
	soldier->_position.x += soldier->_velocity.x*fabs(sin(_currentHeadingAngles[soldier->_currentHeading]))*dt*(float)(g_Globals->World.Constants.PixelsPerMeter)/1000.0f;
	soldier->_position.y += soldier->_velocity.y*fabs(cos(_currentHeadingAngles[soldier->_currentHeading]))*dt*(float)(g_Globals->World.Constants.PixelsPerMeter)/1000.0f;

	// Go ahead and move this fucker
	Point oldPosition = soldier->Position;
	soldier->Position.x = (int) soldier->_position.x;
	soldier->Position.y = (int) soldier->_position.y;
	g_Globals->World.CurrentWorld->MoveObject(soldier, &oldPosition, &soldier->Position);
}

void
SoldierActionHandlers::CalculateSeparationForce(Soldier *soldier, Vector2 *force)
{
	Array<Soldier> *soldiers = soldier->_currentSquad->GetSoldiers();

	// Find the average position of all our soldiers
	Vector2 pos;
	pos.x = 0.0f;
	pos.y = 0.0f;
	int nPos = 0;
	for(int i = 0; i < soldiers->Count; ++i)
	{
		if(!soldiers->Items[i]->IsDead() && soldiers->Items[i]->GetID() != soldier->GetID())
		{
			pos.x += soldiers->Items[i]->Position.x;
			pos.y += soldiers->Items[i]->Position.y;
			++nPos;
		}
	}

	if(nPos > 0)
	{
		pos.x /= (float)nPos;
		pos.y /= (float)nPos;
	}

	force->x = ((float)soldier->Position.x)-pos.x;
	force->y = ((float)soldier->Position.y)-pos.y;
	force->Normalize();
}

void
SoldierActionHandlers::CalculateCohesionForce(Soldier *soldier, Vector2 *force)
{
	Array<Soldier> *soldiers = soldier->_currentSquad->GetSoldiers();

	// Find the average position of all our soldiers
	Vector2 pos;
	pos.x = 0.0f;
	pos.y = 0.0f;
	int nPos = 0;
	for(int i = 0; i < soldiers->Count; ++i)
	{
		if(!soldiers->Items[i]->IsDead() && soldiers->Items[i]->GetID() != soldier->GetID())
		{
			pos.x += soldiers->Items[i]->Position.x;
			pos.y += soldiers->Items[i]->Position.y;
			++nPos;
		}
	}

	if(nPos > 0)
	{
		pos.x /= (float)nPos;
		pos.y /= (float)nPos;
	}

	force->x = pos.x - ((float)soldier->Position.x);
	force->y = pos.y - ((float)soldier->Position.y);
	force->Normalize();
}
