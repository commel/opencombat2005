#include ".\sound.h"
#include <directx\DSUtil.h>

Sound::Sound(char *name, char *soundFile)
{
	strcpy(_name, name);
	strcpy(_soundFileName, soundFile);
	_sound = NULL;
}

Sound::~Sound(void)
{
}

void
Sound::Play()
{
	_sound->Play();
}
