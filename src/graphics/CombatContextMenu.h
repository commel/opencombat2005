#pragma once

#include <Misc\Structs.h>
#include <Graphics\WidgetManager.h>
#include <Graphics\Widget.h>

class Screen;

class CombatContextMenu
{
public:
	CombatContextMenu(void);
	virtual ~CombatContextMenu(void);

	// Choices available for this context menu
	enum ContextMenuChoice {
		Move, MoveFast, Fire, Ambush, Smoke, Sneak, Defend, None
	};

	// Renders this context menu
	virtual void Render(Screen *screen, int cursorX, int cursorY, Rect *clip);

	// Makes a choice on this context menu
	virtual ContextMenuChoice Choose(int x, int y);

	// Show this context menu
	inline void Show(int x, int y) { _bShow = true; _x = x; _y = y;}

	// Hides this context menu
	inline void Hide() { _bShow = false; }

	// Queries whether or not we are showing the context menu
	inline bool IsShowing() { return _bShow; }

	// Initializes this context menu
	void Initialize(char *widgetsFile);

	// Set's the state of the context menu choices
	inline void SetAmbush(bool b) { _canAmbush = b; }
	inline void SetMove(bool b) { _canMove = b; }
	inline void SetMoveFast(bool b) { _canMoveFast = b; }
	inline void SetDefend(bool b) { _canDefend = b; }
	inline void SetFire(bool b) { _canFire = b; }
	inline void SetSmoke(bool b) { _canSmoke = b; }
	inline void SetSneak(bool b) { _canSneak = b; }

protected:
	// Variable for whether or not we are supposed to show this context menu
	bool _bShow;

	// The widget manager for this context menu
	WidgetManager *_widgetManager;

	// The individual widgets for this context menu
	Widget *_bg;
	Widget *_moveLight, *_moveDark, *_moveNeg;
	Widget *_moveFastLight, *_moveFastDark, *_moveFastNeg;
	Widget *_sneakLight, *_sneakDark, *_sneakNeg;
	Widget *_smokeLight, *_smokeDark, *_smokeNeg;
	Widget *_ambushLight, *_ambushDark, *_ambushNeg;
	Widget *_defendLight, *_defendDark, *_defendNeg;
	Widget *_fireLight, *_fireDark, *_fireNeg;

	// The widget and height of a context menu choice
	int _width, _height;

	// The (x,y) position of where the menu is supposed to be shown
	int _x, _y;

	// The context menu choices
	bool _canFire;
	bool _canMove;
	bool _canMoveFast;
	bool _canSneak;
	bool _canDefend;
	bool _canAmbush;
	bool _canSmoke;
};
