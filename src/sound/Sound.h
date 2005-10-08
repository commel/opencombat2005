#pragma once

#include <misc\Structs.h>
class CSound;

class Sound
{
public:
	Sound(char *name, char *soundFile);
	virtual ~Sound(void);

	// Gets the name of this sound
	inline char *GetName() { return _name; }

	// Plays this sound
	void Play();

protected:
	// The name of this sound
	char _name[MAX_NAME];
	// The file of this sound
	char _soundFileName[MAX_NAME];

	// The underlying implementation of this sound
	CSound *_sound;

	friend class SoundManager;
};
