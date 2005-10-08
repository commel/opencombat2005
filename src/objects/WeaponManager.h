#pragma once

#include <Misc\Array.h>

class Weapon;
struct WeaponTemplate;

class WeaponManager
{
public:
	WeaponManager(void);
	virtual ~WeaponManager(void);

	// Loads a group of widgets into this widget manager
	void LoadWeapons(char *fileName);

	// Retrieves a widget from this manager
	Weapon *GetWeapon(char *widgetName);

protected:
	// The array of widgets we are managing
	Array<WeaponTemplate> _weapons;
};
