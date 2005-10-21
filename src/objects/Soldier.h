#pragma once
#include <objects\Object.h>
#include <graphics\Animation.h>
#include <orders\Orders.h>
#include <objects\Target.h>
#include <ai\Path.h>
#include <states\State.h>
#include <states\Action.h>
#include <states\ObjectActions.h>
#include <objects\SoldierActionHandlers.h>

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

	// Returns whether or not this soldier is dead
	virtual bool IsDead();

	// Inserts a weapon into the weapon pool
	void InsertWeapon(Weapon *weapon, int numClips);

	bool IsInVehicle() { return _inVehicle; }
	void SetInVechicle(bool v) { _inVehicle = v; }

protected:

	enum AnimationState {
		Standing=0, Prone, Walking, Sneaking, Running, StandingFiring, ProneFiring, StandingReloading, ProneReloading, DyingBlownUp, DyingBackward, DyingForward, Dead, StandingUp, LyingDown, NumStates
	};

	// These functions handle various orders
	bool HandleMoveOrder(long dt, MoveOrder *order);
	bool HandleMoveFastOrder(long dt, MoveOrder *order);
	bool HandleSneakOrder(long dt, MoveOrder *order);
	bool HandleDestinationOrder(MoveOrder *order);
	bool HandleStopOrder(StopOrder *order);
	bool HandleFireOrder(FireOrder *order);

	// This function handles the movement logic
	void PlanMovement(long dt);
	
	// Finds my next move to this next point in my path.
	bool FindPath(Path *path);

	// This function outputs the position and velocity of the
	// input vectors. It is used to find out where we would end up
	// if we moved.
	void Move(Vector2 *posOut, Vector2 *velOut, Direction heading, long dt);

	// Let's shoot at a target using the given weapon
	void Shoot(Weapon *weapon, Object *target, Target::Type targetType, int targetX, int targetY);

	// Let's calculate a shot fired at us. Returns true if we die as a result of this
	// shot
	bool CalculateShot(Soldier *shooter, Weapon *weapon);

	// Finds a target within a squad
	Soldier *FindTarget(Squad *squad);

	// Keeps track of the maximum speeds for each state
	float _maxRunningSpeed;
	float _maxWalkingSpeed;
	float _maxWalkingSlowSpeed;
	float _maxCrawlingSpeed;

	// Keeps track of the maximum accelerations for each state
	float _runningAccel;
	float _walkingAccel;
	float _walkingSlowAccel;
	float _crawlingAccel;

	// The velocity of this soldier
	Vector2 _velocity;

	// A floating point representation of the position
	Vector2 _position;

	// The current state of this soldier
	State _currentState;

	// And the animation state that corresponds to our physical state
	AnimationState _currentAnimationState;

	// If we have a destination, then this is the path we are
	// following
	Path *_currentPath;

	// The current heading of this soldier
	Direction _currentHeading;

	// The animation states of this object
	Animation *_animations[NumStates];

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

	// The current action of this guy. Used only for reporting.
	Unit::Action _currentAction;

	// The current statuc of this guy. Used only for reporting.
	Unit::Status _currentStatus;

	// This is our array of functions that implement the possible
	// actions that a soldier can take
	SoldierActionHandlers::SoldierActionHandler _actionHandlers[SoldierAction::NumActions];

	friend class SoldierManager;
	friend class SoldierActionHandlers;
};
