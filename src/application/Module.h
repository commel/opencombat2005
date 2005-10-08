#pragma once

class Screen;

class Module {
public:

	// Initializes this module
	virtual void Initialize(void *app) = 0;

	// Run the simulation for dt milliseconds. This is how we update the
	// state of the module
	virtual void Simulate(long dt) = 0;

	// Render this module to the screen
	virtual void Render(Screen *screen) = 0;
	
	// Mouse events
	virtual void LeftMouseDown(int x, int y) = 0;
	virtual void LeftMouseUp(int x, int y) = 0;
	virtual void LeftMouseDrag(int x, int y) = 0;
	virtual void RightMouseDown(int x, int y) = 0;
	virtual void RightMouseUp(int x, int y) = 0;
	virtual void RightMouseDrag(int x, int y) = 0;

	// Keyboard events
	virtual void KeyUp(int key) = 0;
};