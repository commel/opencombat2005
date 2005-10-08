#include ".\gameapplication.h"
#include <application\Modules.h>

GameApplication::GameApplication()
{
}

GameApplication::~GameApplication(void)
{
}

void 
GameApplication::Initialize(void *app)
{
	_app = app;

	// Set the current module
	_currentModule = AvailableModules::Combat;

	// Create all of the modules
	_modules[AvailableModules::Combat] = new CombatModule();
}

void
GameApplication::ChooseModule(AvailableModules module)
{
	_currentModule = module;
	_modules[_currentModule]->Initialize(_app);
}

void 
GameApplication::Simulate(long dt)
{
	_modules[_currentModule]->Simulate(dt);
}

void 
GameApplication::Render(Screen *screen)
{
	_modules[_currentModule]->Render(screen);
}

void 
GameApplication::LeftMouseDown(int x, int y)
{
	_modules[_currentModule]->LeftMouseDown(x, y);
}

void 
GameApplication::LeftMouseUp(int x, int y)
{
	_modules[_currentModule]->LeftMouseUp(x, y);
}

void 
GameApplication::LeftMouseDrag(int x, int y)
{
	_modules[_currentModule]->LeftMouseDrag(x,y);
}

void 
GameApplication::RightMouseDown(int x, int y)
{
	_modules[_currentModule]->RightMouseDown(x,y);
}

void 
GameApplication::RightMouseUp(int x, int y)
{
	_modules[_currentModule]->RightMouseUp(x,y);
}

void 
GameApplication::RightMouseDrag(int x, int y)
{
	_modules[_currentModule]->RightMouseDrag(x,y);
}

void 
GameApplication::KeyUp(int key)
{
	_modules[_currentModule]->KeyUp(key);
}
