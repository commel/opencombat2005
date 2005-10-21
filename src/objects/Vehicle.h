#pragma once
#include <Objects\object.h>
#include <Misc\Structs.h>
#include <Orders\Orders.h>

class Screen;
class Soldier;
class Squad;
class Squad;
class TGA;
class Weapon;
class World;

#define MAX_WEAPONS_PER_VEHICLE	8
#define MAX_CREW	8

class Vehicle :
	public Object
{
public:
	Vehicle(void);
	virtual ~Vehicle(void);

	// From class Object
	virtual void SetPosition(int x, int y);
	virtual bool IsMobile() { return true; }
	virtual bool Select(int x, int y);
	virtual void Select(bool s) { Object::Select(s); }
	virtual bool Contains(int x, int y);
	virtual void UpdateInterfaceState(InterfaceState *state, int teamIdx, int unitIdx);

	// Render this object to the screen
	virtual void Render(Screen *screen, Rect *clip);

	// Simulate this object for dt milliseconds
	virtual void Simulate(long dt, World *world);

	// Clones this object
	Vehicle *Clone();

	bool IsDestroyed() { return false; }
	bool IsAbandoned() { return false; }

	void SetSquad(Squad *s) { _currentSquad = s; }

	Weapon *GetWeapon(int slot) { return _weapons[slot]; }
	int GetWeaponNumClips(int slot) { return _weaponsNumClips[slot]; }

	// Adds a crew member to this vehicle
	void AddCrew(Soldier *solder, int slot);

protected:
	friend class VehicleManager;

	enum State {
		Stopped=0, Moving, Firing, NumStates
	};

	// Adds a weapon to this vehicle
	void AddWeapon(Weapon *weapon, int slot, int numClips, bool hull);

	// Movement handling routines
	bool HandleMoveOrder(long dt, MoveOrder *order, State newState);
	bool HandleFireOrder(FireOrder *order);
	bool HandleDestinationOrder(MoveOrder *order);
	bool HandleStopOrder();

	// Shoots my weapons
	void Shoot(Weapon *weapon, Object *target, Target::Type targetType, int targetX, int targetY);

	// Aims the turret
	void AimTurret(int x, int y);

	// Finds my next target
	Soldier *FindTarget(Squad *squad);

	// Plans our movement
	void PlanMovement(long dt);

	// The current state of this vehicle
	State _currentState;

	// The velocity of this vehicle
	Vector2 _velocity;

	// A floating point representation of the position
	Vector2 _position;

	// The maximum road speed of this vehicle
	float _maxRoadSpeed;

	// The acceleration of the vehicle
	float _acceleration;

	// If we have a destination, then this is the end result.
	// We also have a short range destination, which is a small
	// destination on route to our larger one
	Point _destination;
	Point _shortDestination;

	// The name of this vehicle
	char _name[64];

	// The hull graphics for this vehic
	TGA *_hullGraphics;

	// The turret graphics
	TGA *_turretGraphics;

	// The wreck graphics
	TGA *_wreckGraphics;

	// The location of the turret on the hull
	Point _turretPosition;

	// The location of the muzzle flash on the main gun, if there is one
	Point _muzzlePosition;

	// The angle of the turret
	float _currentTurretAngle;

	// The current heading of this vehicle
	float _currentHullAngle;

	// The squad I am assigned to
	Squad *_currentSquad;

	// The rotation rate of the turret and the hull. This rate is the
	// time in milliseconds it takes to rotate 22.5 degrees (1/16th of a full arc)
	int _turretRotationRate;
	int _hullRotationRate;
	bool _turretRotating;
	bool _hullRotating;
	float _turretTargetAngle;
	float _hullTargetAngle;
	float _turretRotationDirection;
	float _hullRotationDirection;

	// The weapons on this vehicle
	Weapon *_weapons[MAX_WEAPONS_PER_VEHICLE];
	int _weaponsNumClips[MAX_WEAPONS_PER_VEHICLE];
	int _numWeapons;

	// Keeps track of whether the weapons are on the hull
	// or on the turret (true is on hull)
	bool _weaponIsOnHull[MAX_WEAPONS_PER_VEHICLE];

	// The crew on this vehicle
	struct CrewSlot {
		Soldier *soldier;
		int weaponSlot;
	};
	CrewSlot _crew[MAX_CREW];
	int _numCrew;

	// The current action of this guy. Used only for reporting.
	Unit::Action _currentAction;

	// The current statuc of this guy. Used only for reporting.
	Unit::Status _currentStatus;

};
