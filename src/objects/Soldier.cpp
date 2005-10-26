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
	_formationPosition = 0;

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
			case Orders::Sneak:
			case Orders::MoveFast:
				// This should never happen, because we handle this stuff
				// through actions now
				assert(0);
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
	UNREFERENCED_PARAMETER(order);

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

void
Soldier::FollowPath(Path *path, SoldierAction::Action movementStyle)
{
	// Let's first stop
	HandleStopOrder(NULL);

	while(path != NULL)
	{
		Action *action = new Action();
		action->Index = movementStyle;
		SoldierActionHandlers::TileData *data = new SoldierActionHandlers::TileData();
		data->TileI = path->X;
		data->TileJ = path->Y;
		action->Data = data;
		path = path->Next;

		// Add our action to the queue
		_actionQueue.Insert(action, _actionQueue.Count());
	}

	// Add an action for our destination reached
	Action *action = new Action(SoldierAction::DestinationReached, NULL);
	_actionQueue.Insert(action, _actionQueue.Count());
	action = new Action(SoldierAction::Stop, NULL);
	_actionQueue.Insert(action, _actionQueue.Count());
}

// Tells this soldier to follow the given object
void 
Soldier::Follow(Object *object, Formation::Type formationType, float formationSpread, int formationIdx, SoldierAction::Action movementStyle)
{
	// Let's first stop
	HandleStopOrder(NULL);

	// Add our follow in formation action
	Action *action = new Action(SoldierAction::FollowInFormation, NULL);
	SoldierActionHandlers::FollowFormationData *data = new SoldierActionHandlers::FollowFormationData();
	data->TargetObject = object;
	data->TargetFormation = formationType;
	data->FormationIndex = formationIdx;
	data->FormationSpread = formationSpread;
	data->MovementStyle = movementStyle;
	action->Data = data;
	_actionQueue.Insert(action, _actionQueue.Count());

	// Add an action for our destination reached
	action = new Action(SoldierAction::DestinationReached, NULL);
	_actionQueue.Insert(action, _actionQueue.Count());
	action = new Action(SoldierAction::Stop, NULL);
	_actionQueue.Insert(action, _actionQueue.Count());

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
	data->TargetObject = order->Target;
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

#define MAX_PATH_AVERAGE 3
void
Soldier::GetGeneralHeading(Vector2 *heading)
{
	// XXX/GWS: We are calling this because we are trying to predict
	//			a future position for our formation alignment stuff.
	//			Let's make that function do this. Remember, our path
	//			is now a series of WalkTo/RunTo/CrawlTo actions, so
	//			we need to find a way to predict this path.
	heading->x = 0.0f;
	heading->y = 0.0f;

	// Let's peak at our current action
	Action *action = _actionQueue.Peek();
	if(action != NULL 
		&& (action->Index == SoldierAction::WalkTo 
			|| action->Index == SoldierAction::RunTo
			|| action->Index == SoldierAction::CrawlTo
			|| action->Index == SoldierAction::WalkSlowTo))
	{
		int x=0,y=0;
		SoldierActionHandlers::TileData *data = (SoldierActionHandlers::TileData *)action->Data;
		g_Globals->World.CurrentWorld->ConvertTileToPosition(data->TileI, data->TileJ, &x, &y);
		heading->x = x-Position.x;
		heading->y = y-Position.y;
		heading->Normalize();
	}

#if 0
	// We need to look at our current path and average the next couple
	// of paths we are going to follow
	Path *path = _currentSquad->GetCurrentPath(), *prevPath = NULL;

	// Initialize our return value
	heading->x = 0.0f;
	heading->y = 0.0f;
	
	int nPaths = 0;
	for(int i = 0; i < MAX_PATH_AVERAGE && path != NULL; ++i)
	{
		if(prevPath != NULL)
		{
			Vector2 dir;
			dir.x = path->X-prevPath->X;
			dir.y = path->Y-prevPath->Y;
			dir.Normalize();
			heading->x += dir.x;
			heading->y += dir.y;
			++nPaths;
		}
		prevPath = path;
		path = path->Next;
	}

	// Divide by the number of paths we saw
	if(nPaths > 0)
	{
		heading->x /= (float)nPaths;
		heading->y /= (float)nPaths;

		// Normalize
		// XXX/GWS: Is this necessary? I think we should already be normalized
		heading->Normalize();
	}
#endif
}

bool
Soldier::IsStopped()
{
	return _currentState.IsSet(SoldierState::Stopped);
}

void
Soldier::Ambush(Direction heading)
{
	// Let's first stop whatever we were doing
	HandleStopOrder(NULL);

	// Now setup the action to perform. First turn in the direction
	// we need. Then start ambushing.
	Action *action = new Action(SoldierAction::Turn, (void *)heading);
	_actionQueue.Enqueue(action);
	action = new Action(SoldierAction::Ambush, (void *)heading);
	_actionQueue.Enqueue(action);
}

void
Soldier::Defend(Direction heading)
{
	// Let's first stop whatever we were doing
	HandleStopOrder(NULL);

	// Now setup the action to perform
	Action *action = new Action(SoldierAction::Turn, (void *)heading);
	_actionQueue.Enqueue(action);
	action = new Action(SoldierAction::Defend, (void *)heading);
	_actionQueue.Enqueue(action);
}
