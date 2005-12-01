#include ".\squad.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <world\World.h>
#include <graphics\Mark.h>
#include <graphics\Screen.h>
#include <graphics\Widget.h>
#include <sound\Sound.h>
#include <application\Globals.h>

static char *_squadQualityIcons[] = { "Team Quality Useless", "Team Quality Fragile", "Team Quality Weak", "Team Quality Average", "Team Quality Good", "Team Quality Strong" };

Squad::Squad() : Object()
{
	_currentStatus = Team::Healthy;
	_currentAction = Team::Defending;
	_quality = Average;
	_type = Target::Squad;
	_currentTarget = NULL;
	_selectedSoldierIdx = -1;
	_selectedVehicleIdx = -1;
	_currentPointManIdx = 0;
	_canMove = true;
	_canMoveFast = true;
	_canDefend = true;
	_canSmoke = true;
	_canAmbush = true;
	_canSneak = true;
	_canFire = true;
	_currentPath = NULL;
	_bShowMark = false;
	_bMarkTargetPosition = false;
	_currentFormationSpread = 2.0f;
	_currentFormation = Formation::Column;
}

Squad::~Squad(void)
{
}

void
Squad::Render(Screen *screen, Rect *clip)
{
	// If our squad contains the cursor, then
	// we need to highlight this
	if(Contains(screen->GetCursorX() + screen->Origin.x, screen->GetCursorY() + screen->Origin.y))
	{
		Color white(255,255,255);
		Highlight(&white);
	} else {
		UnHighlight();
	}

	// Render our current path as a series of filled blocks
	if(g_Globals->World.bRenderPaths && _currentPath != NULL) {
		Path *p = _currentPath;
		while(p != NULL) {
			Color red(128,0,0);

			// We need to clip the path rectangles
			if(screen->PointInRegion(p->X*g_Globals->World.CurrentWorld->TileSize.w-screen->Origin.x, 
				p->Y*g_Globals->World.CurrentWorld->TileSize.h-screen->Origin.y, 
				clip->x, clip->y, clip->w, clip->h))
			{
				screen->FillRect(p->X*g_Globals->World.CurrentWorld->TileSize.w-screen->Origin.x,
					p->Y*g_Globals->World.CurrentWorld->TileSize.h-screen->Origin.y,
					g_Globals->World.CurrentWorld->TileSize.w,
					g_Globals->World.CurrentWorld->TileSize.h, &red);
			}
			p = p->Next;
		}
	}

	for(int i = 0; i < _vehicles.Count; ++i) {
		_vehicles.Items[i]->Render(screen, clip);
	}

	for(int i = 0; i < _soldiers.Count; ++i) {
		_soldiers.Items[i]->Render(screen, clip);
		if(_selectedSoldierIdx == i && IsSelected()) {
			Color white(255,255,255);
			Widget *w = g_Globals->World.Icons->GetWidget("Unit Selected Bracket");
			w->Render(screen, _soldiers.Items[i]->Position.x - screen->Origin.x, _soldiers.Items[i]->Position.y-screen->Origin.y, &white);
			delete w;
		}
	}

	if(IsSelected() && IsActive() && _bShowMark)
	{
		if(_bMarkTargetPosition) {
			g_Globals->World.CurrentWorld->AddMark(_markColor, _currentTarget->Position.x, _currentTarget->Position.y);
		} else {
			g_Globals->World.CurrentWorld->AddMark(_markColor, _currentTargetX, _currentTargetY);
		}
	}
}

void
Squad::Simulate(long dt, World *world)
{
	// Simulate any vehicles
	for(int i = 0; i < _vehicles.Count; ++i) {
		_vehicles.Items[i]->Simulate(dt, world);
	}

	// XXX/GWS: Make sure the squad leader always moves first!!!
	for(int i = 0; i < _soldiers.Count; ++i) {
		_soldiers.Items[i]->Simulate(dt, world);
	}

	// Now update the position of the squad. Use the position of the
	// 'squad leader'
	// XXX/GWS: Need to handle squads of more than one vehicle here!
	Position.x = GetSquadLeader()->Position.x;
	Position.y = GetSquadLeader()->Position.y;
}

bool
Squad::Select(int x, int y)
{
	for(int i = 0; i < _vehicles.Count; ++i) {
		if(_vehicles.Items[i]->Contains(x,y)) {
			_selectedVehicleIdx = i;
			Select(true);
			return true;
		}
	}

	for(int i = 0; i < _soldiers.Count; ++i) {
		if(_soldiers.Items[i]->Contains(x,y)) {
			// Play a sound
			g_Globals->World.Voices->GetSound("awaiting orders")->Play();
			Select(true);
			_selectedSoldierIdx = i;
			return true;
		}
	}
	_selectedVehicleIdx = -1;
	_selectedSoldierIdx = -1;
	return false;
}

bool 
Squad::IsSelected()
{
	for(int i = 0; i < _vehicles.Count; ++i) {
		if(	_vehicles.Items[i]->IsSelected()) {
			return true;
		}
	}

	for(int i = 0; i < _soldiers.Count; ++i) {
		if(_soldiers.Items[i]->IsSelected()) {
			return true;
		}
	}
	return false;
}

void 
Squad::SetPosition(int x, int y)
{
	int sx=0,sy=0;
	Object::SetPosition(x,y);

	// Let's set our soldier positions based on the current formation
	// XXX/GWS: Need smarter formation setting here....cover, etc
	int j = 1;
	for(int i = 0; i < _soldiers.Count; ++i) {
		if(i != _currentPointManIdx)
		{
			Formation::GetFormationPosition(_currentFormation, j++, _currentFormationSpread, &Position, 
				_soldiers.Items[_currentPointManIdx]->GetHeading(), &sx, &sy);
			_soldiers.Items[i]->SetPosition(sx, sy);
		}
		else
		{
			Formation::GetFormationPosition(_currentFormation, 0, _currentFormationSpread, &Position, 
				_soldiers.Items[_currentPointManIdx]->GetHeading(), &sx, &sy);
			_soldiers.Items[i]->SetPosition(sx, sy);
		}
	}

	for(int i = 0; i < _vehicles.Count; ++i) {
		_vehicles.Items[i]->SetPosition(x+i*20, y);
	}
}

void 
Squad::Select(bool s)
{
	for(int i = 0; i < _vehicles.Count; ++i) {
		_vehicles.Items[i]->Select(s);
	}

	for(int i = 0; i < _soldiers.Count; ++i) {
		((Object *)_soldiers.Items[i])->Select(s);
	}
}

void
Squad::AddOrder(Order *o)
{
	_bShowMark = false;
	switch(o->GetType()) {
		case Orders::Ambush:
			HandleAmbushOrder((AmbushOrder *)o);
			return;
		case Orders::Defend:
			HandleDefendOrder((DefendOrder *)o);
			return;
		case Orders::Fire:
			{
				FireOrder *f = (FireOrder *)o;
				_currentAction = Team::Firing;
				_currentTarget = f->Target;
				_currentTargetType = f->TargetType;
				_currentTargetX = f->X;
				_currentTargetY = f->Y;
				_markColor = Mark::Red;
				_bShowMark = true;
				if(f->TargetType == Target::Area) {
					_bMarkTargetPosition = false;
				} else {
					_bMarkTargetPosition = true;
				}
			}
			break;
		case Orders::Hide:
			_currentAction = Team::Hiding;
			break;
		case Orders::Move:
			HandleMoveOrder((MoveOrder *)o, SoldierAction::WalkTo, Mark::Blue);
			return;
		case Orders::MoveFast:
			HandleMoveOrder((MoveOrder *)o, SoldierAction::RunTo, Mark::Purple);
			return;
		case Orders::Sneak:
			HandleMoveOrder((MoveOrder *)o, SoldierAction::CrawlTo, Mark::Yellow);
			return;
	}

	for(int i = 0; i < _vehicles.Count; ++i) {
		_vehicles.Items[i]->AddOrder(o);
	}

	for(int i = 0; i < _soldiers.Count; ++i) {
		_soldiers.Items[i]->AddOrder(o);
	}
}

void
Squad::HandleMoveOrder(MoveOrder *order, SoldierAction::Action movementStyle, Mark::Color color)
{
	int i=0,j=0,di=0,dj=0;

	_currentAction = Team::Moving;
	_currentTargetX = order->X;
	_currentTargetY = order->Y;

	// Find a path
	FreePath(_currentPath, true);
	g_Globals->World.CurrentWorld->ConvertPositionToTile(Position.x, Position.y, &i, &j);
	g_Globals->World.CurrentWorld->ConvertPositionToTile(_currentTargetX, _currentTargetY, &di, &dj);
	Element::Level level = Element::Medium;
	switch(movementStyle)
	{
	case SoldierAction::Crawl:
	case SoldierAction::CrawlTo:
		level = Element::Prone;
		break;

	case SoldierAction::Run:
	case SoldierAction::RunTo:
		level = Element::High;
		break;

	case SoldierAction::WalkSlow:
	case SoldierAction::WalkSlowTo:
		level = Element::Low;
		break;

	case SoldierAction::Walk:
	case SoldierAction::WalkTo:
	default:
		level = Element::Medium;
		break;
	}

	_currentPath = g_Globals->World.Pathing.FindPath(i, j, di, dj, level);
	if(NULL == _currentPath) {
		g_Globals->World.Voices->GetSound("no clear path")->Play();
		return;
	}

	// Set our mark colors
	_markColor = color;
	_bShowMark = true;
	_bMarkTargetPosition = false;

	// Now we need to get our soldiers moving. We order the point man
	// to walk/run/crawl to the location, and we order everyone else
	// to follow him in formation.
	//
	// XXX/GWS: What happens if our point man is dead?
	j = 1;
	for(i = 0; i < _soldiers.Count; ++i) {
		if(i == _currentPointManIdx)
		{
			// Order this soldier to walk/run/crawl to the destination
			_soldiers.Items[i]->FollowPath(_currentPath, movementStyle);
		}
		else if(!_soldiers.Items[i]->IsDead())
		{
			// Order this soldier to follow our point man
			_soldiers.Items[i]->Follow(_soldiers.Items[_currentPointManIdx], _currentFormation, _currentFormationSpread, j++, movementStyle);
		}
	}
}

void
Squad::HandleAmbushOrder(AmbushOrder *order)
{
	_currentAction = Team::Ambushing;
	for(int i = 0; i < _soldiers.Count; ++i)
	{
		_soldiers.Items[i]->Ambush(order->Heading);
	}
}

void
Squad::HandleDefendOrder(DefendOrder *order)
{
	_currentAction = Team::Defending;
		for(int i = 0; i < _soldiers.Count; ++i)
	{
		_soldiers.Items[i]->Defend(order->Heading);
	}
}

void
Squad::ClearOrders()
{
	for(int i = 0; i < _soldiers.Count; ++i) {
		_soldiers.Items[i]->ClearOrders();
	}
	for(int i = 0; i < _vehicles.Count; ++i) {
		_vehicles.Items[0]->ClearOrders();
	}
}

Object *
Squad::GetSquadLeader()
{
	// XXX/GWS: This needs to work with vehicles too
	for(int i = 0; i < _soldiers.Count; ++i) {
		if(_soldiers.Items[i]->IsSquadLeader()) {
			return _soldiers.Items[i];
		}
	}
	for(int i = 0; i < _vehicles.Count; ++i) {
		if(_vehicles.Items[i]->IsSquadLeader()) {
			return _vehicles.Items[i];
		}
	}
	assert(false);
	return NULL;
}

Object *
Squad::GetPointMan()
{
	return _soldiers.Items[_currentPointManIdx];
}

char *
Squad::GetQualityDesc()
{
	return _squadQualityIcons[_quality];
}

void
Squad::UpdateInterfaceState(InterfaceState *state, int teamIdx, int unitIdx)
{
	UNREFERENCED_PARAMETER(unitIdx);
	sprintf(state->SquadStates[teamIdx].Name, "%s", GetName());
	sprintf(state->SquadStates[teamIdx].Icon, "%s", GetIconName());
	state->SquadStates[teamIdx].NumUnits = GetSoldiers()->Count;
	state->SquadStates[teamIdx].ID = GetID();
	state->SquadStates[teamIdx].CurrentAction = _currentAction;
	strcpy(state->SquadStates[teamIdx].Quality, GetQualityDesc());
	state->SquadStates[teamIdx].SelectedSoldierIdx = _selectedSoldierIdx;
	for(int j = 0; j < GetSoldiers()->Count; ++j) {
		GetSoldiers()->Items[j]->UpdateInterfaceState(state, teamIdx, j);
		if(GetSquadLeader()->GetID() == GetSoldiers()->Items[j]->GetID()) {
			state->SquadStates[teamIdx].SquadLeaderIdx = j;
		}
	}
	for(int j = 0; j < _vehicles.Count; ++j) {
		_vehicles.Items[j]->UpdateInterfaceState(state, teamIdx, j);
		if(GetSquadLeader()->GetID() == _vehicles.Items[j]->GetID()) {
			state->SquadStates[teamIdx].SquadLeaderIdx = j;
		}
	}
}

void
Squad::Kill()
{
	// XXX/GWS: This is supposed to kill this squad, but for now,
	// just kill a random soldier in it
	int i = rand() % _soldiers.Count;
	_soldiers.Items[i]->Kill();
}

bool 
Squad::Contains(int x, int y)
{
	for(int i = 0; i < _soldiers.Count; ++i) {
		if(_soldiers.Items[i]->Contains(x, y)) {
			return true;
		}
	}

	for(int i = 0; i < _vehicles.Count; ++i) {
		if(_vehicles.Items[i]->Contains(x, y)) {
			return true;
		}
	}

	return false;
}

bool
Squad::IsActive()
{
	// XXX/GWS: Needs to work with vehicles
	for(int i = 0; i < _soldiers.Count; ++i) {
		if(!_soldiers.Items[i]->IsDead()) {
			return true;
		}
	}
	return false;
}

void 
Squad::Highlight(Color *color)
{
	Object::Highlight(color);
	for(int i = 0; i < _soldiers.Count; ++i) {
		_soldiers.Items[i]->Highlight(color);
	}
	for(int i = 0; i < _vehicles.Count; ++i) {
		_vehicles.Items[i]->Highlight(color);
	}

}

void 
Squad::UnHighlight()
{
	Object::UnHighlight();
	for(int i = 0; i < _soldiers.Count; ++i) {
		_soldiers.Items[i]->UnHighlight();
	}
	for(int i = 0; i < _vehicles.Count; ++i) {
		_vehicles.Items[i]->UnHighlight();
	}
}
