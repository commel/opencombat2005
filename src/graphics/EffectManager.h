#pragma once

#include <Misc\Array.h>

class Effect;
class TGA;

class EffectManager
{
public:
	EffectManager(void);
	virtual ~EffectManager(void);

	void LoadEffects(char *fileName);
	Effect *GetEffect(char *effectName);

protected:
	// The array of effects
	Array<Effect> _effects;

	// The array of source images for these widgets
	Array<TGA> _sourceImages;
};
