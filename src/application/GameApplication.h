#pragma once

#include <application\Module.h>

class GameApplication : public Module
{
public:
	GameApplication();
	virtual ~GameApplication(void);

	// Modules available to the application
	enum AvailableModules {
		Introduction = 0,
		Combat,
		NumAvailableModules // Needs to be last
	};

	// The following are inherited from class Module
	virtual void Initialize(void *app);
	virtual void Simulate(long dt);
	virtual void Render(Screen *screen);
	virtual void LeftMouseDown(int x, int y);
	virtual void LeftMouseUp(int x, int y);
	virtual void LeftMouseDrag(int x, int y);
	virtual void RightMouseDown(int x, int y);
	virtual void RightMouseUp(int x, int y);
	virtual void RightMouseDrag(int x, int y);

	// Keyboard events
	virtual void KeyUp(int key);

	// Selects a new module and initializes it
	virtual void ChooseModule(AvailableModules module);

protected:
	AvailableModules _currentModule;
	Module *_modules[AvailableModules::NumAvailableModules];

	// This is the windows implementation of the wrapper app
	void *_app;
};
