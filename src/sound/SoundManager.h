#pragma once

#include <Misc\Array.h>

class CSoundManager;
class Sound;

class SoundManager
{
public:
	SoundManager(void *soundManager);
	virtual ~SoundManager(void);

	// Loads a group of widgets into this widget manager
	virtual void LoadSounds(char *fileName);

	// Retrieves a widget from this manager
	virtual Sound *GetSound(char *soundName);

protected:
	// The array of widgets we are managing
	Array<Sound> _sounds;

	// The direct show sound manager
	CSoundManager *_soundManager;
};
