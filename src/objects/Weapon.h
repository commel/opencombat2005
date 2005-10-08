#pragma once

#include <Misc\Structs.h>

class Weapon
{
public:
	Weapon(void);
	virtual ~Weapon(void);

	// Checks to see if we can fire this weapon
	bool CanFire();

	// Checks to see if the clip is empty
	bool IsEmpty();

	// Fires this weapon
	void Fire();

	// Reloads this weapon
	void Reload();

	// Checks to see if we are reloading the weapon
	bool IsReloading();

	// Simulates this weapon. Updates firing times and stuff like that
	void Simulate(long dt);

	inline char *GetName() { return _name; }
	inline int GetCurrentRounds() { return _numRounds; }
	inline char *GetIconName() { return _iconName; }
	inline int GetRoundsPerClip() { return _totalRounds; }
	inline int GetRoundsPerBurst() { return _roundsPerBurst; }

	// Set and get effects
	void SetEffect(char *effectName);
	inline char *GetEffect(Direction heading) { return _effects[heading]; }

	// Does this weapon cause a big boom?
	inline bool IsGroundShaker() { return _bGroundShaker; }
	inline void SetGroundShaker(bool b) { _bGroundShaker = b; }

protected:
	friend class WeaponManager;

	enum State {
		Reloading, Firing, Reloaded, NumStates
	};

	// The name of this weapon
	char _name[32];

	// The sound for this weapon
	char _sound[64];

	// The animation for this weapon
	char _animation[64];

	// The time it takes to fire the weapon
	int _timeToFire;

	// The number of rounds in the weapon
	int _numRounds;

	// The number of rounds this weapon can hold
	int _totalRounds;

	// The number of rounds in one burst
	int _roundsPerBurst;

	// The name of the icon for this weapon
	char _iconName[32];

	// The counter for our firing times
	long _counter;

	// Our current firing state
	State _state;

	// The reload time for a chamber
	int _reloadTimeChamber;

	// The reload time for a clip
	int _reloadTimeClip;
	
	// Effects
	char _effects[NumDirections][64];

	// Goes big boom
	bool _bGroundShaker;
};
