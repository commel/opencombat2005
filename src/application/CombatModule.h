#pragma once
#include <application\module.h>

class TGA;
class World;
class AnimationManager;
class SoldierManager;
class MiniMap;
class SoundManager;
class WidgetManager;
class ColorModifierManager;
class Mark;

class CombatModule :
	public Module
{
public:
	CombatModule();
	virtual ~CombatModule(void);

	// Implementations of the Module interface
	virtual void Initialize(void *app);
	virtual void Simulate(long dt);
	virtual void Render(Screen *screen);
	virtual void LeftMouseDown(int x, int y);
	virtual void LeftMouseUp(int x, int y);
	virtual void LeftMouseDrag(int x, int y);
	virtual void RightMouseDown(int x, int y);
	virtual void RightMouseUp(int x, int y);
	virtual void RightMouseDrag(int x, int y);
	virtual void KeyUp(int key);

protected:
	// Graphical elements of the Combat UI
	WidgetManager *_uiManager;
	WidgetManager *_iconManager;
	WidgetManager *_weaponIconManager;
	WidgetManager *_terrainManager;

	TGA *_longBottomBackground;
	TGA *_unitBackground;
	TGA *_teamBarBlank;

	TGA *_airstrikeNeg;
	TGA *_artilleryNeg;
	TGA *_bombardNeg;

	TGA *_activeTeamPanel;

	// The current world for this module
	World *_currentWorld;

	// The minimap for this module, which controls the location of the map
	MiniMap *_currentMiniMap;

	// Manages the resources for all of the animations in this module
	AnimationManager *_animationManager;

	// Manages the resourcs for all the soldiers in this module
	SoldierManager *_soldierManager;

	// Manages the sound resouces in this module
	SoundManager *_soundManager;

	// Manages the sound effects in this module
	SoundManager *_soundEffectsManager;

	// Loads the soldier color modifiers
	ColorModifierManager *_colorModifierManager;

	// The underlying application implementation
	void *_app;

	// Variable to show the team panel
	bool _showTeamPanel;

	// Variable to show the minimap
	bool _showMiniMap;

	// Variable to show the unit panel
	bool _showUnitPanel;

	// Stores the various marks used in the ranger, etc
	Mark *_marks;
};
