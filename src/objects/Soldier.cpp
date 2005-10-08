#include ".\soldier.h"
#include <assert.h>
#include <math.h>
#include <string.h>
#include <graphics\Animation.h>
#include <orders\Order.h>
#include <graphics\Screen.h>
#include <objects\Squad.h>
#include <misc\Structs.h>
#include <objects\Weapon.h>
#include <world\World.h>
#include <graphics\Effect.h>
#include <objects\Action.h>
#include <misc\Utilities.h>
#include <graphics\Widget.h>
#include <sound\Sound.h>
#include <application\Globals.h>

static float _currentHeadingAngles[8] = { 0.0f, 1.0f*2.0f*(float)M_PI/8.0f, 2.0f*2.0f*(float)M_PI/8.0f, 3.0f*2.0f*(float)M_PI/8.0f, 4.0f*2.0f*(float)M_PI/8.0f, 5.0f*2.0f*(float)M_PI/8.0f, 6.0f*2.0f*(float)M_PI/8.0f, 7.0f*2.0f*(float)M_PI/8.0f};

// The old close combat used 5 pixels per meter
#define MIN_DISTANCE_BETWEEN_SOLDIERS 5.0f
#define DIRECTION_CHANGE_PAUSE 500

Soldier::Soldier(void) : Object()
{
	_moving = false;
	_currentSquad = NULL;
	_currentStatus = Unit::Healthy;
	_currentAction = Action::Defending;
	_camoIdx = 0;
	_currentTargetType = Target::NoTarget;
	_type = Target::Soldier;
	_bSquadLeader = false;
	_currentWeaponIdx = 0;
	_numWeapons = 0;
	_inVehicle = false;
}

Soldier::~Soldier(void)
{
}

void
Soldier::Render(Screen *screen, Rect *clip)
{
	// Make sure I am in my clipping region
	Region r;
	r.points[0].x = clip->x+screen->Origin.x;r.points[0].y = clip->y+screen->Origin.y;
	r.points[1].x = clip->x+clip->w+screen->Origin.x;r.points[1].y = clip->y+screen->Origin.y;
	r.points[2].x = clip->x+clip->w+screen->Origin.x;r.points[2].y = clip->y+clip->h+screen->Origin.y;
	r.points[3].x = clip->x+screen->Origin.x;r.points[3].y = clip->y+clip->h+screen->Origin.y;
	if(!screen->PointInRegion(Position.x, Position.y, &r)) {
		return;
	}

	// Render any donuts if I need them
	if(!IsInVehicle()) {
		if(IsSquadLeader()) {
			Color white(255,255,255);
			Widget *w = g_Globals->World.Icons->GetWidget("Donut Med Green");
			w->Render(screen, Position.x-screen->Origin.x, Position.y-screen->Origin.y, &white);
			delete w;
		}

		if(_bHighlight) {
			_animations[_currentState]->Render(screen, _currentHeading, Position.x-screen->Origin.x, Position.y-screen->Origin.y, true, &_highlightColor, _camoIdx);
		} else {
			Color yellow(255,255,0);
			_animations[_currentState]->Render(screen, _currentHeading, Position.x-screen->Origin.x, Position.y-screen->Origin.y, IsSelected(), &yellow, _camoIdx);
		}
	}

	// Render any effects
	for(int i = 0; i < _effects.Count; ++i) {
		Effect *e = _effects.Items[i];
		if(e->IsDynamic()) {
			// Set the positions
			e->SetPosition(Position.x, Position.y);
		}
		_effects.Items[i]->Render(screen);
	}
}

void
Soldier::Simulate(long dt, World *world)
{
	UNREFERENCED_PARAMETER(world);
	if(_currentState == State::Dead)
	{
		// No need to simulate anything, cuz we dead
		// Just clear out our orders
		return;
	} else if(_currentState == State::DyingBlownUp
			|| _currentState == State::DyingForward
			|| _currentState == State::DyingBackward)
	{
		// Just update our animation
		_animations[_currentState]->Update(dt);
		_currentFrameCurrentState = _animations[_currentState]->GetCurrentFrameNumber(_currentHeading);
	
				// Am I dead yet?
		if(_currentFrameCurrentState == _animations[_currentHeading]->GetCurrentFrameNumber(_currentHeading)
			&& !_currentAnimationMarker)
		{
			// I am dead now
			switch(_currentState) {
				case State::DyingBackward:
					_currentHeading = (Direction) ((_currentHeading+4) % 8);
					_currentState = State::Dead;
					break;
				case State::DyingBlownUp:
				case State::DyingForward:
				default:
                    _currentState = State::Dead;
					break;
			}
			_currentStatus = Unit::Dead;
		} else {
			if(_currentFrameCurrentState != _animations[_currentHeading]->GetCurrentFrameNumber(_currentHeading))
			{
				_currentAnimationMarker = false;
			}
		}

		// Clear out orders
		Order *o;
		while((o = _orders.Dequeue()) != NULL) {
			o->Release();
		}
		return;
	}

	// Check our current orders
	Order *order = _orders.Peek();

	if(order != NULL) {
		bool handled = false;
		switch(order->GetType()) {
			case Orders::Move:
				// This is a move order, let's head in that direction
				_currentAction = Action::Moving;
				handled = HandleMoveOrder(dt, (MoveOrder *) order, Walking);
				break;
			case Orders::Sneak:
				_currentAction = Action::Crawling;
				handled = HandleMoveOrder(dt, (MoveOrder *) order, Sneaking);
				break;
			case Orders::MoveFast:
				_currentAction = Action::MovingFast;
				handled = HandleMoveOrder(dt, (MoveOrder *) order, Running);
				break;
			case Orders::Destination:
				handled = HandleDestinationOrder((MoveOrder *)order);
				break;
			case Orders::Stop:
				handled = HandleStopOrder((StopOrder *)order);
				break;
			case Orders::Pause:
				handled = HandlePauseOrder((PauseOrder *)order, dt);
				if(!handled) {
					return;
				} else {
					_currentState = (Soldier::State) ((PauseOrder *)order)->GetOldState();
				}
				break;
			case Orders::Fire:
				handled = HandleFireOrder((FireOrder *) order);
				break;

			default:
				handled = true;
				break;
		}

		if(handled) {
			_orders.Dequeue();
			order->Release();
		}
	}

	PlanMovement(dt);
	
	// Now update the animations
	_animations[_currentState]->Update(dt);
	_currentFrameCurrentState = _animations[_currentState]->GetCurrentFrameNumber(_currentHeading);

	// Update any effects
	for(int i = 0; i < _effects.Count; ++i) {
		_effects.Items[i]->Simulate(dt);
		if(_effects.Items[i]->IsCompleted()) {
			delete _effects.RemoveAt(i);
			--i;
		}
	}

	// Update my weapon if I can
	_weapons[_currentWeaponIdx]->Simulate(dt);
	if(_currentState == State::StandingFiring || _currentState == State::ProneFiring
		|| _currentState == State::StandingReloading || _currentState == State::ProneReloading)
	{
		if(_weapons[_currentWeaponIdx]->CanFire()) {
			Shoot(_weapons[_currentWeaponIdx], _currentTarget, _currentTargetType, _currentTargetX, _currentTargetY);
		} else if(_weapons[_currentWeaponIdx]->IsEmpty()) {
			if(_weaponsNumClips[_currentWeaponIdx] > 0) {
				_weapons[_currentWeaponIdx]->Reload();
				--_weaponsNumClips[_currentWeaponIdx];
				_currentAction = Action::Reloading;
			} else {
				_currentState = (_currentState==State::StandingFiring || _currentState==State::StandingReloading) ? _currentState = State::Standing : _currentState = State::Prone;
				_currentAction = Action::Defending;
			}
		} else if(_weapons[_currentWeaponIdx]->IsReloading()) {
			_currentState = (_currentState==State::StandingFiring || _currentState==State::StandingReloading) ? _currentState = State::StandingReloading : _currentState = State::ProneReloading;
		}
	}

	// Now update some of my states, like the dying state
	if(_currentState == State::DyingBlownUp || _currentState == State::DyingForward || _currentState == State::DyingBackward) 
	{
		// Am I dead yet?
		if(_currentFrameCurrentState == _animations[_currentHeading]->GetCurrentFrameNumber(_currentHeading)
			&& !_currentAnimationMarker)
		{
			// I am dead now
			switch(_currentState) {
				case State::DyingBackward:
					_currentHeading = (Direction) ((_currentHeading+4) % 8);
					_currentState = State::Dead;
					break;
				case State::DyingBlownUp:
				case State::DyingForward:
				default:
                    _currentState = State::Dead;
					break;
			}
			_currentStatus = Unit::Dead;
		} else {
			if(_currentFrameCurrentState != _animations[_currentHeading]->GetCurrentFrameNumber(_currentHeading))
			{
				_currentAnimationMarker = false;
			}
		}
	}
}

void
Soldier::SetPosition(int x, int y)
{
	Object::SetPosition(x,y);
	_position.x = (float) x;
	_position.y = (float) y;
}

bool
Soldier::Select(int x, int y)
{
	// I need to find the screen coordinates of this object
	bool rv = Contains(x, y);
	if(rv) {
		Object::Select(true);
	}
	return rv;
}

bool
Soldier::Contains(int x, int y)
{
	Region r;
	_animations[_currentState]->GetExtents(_currentHeading, Position.x, Position.y, &r);
	return Screen::PointInRegion(x, y, &r);
}

bool
Soldier::HandleDestinationOrder(MoveOrder *order)
{
	if(Position.x == order->X && Position.y == order->Y) {
		AddOrder(new StopOrder());

		// If I am the squad leader, then announce we have stopped
		if(_currentSquad != NULL) {
			if(_currentSquad->GetSquadLeader() == this) {
				g_Globals->World.Voices->GetSound("move completed")->Play();	
			}
		} else {
			g_Globals->World.Voices->GetSound("move completed")->Play();			
		}
		return true;
	}
	return false;
}

bool 
Soldier::HandleStopOrder(StopOrder *order)
{
	UNREFERENCED_PARAMETER(order);
	// order can be NULL!
	_moving = false;
	_currentState = Standing;
	_currentAction = Action::Defending;
	_velocity.x = 0; // Stop moving!
	_velocity.y = 0;
	return true;
}

bool
Soldier::HandleMoveOrder(long dt, MoveOrder *order, Soldier::State newState)
{
	UNREFERENCED_PARAMETER(dt);
	// Head to the destination
	MoveOrder *o = new MoveOrder(order->X, order->Y, Orders::Destination);
	AddOrder(o);

	_moving = true;
	_currentState = newState;
	
	// Get my current path from my squad
	_currentPath = _currentSquad->GetCurrentPath();

	// Start at zero velocity
	_velocity.x = 0; // Stop moving!
	_velocity.y = 0;

	return true;
}

/**
 * Movement involves quite a few things. Listed in order:
 *
 * 1.) The squad leader is responsible for pathfinding algorithms. Individual
 *	   squad members simply fill in behind and follow the squad leader.
 * 2.) Soldiers should try to stay a couple pixels away from each other.
 * 3.) If the squad leader is moving, then the individual soldiers should move too.
 * 4.) When the squad leader stops, the individual soldiers should find cover and
 *	   stop too.
 */
void
Soldier::PlanMovement(long dt)
{
	// Am I the squad leader?
	Object *squadLeader=NULL;
	if(_currentSquad != NULL && ((squadLeader = _currentSquad->GetSquadLeader()) == this)) {
		// I am the squad leader
		if(_moving) {
			if(FindPath(_currentPath)) {
				// We are about to change direction, so let's pause for a little
				// bit.
				InsertOrder(new PauseOrder(DIRECTION_CHANGE_PAUSE, _currentState, State::Standing), 0);
			} else {
				// Now test move it
				Vector2 vel, pos;
				Move(&pos, &vel, _currentHeading, dt, _currentState);

				// Go ahead and move this fucker
				_position.x = pos.x;
				_position.y = pos.y;
				_velocity.x = vel.x;
				_velocity.y = vel.y;
				Position.x = (int) _position.x;
				Position.y = (int) _position.y;
			}
			return;
		}
	} else {
		// I am not the squad leader, or I am not in a squad. If I am not in a
		// squad, then I am responsible for handling movement just the
		// same as being a squad leader
		if(_currentSquad == NULL) {
			// There is no squad, so do my movement
			if(_moving) {
				if(FindPath(_currentPath)) {
					// We are about to change direction, so let's pause for a little
					// bit.
					InsertOrder(new PauseOrder(DIRECTION_CHANGE_PAUSE, _currentState, State::Standing), 0);
				} else {
					// Now test move it
					Vector2 vel, pos;
					Move(&pos, &vel, _currentHeading, dt, _currentState);

					// Go ahead and move this fucker
					_position.x = pos.x;
					_position.y = pos.y;
					_velocity.x = vel.x;
					_velocity.y = vel.y;
					Position.x = (int) _position.x;
					Position.y = (int) _position.y;
				}
				return;
			}
		} else {
			// I am not the squad leader, so try to follow the squad leader
			// if we can.
			// Now, is the squad leader moving?
			if(squadLeader->IsMoving()) {
				// The squad leader is moving, so try to follow him
				Point squadLeaderPos;
				squadLeaderPos.x = squadLeader->Position.x;
				squadLeaderPos.y = squadLeader->Position.y;

				// Now, which direction should I be going?
				Direction heading = Utilities::FindHeading(Position.x, Position.y, squadLeaderPos.x, squadLeaderPos.y);

				if(heading != _currentHeading) {
					// We are about to change direction, so let's pause for a little
					// bit, but only if we're not already paused
					Order *o = _orders.Peek();
					if(o == NULL || (o != NULL && (_orders.Peek()->GetType() != Orders::Pause))) {
						_currentHeading = heading;
						InsertOrder(new PauseOrder(DIRECTION_CHANGE_PAUSE, _currentState, Soldier::State::Standing), 0);
						return;
					}
				}
				
				// Try moving to this location.
				Vector2 vel, pos;
				Move(&pos, &vel, heading, dt, _currentState);

				// Now, find out if this move is valid. This means that
				// make sure we arent too close to anyone else.
				Vector2 dist;
				Array<Soldier> *soldiers = _currentSquad->GetSoldiers();
				for(int i = 0; i < soldiers->Count; ++i) {
					Soldier *s = soldiers->Items[i];
					if(s != this) {
						dist.x = s->Position.x - pos.x;
						dist.y = s->Position.y - pos.y;
						if(dist.Magnitude() <= MIN_DISTANCE_BETWEEN_SOLDIERS) {
							// We are too close, so let's not move this fucker
							// yet
							// We are about to change direction, so let's pause for a little
							// bit.
							_currentHeading = heading;
							InsertOrder(new PauseOrder(DIRECTION_CHANGE_PAUSE, _currentState, Soldier::State::Standing), 0);
							return;
						}
					}
				}

				// Go ahead and move this fucker
				_position.x = pos.x;
				_position.y = pos.y;
				_velocity.x = vel.x;
				_velocity.y = vel.y;
				_currentHeading = heading;
				Position.x = (int) _position.x;
				Position.y = (int) _position.y;
				_moving = true;				
				return;
			} else {
				// The squad leader is not moving, so if we are moving,
				// then we need to stop
				if(_moving) {
					// Stop!
					ClearOrders();
					AddOrder(new StopOrder());
					return;
				} else {
					// We are not moving, he's not moving, find
					// cover if we have not already
					// XXX/GWS: Todo
				}
			}
		}
	}
}


// Returns true if our heading changes
bool
Soldier::FindPath(Path *path)
{
	// XXX/GWS: This needs to be a better direction finding algorithm
	Direction oldHeading = _currentHeading;
	if(NULL == path) {
		return false;
	}

	int x=0,y=0;
	g_Globals->World.CurrentWorld->ConvertTileToPosition(path->X, path->Y, &x, &y);

	if(Position.x < x) {
		if(Position.y < y) {
			_currentHeading = SouthEast;
		} else if(Position.y == y) {
			_currentHeading = East;
		} else {
			_currentHeading = NorthEast;
		}
	} else if(Position.x == x) {
		if(Position.y < y) {
			_currentHeading = South;
		} else if(Position.y == y) {
			// Let's move to the next path item!
			_currentPath = _currentPath->Next;

			// If there is nothing left then we are done
			if(NULL == _currentPath) {
				HandleStopOrder(NULL);
			}
		} else {
			_currentHeading = North;
		}
	} else {
		if(Position.y < y) {
			_currentHeading = SouthWest;
		} else if(Position.y == y) {
			_currentHeading = West;
		} else {
			_currentHeading = NorthWest;
		}
	}
	return _currentHeading != oldHeading;
}

void
Soldier::Move(Vector2 *posOut, Vector2 *velOut, Direction heading, long dt, State state)
{
	velOut->x = _velocity.x - _accels[state]*dt*sin(_currentHeadingAngles[heading])/1000.0f;
	velOut->y = _velocity.y + _accels[state]*dt*cos(_currentHeadingAngles[heading])/1000.0f;

	if(velOut->Magnitude() > _speeds[state]) { 
		velOut->Normalize();
		velOut->Multiply(_speeds[state]);
	}

	// Now we need to try moving this object to its new position
	posOut->x = _position.x + velOut->x*fabs(sin(_currentHeadingAngles[heading]))*dt*(float)(g_Globals->World.Constants.PixelsPerMeter)/1000.0f;
	posOut->y = _position.y + velOut->y*fabs(cos(_currentHeadingAngles[heading]))*dt*(float)(g_Globals->World.Constants.PixelsPerMeter)/1000.0f;
}

bool
Soldier::HandlePauseOrder(PauseOrder *order, long dt)
{
	_currentState = (Soldier::State) order->GetPauseState();
	order->IncrementTotalTime(dt);
	return order->GetTotalTime() >= order->GetPauseTime();
}

void 
Soldier::SetTitle(char *title)
{
	assert(strlen(title) < 32);
	strcpy(_title, title);
}

void
Soldier::SetRank(char *rank)
{
	assert(strlen(rank) < 32);
	strcpy(_rank, rank);
}

void
Soldier::SetCamouflage(char *camo)
{
	assert(strlen(camo) < 64);
	strcpy(_camo, camo);

	// Find the color modifier idx for this camo
	for(int i = 0; i < g_NumColorModifiers; ++i) {
		if(strcmp(_camo, g_ColorModifiers[i].Name) == 0) {
			_camoIdx = i;
			return;
		}
	}
}

void
Soldier::UpdateInterfaceState(InterfaceState *state, int teamIdx, int unitIdx)
{
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].Name, GetPersonalName());
	state->SquadStates[teamIdx].UnitStates[unitIdx].CurrentAction = GetCurrentAction();
	state->SquadStates[teamIdx].UnitStates[unitIdx].CurrentStatus = GetCurrentStatus();
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].WeaponIcon, _weapons[_currentWeaponIdx]->GetIconName());
	state->SquadStates[teamIdx].UnitStates[unitIdx].NumRounds = _weapons[_currentWeaponIdx]->GetCurrentRounds() + _weapons[_currentWeaponIdx]->GetRoundsPerClip()*_weaponsNumClips[_currentWeaponIdx];
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].Title, _title);
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].Rank, _rank);
}

bool
Soldier::HandleFireOrder(FireOrder *order)
{
	// Stop moving
	HandleStopOrder(NULL);

	// Set my state
	_currentState = State::StandingFiring;
	_currentAction = Action::Firing;

	// Set the current target and stuff
	_currentTarget = order->Target;
	_currentTargetType = order->TargetType;
	_currentTargetX = order->X;
	_currentTargetY = order->Y;

	return true;
}

void
Soldier::Kill()
{
	// Stop the soldier
	HandleStopOrder(NULL);

	// Kills this soldier
	_currentState = (State) ((rand()%3) + State::DyingBlownUp);

	// Reset the dying animation
	_animations[_currentState]->Reset();

	// Remember the current animation frame
	_currentFrameCurrentState = _animations[_currentState]->GetCurrentFrameNumber(_currentHeading);
	_currentAnimationMarker = true;

	// XXX/GWS: Probably should play a sound here
	g_Globals->World.SoundEffects->GetSound("Dying")->Play();
}

void
Soldier::Shoot(Weapon *weapon, Object *target, Target::Type targetType, int targetX, int targetY)
{
	switch(targetType) {
		case Target::Soldier:
			if(target != NULL) {
				// Make sure my target is not already dead or dying!
				Soldier *s = (Soldier *) target;
				if(s->_currentState == State::Dead
					|| s->_currentState == State::DyingBlownUp
					|| s->_currentState == State::DyingForward
					|| s->_currentState == State::DyingBackward) 
				{
					_currentTarget = s->GetSquad();
					_currentTargetType = Target::Squad;
					return;
				}

				_currentHeading = Utilities::FindHeading(Position.x, Position.y, _currentTarget->Position.x, _currentTarget->Position.y);
				if(((Soldier *)_currentTarget)->CalculateShot(this, weapon)) {
					// We killed the guy, so find another target next time
					_currentTarget = ((Soldier *)_currentTarget)->GetSquad();
					_currentTargetType = Target::Squad;
				}
			}
			break;
		case Target::Squad:
			// Find a target in the squad
			_currentTarget = FindTarget((Squad *) target);
			_currentTargetType = Target::Soldier;
			if(_currentTarget == NULL) {
				_currentAction = Action::NoTarget;
				_currentState = State::Standing;
			}
			return;
		case Target::Area:
			// Set my heading
			_currentHeading = Utilities::FindHeading(Position.x, Position.y, targetX, targetY);
			break;
	}
	weapon->Fire();
			
	_currentState = (_currentState==State::StandingFiring || _currentState==State::StandingReloading) ? _currentState = State::StandingFiring : _currentState = State::ProneFiring;
	_currentAction = Action::Firing;
	_effects.Add(g_Globals->World.Effects->GetEffect(weapon->GetEffect(_currentHeading)));
}

// Let's calculate a shot fired at us
bool
Soldier::CalculateShot(Soldier *shooter, Weapon *weapon)
{
	// How far away are we from the shooter?
	Vector2 rangeVec;
	rangeVec.x = (float)(shooter->Position.x - Position.x);
	rangeVec.y = (float)(shooter->Position.y - Position.y);

	// What's the max effective range of this weapon?
	//int maxRange = weapon->GetMaxEffectiveRange();
	_health -= weapon->GetRoundsPerBurst()*10;
	if(_health <= 0) {
		Kill();
		return true;
	}
	return false;
}

Soldier *
Soldier::FindTarget(Squad *squad)
{
	if(NULL == squad) {
		return NULL;
	}

	bool anyAlive = false;
	Array<Soldier> *soldiers = squad->GetSoldiers();
	for(int i = soldiers->Count-1; i >= 0 ; --i) {
		if(!soldiers->Items[i]->IsDead())
		{
			anyAlive = true;
			break;
		}
	}
	if(anyAlive) {
		Soldier *o;
		for(;;)
		{
			if(!((o = soldiers->Items[rand()%soldiers->Count])->IsDead())) {
				return o;
			}
		}
	}
	return NULL;
}

Unit::Status 
Soldier::GetCurrentStatus()
{
	if(_health < 100 && _health > 0) {
		return Unit::Wounded;
	} else if(_health == HEALTH_MAX) {
		return Unit::Healthy;
	} else if(_health <= 0) {
		return Unit::Dead;
	}
	assert(0);
	return Unit::Dead;
}

bool
Soldier::IsDead()
{
	return _currentState == State::Dead || _currentState == State::DyingBlownUp || _currentState == DyingBackward || _currentState == DyingForward;
}

void 
Soldier::InsertWeapon(Weapon *weapon, int numClips)
{
	// Move all of the weapons down
	assert(_numWeapons < (MAX_WEAPONS_PER_SOLDIER-1));
	for(int i = _numWeapons-1; i >= 0; --i)
	{
		_weapons[i+1] = _weapons[i];
		_weaponsNumClips[i+1] = _weaponsNumClips[i];
	}
	_weapons[0] = weapon;
	_weaponsNumClips[0] = numClips;
}
