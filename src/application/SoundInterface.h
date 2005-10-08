#pragma once

/**
 * This class is one way to access the underlying sound implementations
 * of the main app.
 */
class SoundInterface
{
public:
	// Gets the sound implementation
	virtual void *GetImplementation() = 0;
};