#include ".\world.h"
#include <assert.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <application\CursorInterface.h>
#include <graphics\Effect.h>
#include <graphics\Mark.h>
#include <world\Map.h>
#include <objects\Object.h>
#include <orders\Orders.h>
#include <graphics\Screen.h>
#include <objects\Squad.h>
#include <misc\Structs.h>
#include <graphics\AnimationManager.h>
#include <graphics\ColorManager.h>
#include <world\ElementManager.h>
#include <world\LineOfSight.h>
#include <graphics\Widget.h>
#include <misc\Utilities.h>
#include <objects\Vehicle.h>
#include <application\Globals.h>

World::World(void)
{
	_contextMenu = new CombatContextMenu();
	State.NumSquads = 0;
	State.SelectedSquad = -1;
	_maxMarks = 8;
	_numMarks = 0;
	_markPoints = (Point*)calloc(_maxMarks, sizeof(Point));
	_markColors = (Mark::Color*) calloc(_maxMarks, sizeof(Mark::Color));
}

World::~World(void)
{
}

void
World::Render(Screen *screen, Rect *clip)
{
	Color white(255,255,255);
	Color black(0,0,0);

	screen->SetOrigin(_originX, _originY);

	// Clear all of our marks
	ClearMarks();

	// Render the map against the clipping rectangle
	_currentMap->SetOrigin(_originX, _originY);
	_currentMap->Render(screen, clip);

	// Render all of the static objects that are in the clipping region
	for(int i = 0; i < _staticObjects.Count; ++i) {
		Object *o = _staticObjects.Items[i];
		if(o->Position.x > (clip->x+_originX) && o->Position.x < (clip->x+_originX+clip->w)
			&& o->Position.y > (clip->y+_originY) && o->Position.y < (clip->y+_originY+clip->h))
		{
			o->Render(screen, clip);
		}
	}

	// Render all of the mobile objects that are in the clipping region
	_rangerSelectedObject = NULL;
	for(int i = 0; i < _mobileObjects.Count; ++i) {
		Object *o = _mobileObjects.Items[i];
		if(o->Position.x > (clip->x+_originX) && o->Position.x < (clip->x+_originX+clip->w)
			&& o->Position.y > (clip->y+_originY) && o->Position.y < (clip->y+_originY+clip->h))
		{
			// Is our ranger over another squad? If it is, then let's highlight that
			// squad
			if(_currentState == ContextSelected 
				&& o->Contains(screen->GetCursorX()+_originX, screen->GetCursorY()+_originY))
			{
				bool oldSelect = o->IsSelected();
				o->Select(true);
				o->Render(screen, clip);
				o->Select(oldSelect);
				_rangerSelectedObject = o;
			} else {
				o->Render(screen, clip);
			}
		}
	}

	// Render all of the elements
	if(g_Globals->World.bRenderElements) {
		_currentMap->RenderElements(screen, clip);
	}

	// Render any of the marks
	int x, y;
	for(int i = 0; i < _numMarks; ++i) {
		x = _markPoints[i].x;
		y = _markPoints[i].y;
		if(x > (clip->x+_originX) && x < (clip->x+_originX+clip->w)
			&& y > (clip->y+_originY) && y < (clip->y+_originY+clip->h))
		{
			g_Globals->World.Marks->Render(screen, _markColors[i], x-_originX, y-_originY);
		}
	}

	// Render all of the effects
	for(int i = 0; i < _effects.Count; ++i) {
		Effect *o = _effects.Items[i];
		if(o->Position.x > (clip->x+_originX) && o->Position.x < (clip->x+_originX+clip->w)
			&& o->Position.y > (clip->y+_originY) && o->Position.y < (clip->y+_originY+clip->h))
		{
			o->Render(screen);
		}
	}

	if(g_Globals->World.bWeaponFan && _selectedObjects.Count > 0) {
		// Show the weapon fan. This is really time consuming!!!
		int sx = _selectedObjects.Items[0]->Position.x/10;
		int sy = _selectedObjects.Items[0]->Position.y/10;
		int ox, oy, oz;

		int m=0,n=0;
		for(int j = (clip->y+_originY)/10; j < (clip->y+clip->h+_originY)/10; ++j) 
		{
			m = 0;
			for(int i = (clip->x+_originX)/10; i < (clip->x+clip->w+_originX)/10; ++i) 
			{
				if(!_lineOfSight->CalculateLOSForTile(sx, sy, i, j, &ox, &oy, &oz, _currentMap)) {
					screen->FillRect(m*10, n*10, 10, 10, &black);
				}
				++m;
			}
			++n;
		}
	}

	if(_currentState == Ambushing)
	{
		assert(_selectedObjects.Count > 0);
		
		// Find out which direction we need to show
		Direction dir = Utilities::FindHeading(_selectedObjects.Items[0]->Position.x - _originX, _selectedObjects.Items[0]->Position.y-_originY, screen->GetCursorX(), screen->GetCursorY());
		Widget *w = NULL;
		switch(dir) {
			case North:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle North");
				break;
			case NorthEast:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle NorthEast");
				break;
			case East:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle East");
				break;
			case SouthEast:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle SouthEast");
				break;
			case South:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle South");
				break;
			case SouthWest:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle SouthWest");
				break;
			case West:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle West");
				break;
			case NorthWest:
				w = g_Globals->World.Icons->GetWidget("Ambush Circle NorthWest");
				break;
			default:
				assert(false);
				break;
		}
		assert(w != NULL);
		w->Render(screen, _selectedObjects.Items[0]->Position.x-_originX, _selectedObjects.Items[0]->Position.y-_originY, w->GetWidth(), w->GetHeight(), true);
		delete w;
	}

	if(_currentState == Defending)
	{
		assert(_selectedObjects.Count > 0);
		
		// Find out which direction we need to show
		Direction dir = Utilities::FindHeading(_selectedObjects.Items[0]->Position.x - _originX, _selectedObjects.Items[0]->Position.y-_originY, screen->GetCursorX(), screen->GetCursorY());
		Widget *w = NULL;
		switch(dir) {
			case North:
				w = g_Globals->World.Icons->GetWidget("Defend Circle North");
				break;
			case NorthEast:
				w = g_Globals->World.Icons->GetWidget("Defend Circle NorthEast");
				break;
			case East:
				w = g_Globals->World.Icons->GetWidget("Defend Circle East");
				break;
			case SouthEast:
				w = g_Globals->World.Icons->GetWidget("Defend Circle SouthEast");
				break;
			case South:
				w = g_Globals->World.Icons->GetWidget("Defend Circle South");
				break;
			case SouthWest:
				w = g_Globals->World.Icons->GetWidget("Defend Circle SouthWest");
				break;
			case West:
				w = g_Globals->World.Icons->GetWidget("Defend Circle West");
				break;
			case NorthWest:
				w = g_Globals->World.Icons->GetWidget("Defend Circle NorthWest");
				break;
			default:
				assert(false);
				break;
		}
		assert(w != NULL);
		w->Render(screen, _selectedObjects.Items[0]->Position.x-_originX, _selectedObjects.Items[0]->Position.y-_originY, w->GetWidth(), w->GetHeight(), true);
		delete w;
	}

	if(_contextMenu->IsShowing()) {
		_contextMenu->Render(screen, screen->GetCursorX(), screen->GetCursorY(), clip);
	}

	if(_currentState == ContextSelected) {
		assert(_selectedObjects.Count > 0);
		_rangerX = _selectedObjects.Items[0]->Position.x-_originX;
		_rangerY = _selectedObjects.Items[0]->Position.y-_originY;

		if(_currentChoice == CombatContextMenu::ContextMenuChoice::Fire) {
			int ox, oy, oz;
			int x0 = _selectedObjects.Items[0]->Position.x/10;
			int y0 = _selectedObjects.Items[0]->Position.y/10;
			int x1 = (screen->GetCursorX()+_originX)/10;
			int y1 = (screen->GetCursorY()+_originY)/10;
			Color c;
			if(_lineOfSight->CalculateLOSForTile(x0, y0, x1, y1, &ox, &oy, &oz, _currentMap))
			{
				c.red = 0;
				c.blue = 0;
				c.green = 255;
				screen->DrawLine(_rangerX, _rangerY, screen->GetCursorX(), screen->GetCursorY(), 3, &c);
				char msg[64];
				Vector2 v;
				v.x = (float)(screen->GetCursorX()-_rangerX);
				v.y = (float)(screen->GetCursorY()- _rangerX);
				sprintf(msg, "%dm", (int)v.Magnitude()/g_Globals->World.Constants.PixelsPerMeter);
				g_Globals->World.Fonts->Render(screen, msg, screen->GetCursorX(), screen->GetCursorY()+10, &white);
			} else {
				char msg[64];

				// Find the distance to the blocked element
				Vector2 vb;
				vb.x = (float)((ox*10+5)-x0*10);
				vb.y = (float)((oy*10+5)-y0*10);
				float bdist = vb.Magnitude();
				// Find the total distance
				Vector2 vt;
				vt.x = (float)(x1*10-x0*10);
				vt.y = (float)(y1*10-y0*10);
				float tdist = vt.Magnitude();
				int bx = (int)(_rangerX + (screen->GetCursorX() - _rangerX)*bdist/tdist);
				int by = (int)(_rangerY + (screen->GetCursorY() - _rangerY)*bdist/tdist);

				Vector2 v;
				v.x = (float)(bx-_rangerX);
				v.y = (float)(by-_rangerY);
				sprintf(msg, "%dm", (int)v.Magnitude()/g_Globals->World.Constants.PixelsPerMeter);
				g_Globals->World.Fonts->Render(screen, msg, bx, by+10, &white);
				c.red = 0;
				c.blue = 0;
				c.green = 255;
				screen->DrawLine(_rangerX, _rangerY, bx, by, 3, &c);
				v.x = (float)(screen->GetCursorX()-_rangerX);
				v.y = (float)(screen->GetCursorY()- _rangerX);
				sprintf(msg, "%dm", (int)v.Magnitude()/g_Globals->World.Constants.PixelsPerMeter);
				g_Globals->World.Fonts->Render(screen, msg, screen->GetCursorX(), screen->GetCursorY()+10, &white);
				c.red = 255;
				c.blue = 0;
				c.green = 0;
				screen->DrawLine(bx, by, screen->GetCursorX(), screen->GetCursorY(), 3, &c);
			}
		} else {
			screen->DrawLine(_rangerX, _rangerY, screen->GetCursorX(), screen->GetCursorY(), 3, &_rangerColor);
		}
	}
}

void
World::Simulate(long dt)
{
	// Simulate all of the mobile objects
	for(int i = 0; i < _mobileObjects.Count; ++i) {
		Object *o = _mobileObjects.Items[i];
		o->Simulate(dt, this);
	}

	// Simulate all of the effects
	for(int i = 0; i < _effects.Count; ++i) {
		_effects.Items[i]->Simulate(dt);
		if(_effects.Items[i]->IsCompleted()) {
			delete _effects.RemoveAt(i);
			--i;
		}
	}

	UpdateState();
}

void
World::Load(char *fileName, SoldierManager *soldierManager, AnimationManager *animationManager)
{
	_soldierManager = soldierManager;
	_animationManager = animationManager;
	_currentState = Normal;
	_originX = 0;
	_originY = 0;

	// Initialize the context menu
	char widgetsFile[256];
	sprintf(widgetsFile, "%s\\ContextMenuWidgets.xml", g_Globals->Application.ConfigDirectory);
	_contextMenu->Initialize(widgetsFile);

	// Create the element manager
	sprintf(widgetsFile, "%s\\Elements.xml", g_Globals->Application.ConfigDirectory);
	_elementManager = new ElementManager();
	_elementManager->LoadElements(widgetsFile);
	g_Globals->World.Elements = _elementManager;

	// Create the color manager
	sprintf(widgetsFile, "%s\\Colors.xml", g_Globals->Application.ConfigDirectory);
	_colorManager = new ColorManager();
	_colorManager->Load(widgetsFile);

	// Create the effect manager
	_effectManager = new EffectManager();
	sprintf(widgetsFile, "%s\\Effects.xml", g_Globals->Application.ConfigDirectory);
	_effectManager->LoadEffects(widgetsFile);
	g_Globals->World.Effects = _effectManager;

	// Create the weapon manager
	_weaponManager = new WeaponManager();
	sprintf(widgetsFile, "%s\\Weapons.xml", g_Globals->Application.ConfigDirectory);
	_weaponManager->LoadWeapons(widgetsFile);
	g_Globals->World.Weapons = _weaponManager;

	// Create the vehicle manager
	// The vehicle manager needs to be created after the weapon manager!
	sprintf(widgetsFile, "%s\\Vehicles.xml", g_Globals->Application.ConfigDirectory);
	_vehicleManager = new VehicleManager();
	_vehicleManager->Load(widgetsFile);
	g_Globals->World.Vehicles = _vehicleManager;

	// Create the squad manager
	sprintf(widgetsFile, "%s\\Squads.xml", g_Globals->Application.ConfigDirectory);
	_squadManager = new SquadManager();
	_squadManager->LoadSquads(widgetsFile);
	g_Globals->World.Squads = _squadManager;

	// XXX/GWS: Load the current map. This is hard coded for now
	_currentMap = Map::Create(fileName);
	_currentMap->GetNumTiles(&(NumTiles.x), &(NumTiles.y));
	_currentMap->GetTileSize(&(TileSize.w), &(TileSize.h));

	// Create the line of sight calculator
	_lineOfSight = new LineOfSight();

	// Create two squads and add them
	// XXX/GWS: The following is temporary, just to populate this world
	//			with some stuff
	State.NumSquads = 0;
	for(int i = 0; i < 5; ++i) {
		Squad *s = _squadManager->CreateSquad("BAR Rifle", _soldierManager, _vehicleManager, _animationManager, _weaponManager);
		s->SetPosition((i+1)*200, 100);
		AddObject(s);
	}

	Squad *squad = _squadManager->CreateSquad("Bazooka", _soldierManager, _vehicleManager, _animationManager, _weaponManager);
	squad->SetPosition(100, 200);
	AddObject(squad);

	squad = _squadManager->CreateSquad(".30 Cal MG", _soldierManager, _vehicleManager, _animationManager, _weaponManager);
	squad->SetPosition(200, 200);
	AddObject(squad);
	
	squad = _squadManager->CreateSquad("Panzer IVG", _soldierManager, _vehicleManager, _animationManager, _weaponManager);
	squad->SetPosition(300, 200);
	AddObject(squad);
}

void
World::UpdateState()
{
	State.NumSquads = _mobileObjects.Count;
	for(int i = 0; i < _mobileObjects.Count; ++i) {
		Object *o = _mobileObjects.Items[i];
		o->UpdateInterfaceState(&State, i, 0);
	}

	// Update the state of any selected objects
	State.SelectedSquad = -1;
	if(_selectedObjects.Count > 0) {
		for(int i = 0; i < State.NumSquads; ++i) {
			if(_selectedObjects.Items[0]->GetID() == State.SquadStates[i].ID) {
				State.SelectedSquad = i;
				break;
			}
		}
	}

	// Now check to see if we have any highlighted objects
	for(int i = 0; i < _mobileObjects.Count; ++i) {
		if(_mobileObjects.Items[i]->IsHighlighted()) {
			for(int j = 0; j < State.NumSquads; ++j) {
				if(_mobileObjects.Items[i]->GetID() == State.SquadStates[j].ID) {
					State.SelectedSquad = j;
					break;
				}
			}
			break;
		}
	}
}

void
World::AddObject(Object *o) 
{
	if(o->IsMobile()) {
		_mobileObjects.Add(o);
	} else {
		_staticObjects.Add(o);
	}
}

bool
World::TryMove(Object *o, int x, int y)
{
	UNREFERENCED_PARAMETER(o);
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	return true;
}

void 
World::LeftMouseDown(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void 
World::LeftMouseUp(int x, int y)
{
	if(_currentState == Normal) {
		// Select anything at this point
		Select(x, y);
	
		// Hide the context menu if we were showing it
		_contextMenu->Hide();

		// If nothing was selected, go BOOM!
		if(_selectedObjects.Count <= 0) {
			Effect *e = _effectManager->GetEffect("Explosion 60m");
			e->SetPosition(x+_originX, y+_originY);
			_effects.Add(e);
		}
	} else if(_currentState == ContextSelecting) {
		_currentState = ContextSelected;

		// Now perform whatever action we need to do from the context menu
		_contextMenu->Hide();
		_currentChoice = _contextMenu->Choose(x, y);

		// Now act on this choice
		assert(_selectedObjects.Count > 0);
		_rangerX = _selectedObjects.Items[0]->Position.x-_originX;
		_rangerY = _selectedObjects.Items[0]->Position.y-_originY;
				
		switch(_currentChoice) {
			case CombatContextMenu::ContextMenuChoice::Move:
				_colorManager->CopyColor("RangerMove", &_rangerColor);
				g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::MarkBlue);
				break;
			case CombatContextMenu::ContextMenuChoice::MoveFast:
				_colorManager->CopyColor("RangerMoveFast", &_rangerColor);
				g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::MarkPurple);
				break;
			case CombatContextMenu::ContextMenuChoice::Fire:
				_colorManager->CopyColor("RangerFire", &_rangerColor);
				g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::MarkRed);
				break;
			case CombatContextMenu::ContextMenuChoice::Sneak:
				_colorManager->CopyColor("RangerSneak", &_rangerColor);
				g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::MarkYellow);
				break;
			case CombatContextMenu::ContextMenuChoice::Smoke:
				_colorManager->CopyColor("RangerSmoke", &_rangerColor);
				break;
			case CombatContextMenu::ContextMenuChoice::Defend:
				_currentState = Defending;
				break;
			case CombatContextMenu::ContextMenuChoice::Ambush:
				_currentState = Ambushing;
				break;
			default:
				// Default action is to cancel everything
				_currentState = Normal;
				break;
		}
	} else if(_currentState == ContextSelected) {
		// Show the cursor
		g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::Regular);

		// Our context selection is done, so finish it
		switch(_currentChoice) {
			case CombatContextMenu::ContextMenuChoice::Move:
				IssueOrder(new MoveOrder(x+_originX, y+_originY, Orders::Move));
				break;
			case CombatContextMenu::ContextMenuChoice::MoveFast:
				IssueOrder(new MoveOrder(x+_originX, y+_originY, Orders::MoveFast));
				break;
			case CombatContextMenu::ContextMenuChoice::Fire:
				if(_rangerSelectedObject != NULL) {
					IssueOrder(new FireOrder(_rangerSelectedObject, _rangerSelectedObject->GetType()));
				} else {
					IssueOrder(new FireOrder(x+_originX, y+_originY));
				}
				break;
			case CombatContextMenu::ContextMenuChoice::Sneak:
				IssueOrder(new MoveOrder(x+_originX, y+_originY, Orders::Sneak));
				break;
			case CombatContextMenu::ContextMenuChoice::Smoke:
				break;
			case CombatContextMenu::ContextMenuChoice::Defend:
				break;
			case CombatContextMenu::ContextMenuChoice::Ambush:
				assert(false);
				break;
			default:
				break;
		}
		_currentState = Normal;
	} else if(_currentState == Ambushing) {
		_currentState = Normal;
	} else if(_currentState == Defending) {
		_currentState = Normal;
	}
}

void 
World::LeftMouseDrag(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void 
World::RightMouseDown(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void 
World::RightMouseUp(int x, int y)
{
	if(_currentState == ContextSelected || _currentState == Ambushing || _currentState == Defending) {
		_currentState = Normal;
		g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::Regular);
		return;
	} else if(_currentState == ContextSelecting) {
		_currentState = Normal;
		g_Globals->Application.Cursor->ShowCursor(true, CursorInterface::CursorType::Regular);
		// Hide the context menu if we were showing it
		_contextMenu->Hide();
		return;
	}

	// We need to make sure that this point is inside our selected objects
	if(_selectedObjects.Count <= 0) {
		// We have no selected items, so try to select something
		Select(x,y);
	} else {
		bool bContains = false;
		for(int i = 0; i < _selectedObjects.Count; ++i) {
			Object *o = _selectedObjects.Items[i];
			if(o->Contains(x+_originX, y+_originY)) {
				bContains = true;
			}
		}

		if(!bContains) {
			// We have some selected objects, but our right click was not
			// in any of their extents. So let's de-select everything, and try to re-select
			// whatever is underneath us
			ClearSelect(x, y);
			Select(x, y);
		}
	}

	// Show the context menu
	bool move=false, moveFast=false, sneak=false, fire=false, defend=false, ambush=false, smoke=false;
	bool bShow = false;
	for(int i = 0; i < _selectedObjects.Count; ++i) {
		Object *o = (Object *) _selectedObjects.Items[i];
		bShow = true;
			
		if(o->CanMove()) {
			move = true;
		}
		if(o->CanMoveFast()) {
			moveFast = true;
		}
		if(o->CanFire()) {
			fire = true;
		}
		if(o->CanDefend()) {
			defend = true;
		}
		if(o->CanAmbush()) {
			ambush = true;
		}
		if(o->CanSmoke()) {
			smoke = true;
		}
		if(o->CanSneak()) {
			sneak = true;
		}
	}

	if(bShow) {
		_contextMenu->SetMove(move);
		_contextMenu->SetMoveFast(moveFast);
		_contextMenu->SetAmbush(ambush);
		_contextMenu->SetFire(fire);
		_contextMenu->SetDefend(defend);
		_contextMenu->SetSmoke(smoke);
		_contextMenu->SetSneak(sneak);
		_contextMenu->Show(x,y);
		_currentState = ContextSelecting;
	}
}

void 
World::RightMouseDrag(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

void
World::Select(int x, int y)
{
	// Clear current selections
	ClearSelect(x+_originX,y+_originY);

	// Try selecting mobile objects first
	for(int i = 0; i < _mobileObjects.Count; ++i) {
		if(_mobileObjects.Items[i]->Select(x+_originX,y+_originY)) {
			_selectedObjects.Add(_mobileObjects.Items[i]);
		}
	}
}

void
World::ClearSelect(int x, int y)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	for(int i = _selectedObjects.Count-1; i >= 0; --i) {
		(_selectedObjects.RemoveAt(i))->Select(false);
	}
}

void
World::IssueOrder(Order *order)
{
	for(int i = 0; i < _selectedObjects.Count; ++i) {
		Object *o = _selectedObjects.Items[i];
		o->ClearOrders();
		o->AddOrder(order);
	}
}

void
World::KeyUp(int key)
{
	// Let's kill one of our soldiers
	if(key == 'k' || key == 'K') {
		for(int i = 0; i < _selectedObjects.Count; ++i) {
			_selectedObjects.Items[i]->Kill();
		}
	}
}

void
World::ConvertTileToPosition(int i, int j, int *x, int *y)
{
	int w=0,h=0;
	_currentMap->GetTileSize(&w, &h);
	*x = i*w+w/2;
	*y = j*h+h/2;
}

void 
World::ConvertPositionToTile(int x, int y, int *i, int *j)
{
	int w=0,h=0;
	_currentMap->GetTileSize(&w, &h);
	*i = x/w;
	*j = y/h;
}

// Add a mark to a given position
void 
World::AddMark(Mark::Color markColor, int x, int y)
{
	if(_numMarks >= _maxMarks)
	{
		_maxMarks*=2;
		_markPoints = (Point*)realloc(_markPoints, _maxMarks*sizeof(Point));
		_markColors = (Mark::Color*)realloc(_markColors, _maxMarks*sizeof(Mark::Color));
	}
	_markPoints[_numMarks].x = x;
	_markPoints[_numMarks].y = y;
	_markColors[_numMarks++] = markColor;
}

// Clear all our marks
void 
World::ClearMarks()
{
	_numMarks = 0;
}

