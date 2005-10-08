#pragma once
#include <Graphics\AnimationManager.h>
#include <Misc\Array.h>

class SoldierAnimationManager : AnimationManager
{
public:
	SoldierAnimationManager(void);
	virtual ~SoldierAnimationManager(void);

	virtual void LoadAnimations(char *fileName);

};
