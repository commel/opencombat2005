#pragma once

#include <objects\Soldier.h>

class AnimationManager;
class SoldierTemplate;
class WeaponManager;

/**
 * This class loads and manages a set of animations.
 */
class SoldierManager
{
public:
	SoldierManager(void);
	virtual ~SoldierManager(void);

	// Loads a group of soldiers from a configuration file
	void LoadSoldiers(char *fileName, char *soldierNames);

	// Creates an instance of a specific soldier
	Soldier *CreateSoldier(char *soldierType, AnimationManager *animationManager, WeaponManager *weaponManager);

protected:
	// The list of soldier templates that we are managing
	Array<SoldierTemplate> _soldiers;

	// A list of soldier names that we can choose from
	Array<char> _soldierNames;

	// Retrieves an animation from the animation manager by name
	Animation *GetAnimation(AnimationManager *animationManager, char *name, SoldierTemplate *tplate);

};
