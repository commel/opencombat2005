#include ".\soldieractionhandlers.h"
#include <application\Globals.h>
#include <objects\Squad.h>
#include <misc\Structs.h>
#include <sound\Sound.h>
#include <objects\Weapon.h>
#include <misc\Utilities.h>

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

};

bool
SoldierActionHandlers::AtDestination(Soldier *s, Point *p1, Point *p2)
{
	Vector2 dest;
	dest.x = p1->x - p2->x;
	dest.y = p1->y - p2->y;
	return (dest.Magnitude() < 3.0f) || (s->_currentPath == NULL);
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
	return true;
}

bool 
SoldierActionHandlers::ProneFireActionHandler(Soldier *soldier, Action *action, long dt)
{
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
			if(fireData->Target != NULL) {
				// Make sure my target is not already dead or dying!
				Object *o = (Object *) fireData->Target;
				if(o->IsDead())
				{
					// Our target is dead, so we need to find a new one
					fireData->Target = o->GetSquad();
					fireData->TargetType = Target::Squad;
					return false;
				}

				// Update our heading to point at the target
				soldier->_currentHeading = Utilities::FindHeading(soldier->Position.x, soldier->Position.y, fireData->Target->Position.x, fireData->Target->Position.y);
				
				// Take our shot
				((Soldier *)(fireData->Target))->CalculateShot(soldier, soldier->_weapons[soldier->_currentWeaponIdx]);
			}
			break;
		case Target::Squad:
			// Find a target in the squad
			fireData->Target = soldier->FindTarget((Squad *) fireData->Target);
			fireData->TargetType = Target::Soldier;
			if(fireData->Target == NULL) {
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
	if(!soldier->_currentState.IsSet(SoldierState::Running))
	{
		// This is the first time we are calling this handler
		g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
		soldier->_moving = true;
	
		// Get my current path from my squad
		soldier->_currentPath = soldier->_currentSquad->GetCurrentPath();
	
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
	if(!soldier->_currentState.IsSet(SoldierState::Walking))
	{
		g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
		soldier->_moving = true;
	
		// Get my current path from my squad
		soldier->_currentPath = soldier->_currentSquad->GetCurrentPath();
	
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
	return true;
}

bool 
SoldierActionHandlers::CrawlActionHandler(Soldier *soldier, Action *action, long dt)
{
	// Let's do some crawling stuff
	if(!soldier->_currentState.IsSet(SoldierState::Crawling))
	{
		g_Globals->World.Actions.Soldiers.UpdateState(action->Index, &soldier->_currentState);
		soldier->_moving = true;
	
		// Get my current path from my squad
		soldier->_currentPath = soldier->_currentSquad->GetCurrentPath();
	
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