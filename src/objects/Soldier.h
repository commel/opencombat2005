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

/**
 * Attribute rating for a soldier. Attributes are rated on a 0-100
 * scale, with 50 being an average rating for an attribute.
 */
class Rating
{
public:
	Rating() { _value = 50; }

private:
	unsigned char _value;
};

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

	// Get's the general heading of this soldier
	virtual void GetGeneralHeading(Vector2 *heading);

	// Sets the formation index for this soldier
	void SetFormationPosition(int index) { _formationPosition = index; }

	// Tells this soldier to follow a given path
	void FollowPath(Path *path, SoldierAction::Action movementStyle);

	// Tells this soldier to follow the given object
	void Follow(Object *object, Formation::Type formationType, float formationSpread, int formationIdx, SoldierAction::Action movementStyle);

	// Tells this soldier to ambush facing a direction
	void Ambush(Direction heading);

	// Tells this soldier to defend a direction
	void Defend(Direction heading);

	// Waiting actions
	void Wait();
	void Wait(long time);

	// Is this soldier stopped?
	virtual bool IsStopped();

	// The physical attributes of this soldier
	struct
	{
		/**
		 * Aggressiveness determines how hard this soldier presses an attack.
		 * An aggressive commander will assault the enemy at the expense of
		 * losing men, or will defend by taking the attack to the enemy. An
		 * aggressive soldier does not like sitting in will place, and
		 * will grow unhappy if forced to defend or stay in one place. Conversely,
		 * an unaggressive soldier prefers defending and ambushing when left alone
		 * and an unaggressive commander prefers holding his ground.
		 */
		Rating Aggressiveness;

		/**
		 * Leadership is a measure of a commander's sphere of influence. A high
		 * leadership mean he can command unit's farther away, and that his
		 * orders will be followed an obeyed. Leadership is a rating that
		 * increases with experience.
		 */
		Rating Leadership;
		
		/**
		 * Charisma is a measure of how well liked this soldier/commander is. A
		 * commander with a high charisma can get his soldiers to give that little 
		 * extra effort, to fight just that much harder. Likewise, a soldier with
		 * a low charisma can affect unit morale, or a commander with a low
		 * charisma is at risk of having his soldiers mutiny, or worse, frag him.
		 */
		Rating Charisma;
		
		/**
		 * Knowledge is a measure of the tactical skill of a commander. A commander
		 * with a high knowledge rating will use a variety of different elements
		 * in planning an attack (terrain, cover, force disposition, etc). Often
		 * this shows up as more finesse in an attack and a desire to minimize
		 * casualties while still achieving the objectives. A less knowledgeable
		 * commander has a tendency to use simplistic tactics and brute force,
		 * prefering direct assaults irregardless of the casualties sustained.
		 * Knowledge is an attribute that can increase with experience.
		 */
		Rating Knowledge;

		/**
		 * Experience is a measure of how much battle time this soldier
		 * has seen. Experienced commanders have the benefit of past precedent
		 * when determining strategies and executing orders. They know what works
		 * and what to avoid. Experienced soldiers are less prone to the stress
		 * of battle. They fire more accurately, follow orders more closely, and
		 * are less likely to break or cower. Experienced soldiers, however, are
		 * less likely to follow incompetent commanders.
		 */
		Rating Experience;
		
		/**
		 * Intelligence is a measure of how well this soldier learns, and the
		 * rate at which this soldier increases his other stats. Intelligence
		 * is a good measure of the potential of a soldier.
		 */
		Rating Intelligence;

		/**
		 * Morale is a measure of the overall mental health of a soldier.
		 * Soldiers with low morale are less likely to follow orders, more likely
		 * to break or cower, and are less effective as a fighting force.
		 */
		Rating Morale;

		/**
		 * Stamina is a measure of the endurance of a soldier. Soldiers with a
		 * high stamina can run farther with higher loads and still fight effectively.
		 */
		Rating Stamina;
	} Attributes;

protected:

	enum AnimationState {
		Standing=0, Prone, Walking, Sneaking, Running, StandingFiring, ProneFiring, StandingReloading, ProneReloading, DyingBlownUp, DyingBackward, DyingForward, Dead, StandingUp, LyingDown, NumStates
	};

	// These functions handle various orders
	bool HandleDestinationOrder(MoveOrder *order);
	bool HandleStopOrder(StopOrder *order);
	bool HandleFireOrder(FireOrder *order);
	
	// Let's shoot at a target using the given weapon
	void Shoot(Weapon *weapon, Object *target, Target::Type targetType, int targetX, int targetY);

	// Let's calculate a shot fired at us. Returns true if we die as a result of this
	// shot
	bool CalculateShot(Soldier *shooter, Weapon *weapon);

	// Finds a target within a squad
	Soldier *FindTarget(Squad *squad);

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

	// The current formation position for this soldier
	int _formationPosition;

	friend class SoldierManager;
	friend class SoldierActionHandlers;
};
