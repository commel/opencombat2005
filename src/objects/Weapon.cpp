#include ".\weapon.h"
#include <stdio.h>

#include <Graphics\Animation.h>
#include <Sound\Sound.h>
#include <Application\Globals.h>

Weapon::Weapon(void)
{
	_numRounds = 0;
	_totalRounds = 0;
	_counter = 0;
	_state = Reloaded;
	_roundsPerBurst = 0;
	_timeToFire = 0;
	_bGroundShaker = false;
}

Weapon::~Weapon(void)
{
}

// Checks to see if we can fire this weapon
bool 
Weapon::CanFire()
{
	return !IsEmpty() && !IsReloading() && (_state == Reloaded || _counter >= (_reloadTimeChamber+_timeToFire));
}

// Checks to see if the clip is empty
bool 
Weapon::IsEmpty()
{
	return _numRounds <= 0;
}

// Fires this weapon
void 
Weapon::Fire()
{
	if(CanFire()) {
		_state = Firing;
		_counter = 0;
		_numRounds -= _roundsPerBurst;
		g_Globals->World.SoundEffects->GetSound(_sound)->Play();	
	}
}

// Reloads this weapon
void 
Weapon::Reload()
{
	_state = Reloading;
	_counter = 0;
	_numRounds = _totalRounds;
}

// Checks to see if we are reloading the weapon
bool
Weapon::IsReloading()
{
	return _state == Reloading;
}

// Simulates this weapon. Updates firing times and stuff like that
void 
Weapon::Simulate(long dt)
{
	_counter += dt;
	if(_state == Reloading) {
		if(_counter >= _reloadTimeClip) {
			_state = Reloaded;
			_counter = 0;
		}
	}
}

void
Weapon::SetEffect(char *effectName)
{
	sprintf(_effects[North], "%s North", effectName);
	sprintf(_effects[NorthEast], "%s NorthEast", effectName);
	sprintf(_effects[East], "%s East", effectName);
	sprintf(_effects[SouthEast], "%s SouthEast", effectName);
	sprintf(_effects[South], "%s South", effectName);
	sprintf(_effects[SouthWest], "%s SouthWest", effectName);
	sprintf(_effects[West], "%s West", effectName);
	sprintf(_effects[NorthWest], "%s NorthWest", effectName);
}
