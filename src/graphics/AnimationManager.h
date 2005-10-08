#pragma once

#include <graphics\Animation.h>

/**
 * This class loads and manages a set of animations.
 */
class AnimationManager
{
public:
	AnimationManager(void);
	virtual ~AnimationManager(void);

	// Loads a group of animations from a configuration file
	virtual void LoadAnimations(char *fileName);

	// Retrieves a copy of an animation for use by an object
	virtual Animation *GetAnimation(char *animationName);

protected:
	// The list of animations that we are managing
	Array<Animation> _animations;

	// The array of source image files
	Array<TGA> _sourceImages;
};
