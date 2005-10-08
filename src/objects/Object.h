#pragma once

#include <misc\Array.h>
#include <misc\Queue.h>
#include <misc\Structs.h>
#include <objects\Action.h>
#include <objects\Status.h>
#include <objects\Target.h>
#include <misc\Color.h>

class Screen;
class Order;
class World;
class InterfaceState;
class Effect;

#define HEALTH_MAX 100

class Object
{
public:
	Object();
	virtual ~Object(void);

	// The position of this object
	Point Position;

	// Render this object to the screen
	virtual void Render(Screen *screen, Rect *clip) = 0;

	// Simulate this object for dt milliseconds
	virtual void Simulate(long dt, World *world) = 0;
	
	// Queries whether or not this object is movable
	virtual bool IsMobile() { return false; }
	
	// Set the position of this object to (x,y)
	virtual void SetPosition(int x, int y) { Position.x = x; Position.y = y; }

	// Routines to select and deselect this object
	virtual bool Select(int x, int y);
	virtual void Select(bool s) { _isSelected = s; }
	virtual bool IsSelected() { return _isSelected; }

	// Determines whether or not this object contains the point (x,y)
	virtual bool Contains(int x, int y);

	// Adds an order to the order queue.
	virtual void AddOrder(Order *o);
	virtual void InsertOrder(Order *o, int i);

	// Clears the order queue.
	virtual void ClearOrders();

	// Queries the state of various actions on this object
	inline bool CanMove() { return _canMove; }
	inline bool CanMoveFast() { return _canMoveFast; }
	inline bool CanFire() { return _canFire; }
	inline bool CanDefend() { return _canDefend; }
	inline bool CanSmoke() { return _canSmoke; }
	inline bool CanAmbush() { return _canAmbush; }
	inline bool CanSneak() { return _canSneak; }

	// Gets the minimum bounds of this object, useful for 
	// collision detection
	inline Bounds *GetMinBounds() { return &_minBounds; }

	// Gets the icon name for this object
	inline char *GetIconName() { return _iconName; }

	// Gets the name of this object
	inline char *GetName() { return _name; }

	// Gets the unique identifier for this object
	inline long GetID() { return _id; }

	// Gets the current action of this object
	inline Action::Type GetCurrentAction() { return _currentAction; }

	// Gets the current status of this object
	virtual Unit::Status GetCurrentStatus() { return _currentStatus; }

	// Updates the interface state
	virtual void UpdateInterfaceState(InterfaceState *state, int teamIdx, int unitIdx) {state;teamIdx;unitIdx;}

	// Kills this objects
	virtual void Kill() {}

	// Gets the target type of this object
	virtual Target::Type GetType() { return _type; }

	// Returns the moving flag
	inline bool IsMoving() { return _moving; }

	// Sets the squad leader flag
	inline void SetSquadLeader(bool v) { _bSquadLeader = v; }
	inline bool IsSquadLeader() { return _bSquadLeader; }

	// Highlight this object
	virtual void Highlight(Color *color) { _bHighlight = true; _highlightColor = *color; }
	virtual void UnHighlight() { _bHighlight = false; }
	virtual bool IsHighlighted() { return _bHighlight; }

protected:
	 // A flag which determines whether or not this guy is selected
	bool  _isSelected;

	// The minimum bounds of this object, useful for collision detection
	Bounds _minBounds;

	// Action states
	bool _canFire;
	bool _canMove;
	bool _canMoveFast;
	bool _canSneak;
	bool _canDefend;
	bool _canAmbush;
	bool _canSmoke;

	// The queue which contains the orders for this object
	Queue<Order> _orders;

	// The name of the icon used for this object
	char _iconName[32];

	// The name of this object
	char _name[32];

	// A unique ID for this object
	long _id;

	// The current action of this guy
	Action::Type _currentAction;

	// The current statuc of this guy
	Unit::Status _currentStatus;

	// All effects on this object
	Array<Effect> _effects;

	// The target type of this object
	Target::Type _type;

	// The current health of this object, from 0-100
	int _health;

	// An array of my current target. We store them by target type
	Object *_currentTarget;
	Target::Type _currentTargetType;
	int _currentTargetX, _currentTargetY; // For area targets

	// A flag to tell if this object is moving or not
	bool _moving;

	// A flag which says whether or not I am a squad leader
	bool _bSquadLeader;

	// A flag which says whether or not we should highlight this object
	bool _bHighlight;

	// The highliting color
	Color _highlightColor;
};
