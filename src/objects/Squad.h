#pragma once
#include <misc\array.h>
#include <objects\object.h>
#include <objects\soldier.h>
#include <objects\vehicle.h>
#include <ai\AStar.h>
#include <graphics\Mark.h>

class Screen;
class Order;
class World;

class Squad :
	public Object
{
public:
	Squad(void);
	virtual ~Squad(void);

	enum Quality {
		Useless=0,
		Fragile,
		Weak,
		Average,
		Good,
		Strong,
		NumQuality // Must be last
	};

	// From Object class
	virtual void Render(Screen *screen, Rect *clip);
	virtual void Simulate(long dt, World *world);
	virtual bool Select(int x, int y);
	virtual bool IsSelected();
	virtual void SetPosition(int x, int y);
	virtual void Select(bool s);
	virtual void AddOrder(Order *o);
	virtual void ClearOrders();
	virtual bool IsMobile() { return true; }
	virtual void UpdateInterfaceState(InterfaceState *state, int teamIdx, int unitIdx);
	virtual bool Contains(int x, int y);
	virtual void Highlight(Color *color);
	virtual void UnHighlight();

	// Gets the squad leader for this squad
	Object *GetSquadLeader();

	// Gets the soldiers in this squad
	inline Array<Soldier> *GetSoldiers() { return &_soldiers; }

	// Gets the description of the team strength
	char *GetQualityDesc();

	// Kills this squad
	virtual void Kill();

	// Queries whether this squad is active or not
	virtual bool IsActive();

	// Gets my current path
	inline Path *GetCurrentPath() { return _currentPath; }

	// Clears and set marks
	void ClearMarks() { _bShowMark = false; }

protected:
	// This is the array of soldiers in this squad
	Array<Soldier> _soldiers;

	// The array of vehicles
	Array<Vehicle> _vehicles;

	// The strength of this squad
	Quality _quality;

	// The current path this squad is on
	Path *_currentPath;

	// The individual soldier selected in this squad
	int _selectedSoldierIdx;
	int _selectedVehicleIdx;

	// A flag to determine if we should show our mark or not
	bool _bShowMark;
	Mark::Color _markColor;
	bool _bMarkTargetPosition;

	// The current action of this guy
	Team::Action _currentAction;

	// The current statuc of this guy
	Team::Status _currentStatus;

	friend class SquadManager;
};
