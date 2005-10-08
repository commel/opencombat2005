#pragma once
#include <objects\Object.h>
#include <graphics\Animation.h>
#include <orders\Orders.h>
#include <objects\Target.h>
#include <ai\Path.h>

class Screen;
class Squad;
class World;
class InterfaceState;
class Weapon;

#define MAX_WEAPONS_PER_SOLDIER	8

class Soldier :
	public Object
{
public:
	Soldier(void);
	virtual ~Soldier(void);

	enum State {
		Standing=0, Prone, Walking, Sneaking, Running, StandingFiring, ProneFiring, StandingReloading, ProneReloading, DyingBlownUp, DyingBackward, DyingForward, Dead, NumStates
	};

	// From class Object
	virtual bool IsMobile() { return true; }
	virtual void Render(Screen *screen, Rect *clip);
	virtual void Simulate(long dt, World *world);
	virtual void SetPosition(int x, int y);
	virtual bool Select(int x, int y);
	virtual bool Contains(int x, int y);
	virtual void UpdateInterfaceState(InterfaceState *state, int teamIdx, int unitIdx);

	// Add's this soldier to a squad
	inline void SetSquad(Squad *squad) { _currentSquad = squad; }

	// Gets the personal name of this soldier
	inline char *GetPersonalName() { return _personalName; }

	// Gets the title of this soldier (leader, asst, soldier, etc)
	inline char *GetTitle() { return _title; }

	// Set's the title of this soldier
	void SetTitle(char *title);

	// Sets and gets the rank of this soldier
	inline char *GetRank() { return _rank; }
	void SetRank(char *rank);

	// Sets the camo scheme
	void SetCamouflage(char *camo);

	// Gets the current unit status
	virtual Unit::Status GetCurrentStatus();

	// Kills this soldier
	virtual void Kill();

	// Gets the squad that this guy belongs to
	Squad *GetSquad() { return _currentSquad; }

	// Returns whether or not this soldier is dead
	virtual bool IsDead();

	// Inserts a weapon into the weapon pool
	void InsertWeapon(Weapon *weapon, int numClips);

	bool IsInVehicle() { return _inVehicle; }
	void SetInVechicle(bool v) { _inVehicle = v; }

protected:

	// These functions handle various orders
	bool HandleMoveOrder(long dt, MoveOrder *order, State newState);
	bool HandleDestinationOrder(MoveOrder *order);
	bool HandleStopOrder(StopOrder *order);
	bool HandlePauseOrder(PauseOrder *order, long dt);
	bool HandleFireOrder(FireOrder *order);

	// This function handles the movement logic
	void PlanMovement(long dt);
	
	// Finds my next move to this next point in my path.
	bool FindPath(Path *path);

	// This function outputs the position and velocity of the
	// input vectors. It is used to find out where we would end up
	// if we moved.
	void Move(Vector2 *posOut, Vector2 *velOut, Direction heading, long dt, State state);

	// Let's shoot at a target using the given weapon
	void Shoot(Weapon *weapon, Object *target, Target::Type targetType, int targetX, int targetY);

	// Let's calculate a shot fired at us. Returns true if we die as a result of this
	// shot
	bool CalculateShot(Soldier *shooter, Weapon *weapon);

	// Finds a target within a squad
	Soldier *FindTarget(Squad *squad);

	// Keeps track of the maximum speeds for each state
	float _speeds[NumStates];

	// Keeps track of the maximum accelerations for each state
	float _accels[NumStates];

	// The velocity of this soldier
	Vector2 _velocity;

	// A floating point representation of the position
	Vector2 _position;

	// The current state of this soldier
	State _currentState;

	// If we have a destination, then this is the path we are
	// following
	Path *_currentPath;

	// The current heading of this soldier
	Direction _currentHeading;

	// The animation states of this object
	Animation *_animations[NumStates];

	// The current squad that this soldier belongs to
	Squad *_currentSquad;

	// The personal name of this soldier
	char _personalName[32];

	// The weapons this soldier carries
	Weapon *_weapons[MAX_WEAPONS_PER_SOLDIER];
	int _weaponsNumClips[MAX_WEAPONS_PER_SOLDIER];
	int _currentWeaponIdx;
	int _numWeapons;

	// The title of the soldier
	char _title[32];

	// The rank of this soldier
	char _rank[32];

	// The camouflage scheme of this soldier
	char _camo[64];
	int _camoIdx;

	// The current frame of the animation in the current state
	// This is used to help find out when an animation is complete.
	int _currentFrameCurrentState;

	// A marker to keep track of whether or not we have looped around
	// or are still at the start of an animation
	bool _currentAnimationMarker;

	// A flag which says whether or not I am in a vehicle
	bool _inVehicle;

	friend class SoldierManager;
};
