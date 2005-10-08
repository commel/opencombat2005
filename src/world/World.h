#pragma once

struct Rect;
class AnimationManager;
class ColorManager;
class Effect;
class EffectManager;
class ElementManager;
class LineOfSight;
class Object;
class Order;
class Screen;
class SoldierManager;
class SquadManager;
class VehicleManager;
class WeaponManager;

#include <misc\Array.h>
#include <misc\Color.h>
#include <graphics\CombatContextMenu.h>
#include <graphics\Mark.h>
#include <world\Map.h>
#include <objects\Action.h>
#include <objects\Status.h>

// This class is how the world communicates its state
// about the objects in it to the user interface
// above it
#define MAX_UNITS		16
#define MAX_SQUADS		32

class UnitState 
{
public:
	char Name[32];
	char State[32];
	char Icon[32];
	char WeaponIcon[32];
	char Title[32];
	char Rank[32];
	int NumRounds;
	long ID;
	Action::Type CurrentAction;
	Unit::Status CurrentStatus;
};

class SquadState
{
public:
	char Name[32];
	char Icon[32];
	char Quality[32];
	UnitState UnitStates[MAX_UNITS];
	int NumUnits;
	long ID;
	int SquadLeaderIdx;
	int SelectedSoldierIdx;
	Action::Type CurrentAction;
	Team::Status CurrentStatus;
};

class InterfaceState
{
public:
	SquadState SquadStates[MAX_SQUADS];
	int NumSquads;
	int SelectedSquad;
};

class World
{
public:
	World(void);
	virtual ~World(void);

	// The states of this world
	enum WorldState {
		Normal,			// Normal rendering operations
		ContextSelecting,	// Currently selecting an item on the context menu
		ContextSelected, // An item on the context menu was selected and we are performing and action
		Ambushing, // Choosing an ambush direction
		Defending  // Choosing a defend direction
	};

	// Renders this world to the screen, given a clipping rectangle
	virtual void Render(Screen *screen, Rect *clip);

	// Loads a world from a file
	virtual void Load(char *fileName, SoldierManager *soldierManager, AnimationManager *animationManager);

	// Adds an object to this world
	virtual void AddObject(Object *o);

	// Simulate this world for an interval of time
	virtual void Simulate(long dt);

	// Trys to move an object to a new position in the world
	virtual bool TryMove(Object *o, int x, int y);

	// Mouse actions
	virtual void LeftMouseDown(int x, int y);
	virtual void LeftMouseUp(int x, int y);
	virtual void LeftMouseDrag(int x, int y);
	virtual void RightMouseDown(int x, int y);
	virtual void RightMouseUp(int x, int y);
	virtual void RightMouseDrag(int x, int y);

	// Key events - XXX/GWS: This is only used for debugging the game I believe
	virtual void KeyUp(int key);

	// Issue orders to all selected objects
	virtual void IssueOrder(Order *o);

	// Gets the width and height of the world
	inline int GetWidth() { return _currentMap->GetWidth(); }
	inline int GetHeight() { return _currentMap->GetHeight(); }

	// Set's the origin of this world
	inline void SetOrigin(int x, int y) { _originX = x; _originY = y; }

	// Gets the list of mobile objects in this world
	inline Array<Object> *GetObjects() { return &_mobileObjects; }

	// The state of objects in this world
	InterfaceState State;

	// Gets the name of the mini map for this world
	char *GetMiniMapName() { return _currentMap->GetMiniName(); }

	// Gets the name of the overland map
	char *GetOverlandName() { return _currentMap->GetOverlandName(); }

	// Checks whether or not the tile (i,j) is passable
	bool IsPassable(int i, int j) { return !_currentMap->IsTileBlockHeight(i,j); }
	Point NumTiles;
	Size TileSize;

	// Converts a tile (i,j) location to a world position (x,y)
	void ConvertTileToPosition(int i, int j, int *x, int *y);
	void ConvertPositionToTile(int x, int y, int *i, int *j);

	// Add a mark to a given position
	void AddMark(Mark::Color markColor, int x, int y);
	// Clear all our marks
	void ClearMarks();

protected:

	// Clears all of the current selections
	void ClearSelect(int x, int y);

	// Trys to select an object at (x,y)
	void Select(int x, int y);

	// Updates the State variable
	void UpdateState();

	// The current map for this world
	Map *_currentMap;

	// The current state of this world
	WorldState _currentState;

	// A list of mobile objects.
	Array<Object> _mobileObjects;

	// A list of static objects
	Array<Object> _staticObjects;

	// The list of selected objects
	Array<Object> _selectedObjects;

	// A list of effects in the world
	Array<Effect> _effects;

	// Soldier and animation manager from the combat module
	SoldierManager *_soldierManager;
	AnimationManager *_animationManager;
	SquadManager *_squadManager;
	EffectManager *_effectManager;
	WeaponManager *_weaponManager;
	ElementManager *_elementManager;
	VehicleManager *_vehicleManager;

	// A system color manager
	ColorManager *_colorManager;

	// A context menu for this world
	CombatContextMenu *_contextMenu;

	// The current choice of the context menu
	CombatContextMenu::ContextMenuChoice _currentChoice;

	// The source position for the 'ranger' line
	int _rangerX, _rangerY;
	
	// The current selection at the end of the ranger line
	Object *_rangerSelectedObject;

	// The color of the ranger line
	Color _rangerColor;

	// The origin of the world
	int _originX, _originY;

	// Lists of marks that we have to render
	Point *_markPoints;
	Mark::Color *_markColors;
	int _numMarks;
	int _maxMarks;

	// A line of sight calculator
	LineOfSight *_lineOfSight;
};
