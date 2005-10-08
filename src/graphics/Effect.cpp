#include ".\effect.h"

#include <graphics\Screen.h>
#include <misc\TGA.h>
#include <sound\Sound.h>
#include <application\Globals.h>

Effect::Effect(char *name)
{
	strcpy(_name, name);
	_frameHoldTime = 0;
	_completed = false;
	_currentFrameNumber = 0;
	_totalTime = 0;
	_incrementalTime = 0;
	Position.x = 0;
	Position.y = 0;
	_sound[0] = '\0';
	_bPlaceOnTurret = false;
}

Effect::~Effect(void)
{
}

// Adds a frame to this effect
void 
Effect::AddFrame(TGA *tga, long frameHoldTime)
{
	_frameHoldTime = frameHoldTime;
	_frames.Add(tga);
}

// Clone's this effect
Effect *
Effect::Clone()
{
	Effect *e = new Effect(_name);
	strcpy(e->_sound, _sound);
	e->_dynamic = _dynamic;
	e->_bPlaceOnTurret = _bPlaceOnTurret;

	for(int i = 0; i < _frames.Count; ++i) {
		e->AddFrame(_frames.Items[i], _frameHoldTime);
	}
	return e;
}

void
Effect::Simulate(long dt)
{
	if(_totalTime == 0 && _sound[0] != '\0') {
		g_Globals->World.SoundEffects->GetSound(_sound)->Play();	
	}

	_totalTime += dt;
	_incrementalTime += dt;
	if(_incrementalTime > _frameHoldTime) {
		_incrementalTime = 0;
		_currentFrameNumber++;

		if(_currentFrameNumber >= _frames.Count) {
			_completed = true;
		}
	}
}

void
Effect::Render(Screen *screen)
{
	if(_completed) {
		return;
	}

	TGA *tga = _frames.Items[_currentFrameNumber];
	screen->Blit(tga->GetData(),
		Position.x-tga->GetOriginX()-screen->Origin.x, 
		Position.y - tga->GetOriginY()-screen->Origin.y,
		tga->GetWidth(), tga->GetHeight(), 0, 0, tga->GetWidth(), tga->GetHeight(), tga->GetDepth(), true);
}

void
Effect::SetSound(char *name)
{
	if(strlen(name) > 0) {
		strcpy(_sound, name);
	} else {
		_sound[0] = '\0';
	}
}