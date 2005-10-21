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
#include <misc\Utilities.h>
#include <graphics\Widget.h>
#include <sound\Sound.h>
#include <application\Globals.h>
#include <objects\SoldierActionHandlers.h>

static float _currentHeadingAngles[8] = { 0.0f, 1.0f*2.0f*(float)M_PI/8.0f, 2.0f*2.0f*(float)M_PI/8.0f, 3.0f*2.0f*(float)M_PI/8.0f, 4.0f*2.0f*(float)M_PI/8.0f, 5.0f*2.0f*(float)M_PI/8.0f, 6.0f*2.0f*(float)M_PI/8.0f, 7.0f*2.0f*(float)M_PI/8.0f};

// The old close combat used 5 pixels per meter
#define MIN_DISTANCE_BETWEEN_SOLDIERS 5.0f
#define DIRECTION_CHANGE_PAUSE 500

Soldier::Soldier(void) : Object()
{
	_moving = false;
	_currentSquad = NULL;
	_currentStatus = Unit::Healthy;
	_currentAction = Unit::Defending;
	_camoIdx = 0;
	_currentTargetType = Target::NoTarget;
	_type = Target::Soldier;
	_bSquadLeader = false;
	_currentWeaponIdx = 0;
	_numWeapons = 0;
	_inVehicle = false;
	_currentState.Set(SoldierState::Standing);
	_currentState.Set(SoldierState::Stopped);
	_currentAnimationState = Soldier::Standing;
	
	// Initialize the speeds and accelerations
	_maxRunningSpeed = _maxWalkingSpeed = _maxWalkingSlowSpeed = _maxCrawlingSpeed = 0.0f;
	_runningAccel = _walkingAccel = _walkingSlowAccel = _crawlingAccel = 0.0f;
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
			_animations[_currentAnimationState]->Render(screen, _currentHeading, Position.x-screen->Origin.x, Position.y-screen->Origin.y, true, &_highlightColor, _camoIdx);
		} else {
			Color yellow(255,255,0);
			_animations[_currentAnimationState]->Render(screen, _currentHeading, Position.x-screen->Origin.x, Position.y-screen->Origin.y, IsSelected(), &yellow, _camoIdx);
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

	if(_currentState.IsSet(SoldierState::Dead))
	{
		// No need to simulate anything, cuz we dead
		// Just clear out our orders
		return;
	} else if(_currentState.IsSet(SoldierState::DyingBlownUp)
		|| _currentState.IsSet(SoldierState::DyingForward)
		|| _currentState.IsSet(SoldierState::DyingBackward))
	{
		// Just update our animation
		_animations[_currentAnimationState]->Update(dt);
		_currentFrameCurrentState = _animations[_currentAnimationState]->GetCurrentFrameNumber(_currentHeading);
	
		// Am I dead yet?
		if(_currentFrameCurrentState == _animations[_currentHeading]->GetCurrentFrameNumber(_currentHeading)
			&& !_currentAnimationMarker)
		{
			// I am dead now
			if(_currentState.IsSet(SoldierState::DyingBackward))
			{
				_currentHeading = (Direction) ((_currentHeading+4) % 8);
				_currentState.UnSet(SoldierState::DyingBackward);
			}
			else if(_currentState.IsSet(SoldierState::DyingForward))
			{
				_currentState.UnSet(SoldierState::DyingForward);
			}
			else if(_currentState.IsSet(SoldierState::DyingBlownUp))
			{
				_currentState.UnSet(SoldierState::DyingBlownUp);
			}
			_currentState.Set(SoldierState::Dead);
			_currentAnimationState = AnimationState::Dead;
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
				_currentAction = Unit::Moving;
				handled = HandleMoveOrder(dt, (MoveOrder *) order);
				break;
			case Orders::Sneak:
				_currentAction = Unit::Crawling;
				handled = HandleSneakOrder(dt, (MoveOrder *) order);
				break;
			case Orders::MoveFast:
				_currentAction = Unit::MovingFast;
				handled = HandleMoveFastOrder(dt, (MoveOrder *) order);
				break;
			case Orders::Destination:
				handled = HandleDestinationOrder((MoveOrder *)order);
				break;
			case Orders::Stop:
				handled = HandleStopOrder((StopOrder *)order);
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

	// Now update the animations. This needs to be done before
	// we handle our actions so that any animations that we are worried
	// about looping are completed before rendered
	_animations[_currentAnimationState]->Update(dt);
	_currentFrameCurrentState = _animations[_currentAnimationState]->GetCurrentFrameNumber(_currentHeading);

	// Simulate my weapon
	_weapons[_currentWeaponIdx]->Simulate(dt);

	// Perform any actions that need to be performed
	Action *action = _actionQueue.Peek();
	if(action != NULL)
	{
		if(SoldierActionHandlers::Handle(this, action, dt))
		{
			action = _actionQueue.Dequeue();
			delete action;
		}
	}

	PlanMovement(dt);
	
	// Update any effects
	for(int i = 0; i < _effects.Count; ++i) {
		_effects.Items[i]->Simulate(dt);
		if(_effects.Items[i]->IsCompleted()) {
			delete _effects.RemoveAt(i);
			--i;
		}
	}
}

void
Soldier::SetPosition(int x, int y)
{
	Point oldPosition = Position;
	Object::SetPosition(x,y);	
	g_Globals->World.CurrentWorld->MoveObject(this, &oldPosition, &Position);
	_position.x = (float) x;
	_position.y = (float) y;
}

bool
Soldier::Select(int x, int y)
{
	bool rv = Contains(x, y);
	if(rv) {
		// Select me
		Object::Select(true);
	}
	return rv;
}

bool
Soldier::Contains(int x, int y)
{
	Region r;
	_animations[_currentAnimationState]->GetExtents(_currentHeading, Position.x, Position.y, &r);
	return Screen::PointInRegion(x, y, &r);
}

bool
Soldier::HandleDestinationOrder(MoveOrder *order)
{
	Vector2 range;
	range.x = (float)(Position.x-order->X);
	range.y = (float)(Position.y-order->Y);

	if(range.Magnitude() < 5.01f) 
	{
		AddOrder(new StopOrder());


		return true;
	}
	return false;
}

bool 
Soldier::HandleStopOrder(StopOrder *order)
{
	// Let's choose an action to implement this order
	Action *action = new Action();
	action->Index = SoldierAction::Stop;
	action->Data = NULL;
	
	// Clear our orders and our actions
	// XXX/GWS: This needs to clean up memory!!!
	_actionQueue.Clear();
	
	// Add our action
	_actionQueue.Enqueue(action);
	
	return true;
}

bool
Soldier::HandleMoveOrder(long dt, MoveOrder *order)
{
	UNREFERENCED_PARAMETER(dt);

	// Let's choose an action to implement this order
	Action *action = new Action();
	action->Index = SoldierAction::Walk;
	Point *p = new Point();
	p->x = order->X;
	p->y = order->Y;
	action->Data = p;
	
	// Let's first stop
	HandleStopOrder(NULL);

	// Add our action
	_actionQueue.Enqueue(action);
	
	return true;
}

bool
Soldier::HandleMoveFastOrder(long dt, MoveOrder *order)
{
	UNREFERENCED_PARAMETER(dt);

	// Let's choose an action to implement this order
	Action *action = new Action();
	action->Index = SoldierAction::Run;
	Point *p = new Point();
	p->x = order->X;
	p->y = order->Y;
	action->Data = p;
	
	// Let's first stop
	HandleStopOrder(NULL);
	
	// Add our action
	_actionQueue.Enqueue(action);
	
	return true;
}

bool
Soldier::HandleSneakOrder(long dt, MoveOrder *order)
{
	UNREFERENCED_PARAMETER(dt);

	// Let's choose an action to implement this order
	Action *action = new Action();
	action->Index = SoldierAction::Crawl;
	Point *p = new Point();
	p->x = order->X;
	p->y = order->Y;
	action->Data = p;
	
	// Let's first stop
	HandleStopOrder(NULL);
	
	// Add our action
	_actionQueue.Enqueue(action);
	
	return true;
}

bool
Soldier::HandleFireOrder(FireOrder *order)
{
	// Let's choose an action to implement this order
	// I can implement a fire order by either a standing
	// fire order or a prone fire order.
	// XXX/GWS: Need to implement action to order matching,
	//			eg figuring out which actions to use to implement
	//			and order
	Action *action = new Action();
	action->Index = SoldierAction::ProneFire;
	SoldierActionHandlers::FireActionData *data = new SoldierActionHandlers::FireActionData();
	data->Target = order->Target;
	data->TargetType = order->TargetType;
	data->X = order->X;
	data->Y = order->Y;
	action->Data = data;

	// Let's first stop
	HandleStopOrder(NULL);

	// Add our action
	_actionQueue.Enqueue(action);

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
				// XXX/GWS: I don't need to do that
				//InsertOrder(new PauseOrder(DIRECTION_CHANGE_PAUSE, _currentState, State::Standing), 0);
				_velocity.x = 0.0f;
				_velocity.y = 0.0f;
			} else {
				// Now test move it
				Vector2 vel, pos;
				Move(&pos, &vel, _currentHeading, dt);

				// Go ahead and move this fucker
				_position.x = pos.x;
				_position.y = pos.y;
				_velocity.x = vel.x;
				_velocity.y = vel.y;
				
				Point oldPosition = Position;
				Position.x = (int) _position.x;
				Position.y = (int) _position.y;
				g_Globals->World.CurrentWorld->MoveObject(this, &oldPosition, &Position);
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
					// XXX/GWS: Don't need to do this
					//InsertOrder(new PauseOrder(DIRECTION_CHANGE_PAUSE, _currentState, State::Standing), 0);
					_velocity.x = 0.0f;
					_velocity.y = 0.0f;
				} else {
					// Now test move it
					Vector2 vel, pos;
					Move(&pos, &vel, _currentHeading, dt);

					// Go ahead and move this fucker
					_position.x = pos.x;
					_position.y = pos.y;
					_velocity.x = vel.x;
					_velocity.y = vel.y;
					Point oldPosition = Position;
					Position.x = (int) _position.x;
					Position.y = (int) _position.y;
					g_Globals->World.CurrentWorld->MoveObject(this, &oldPosition, &Position);
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
						// XXX/GWS: Don't do that
						//InsertOrder(new PauseOrder(DIRECTION_CHANGE_PAUSE, _currentState, SoldierState::Standing), 0);
						return;
					}
				}
				
				// Try moving to this location.
				Vector2 vel, pos;
				Move(&pos, &vel, heading, dt);

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
							// XXX/GWS: Don't need to do this
							//InsertOrder(new PauseOrder(DIRECTION_CHANGE_PAUSE, _currentState, SoldierState::Standing), 0);
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
				Point oldPosition = Position;
				Position.x = (int) _position.x;
				Position.y = (int) _position.y;
				g_Globals->World.CurrentWorld->MoveObject(this, &oldPosition, &Position);
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
				//HandleStopOrder(NULL);
				return false;
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
Soldier::Move(Vector2 *posOut, Vector2 *velOut, Direction heading, long dt)
{
	float accel = 0.0f;
	float maxSpeed = 0.0f;

	if(_currentState.IsSet(SoldierState::Running))
	{
		accel = _runningAccel;
		maxSpeed = _maxRunningSpeed;
	}
	else if(_currentState.IsSet(SoldierState::Walking))
	{
		accel = _walkingAccel;
		maxSpeed = _maxWalkingSpeed;
	}
	else if(_currentState.IsSet(SoldierState::WalkingSlow))
	{
		accel = _walkingSlowAccel;
		maxSpeed = _maxWalkingSlowSpeed;
	}
	else if(_currentState.IsSet(SoldierState::Crawling))
	{
		accel = _crawlingAccel;
		maxSpeed = _maxCrawlingSpeed;
	}

	velOut->x = _velocity.x - accel*dt*sin(_currentHeadingAngles[heading])/1000.0f;
	velOut->y = _velocity.y + accel*dt*cos(_currentHeadingAngles[heading])/1000.0f;

	if(velOut->Magnitude() > maxSpeed) { 
		velOut->Normalize();
		velOut->Multiply(maxSpeed);
	}

	// Now we need to try moving this object to its new position
	posOut->x = _position.x + velOut->x*fabs(sin(_currentHeadingAngles[heading]))*dt*(float)(g_Globals->World.Constants.PixelsPerMeter)/1000.0f;
	posOut->y = _position.y + velOut->y*fabs(cos(_currentHeadingAngles[heading]))*dt*(float)(g_Globals->World.Constants.PixelsPerMeter)/1000.0f;
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
	state->SquadStates[teamIdx].UnitStates[unitIdx].CurrentAction = _currentAction;
	state->SquadStates[teamIdx].UnitStates[unitIdx].CurrentStatus = _currentStatus;
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].WeaponIcon, _weapons[_currentWeaponIdx]->GetIconName());
	state->SquadStates[teamIdx].UnitStates[unitIdx].NumRounds = _weapons[_currentWeaponIdx]->GetCurrentRounds() + _weapons[_currentWeaponIdx]->GetRoundsPerClip()*_weaponsNumClips[_currentWeaponIdx];
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].Title, _title);
	strcpy(state->SquadStates[teamIdx].UnitStates[unitIdx].Rank, _rank);
}

void
Soldier::Kill()
{
	// Stop the soldier
	HandleStopOrder(NULL);

	// Kills this soldier
	switch(rand()%3)
	{
	case 0:
		_currentState.Set(SoldierState::DyingBackward);
		_currentAnimationState = AnimationState::DyingBackward;
		break;
	case 1:
		_currentState.Set(SoldierState::DyingForward);
		_currentAnimationState = AnimationState::DyingForward;
		break;
	case 2:
		_currentState.Set(SoldierState::DyingBlownUp);
		_currentAnimationState = AnimationState::DyingBlownUp;
		break;
	default:
		break;
	}

	// Reset the dying animation
	_animations[_currentAnimationState]->Reset();

	// Remember the current animation frame
	_currentFrameCurrentState = _animations[_currentAnimationState]->GetCurrentFrameNumber(_currentHeading);
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
				if(s->_currentState.IsSet(SoldierState::Dead)
					|| s->_currentState.IsSet(SoldierState::DyingBlownUp)
					|| s->_currentState.IsSet(SoldierState::DyingForward)
					|| s->_currentState.IsSet(SoldierState::DyingBackward))
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
				_currentAction = Unit::NoTarget;
				_currentState.UnSet(SoldierState::Firing);
			}
			return;
		case Target::Area:
			// Set my heading
			_currentHeading = Utilities::FindHeading(Position.x, Position.y, targetX, targetY);
			break;
	}
	weapon->Fire();
	_currentState.Set(SoldierState::Firing);
	_currentAction = Unit::Firing;
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
	return _currentState.IsSet(SoldierState::Dead) || _currentState.IsSet(SoldierState::DyingBlownUp) || _currentState.IsSet(SoldierState::DyingBackward) || _currentState.IsSet(SoldierState::DyingForward);
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
