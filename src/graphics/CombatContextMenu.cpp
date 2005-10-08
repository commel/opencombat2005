#include ".\combatcontextmenu.h"
#include <assert.h>
#include <graphics\Screen.h>

CombatContextMenu::CombatContextMenu(void)
{
	_bShow = false;
	_widgetManager = new WidgetManager();
	_canFire = false;
	_canMove = false;
	_canMoveFast = false;
	_canSneak = false;
	_canDefend = false;
	_canAmbush = false;
	_canSmoke = false;

}

CombatContextMenu::~CombatContextMenu(void)
{
}

void
CombatContextMenu::Initialize(char *widgetsFile)
{
	_widgetManager->LoadWidgets(widgetsFile);

	// Set the individual widgets
	assert((_bg = _widgetManager->GetWidget("Order Background")) != NULL);

	assert((_moveLight = _widgetManager->GetWidget("Order Move Light")) != NULL);
	assert((_moveDark = _widgetManager->GetWidget("Order Move Dark")) != NULL);
	assert((_moveNeg = _widgetManager->GetWidget("Order Move Neg")) != NULL);

	assert((_moveFastLight = _widgetManager->GetWidget("Order Move Fast Light")) != NULL);
	assert((_moveFastDark = _widgetManager->GetWidget("Order Move Fast Dark")) != NULL);
	assert((_moveFastNeg = _widgetManager->GetWidget("Order Move Fast Neg")) != NULL);

	assert((_ambushLight = _widgetManager->GetWidget("Order Ambush Light")) != NULL);
	assert((_ambushDark = _widgetManager->GetWidget("Order Ambush Dark")) != NULL);
	assert((_ambushNeg = _widgetManager->GetWidget("Order Ambush Neg")) != NULL);

	assert((_defendLight = _widgetManager->GetWidget("Order Defend Light")) != NULL);
	assert((_defendDark = _widgetManager->GetWidget("Order Defend Dark")) != NULL);
	assert((_defendNeg = _widgetManager->GetWidget("Order Defend Neg")) != NULL);

	assert((_smokeLight = _widgetManager->GetWidget("Order Smoke Light")) != NULL);
	assert((_smokeDark = _widgetManager->GetWidget("Order Smoke Dark")) != NULL);
	assert((_smokeNeg = _widgetManager->GetWidget("Order Smoke Neg")) != NULL);

	assert((_sneakLight = _widgetManager->GetWidget("Order Sneak Light")) != NULL);
	assert((_sneakDark = _widgetManager->GetWidget("Order Sneak Dark")) != NULL);
	assert((_sneakNeg = _widgetManager->GetWidget("Order Sneak Neg")) != NULL);

	assert((_fireLight = _widgetManager->GetWidget("Order Fire Light")) != NULL);
	assert((_fireDark = _widgetManager->GetWidget("Order Fire Dark")) != NULL);
	assert((_fireNeg = _widgetManager->GetWidget("Order Fire Neg")) != NULL);

	_height = _moveDark->GetHeight();
	_width = _moveDark->GetWidth();

}

void
CombatContextMenu::Render(Screen *screen, int cursorX, int cursorY, Rect *clip)
{
	if((_y+_bg->GetHeight()) > (clip->y + clip->h)) {
		_y = clip->y + clip->h - _bg->GetHeight();
	}
	if((_x+_bg->GetWidth()) > (clip->x + clip->w)) {
		_x = clip->x + clip->w - _bg->GetWidth();
	}

	int x = _x;
	int y = _y;

	_bg->Render(screen, x, y, clip);

	if(_canMove) {
		if(cursorY >= (y+2) && cursorY < (y+2+_height) && cursorX >= (x+2) && cursorX < (x+2+_width)) {
			_moveLight->Render(screen, x+2, y+2, clip);
		} else {
			_moveDark->Render(screen, x+2, y+2, clip);
		}
	} else {
		_moveNeg->Render(screen, x+2, y+2, clip);
	}

	if(_canMoveFast) {
		if(cursorY >= (y+2+_height*1) && cursorY < (y+2+_height+_height*1) && cursorX >= (x+2) && cursorX < (x+2+_width)) {
			_moveFastLight->Render(screen, x+2, y+2+_height*1, clip);
		} else {
			_moveFastDark->Render(screen, x+2, y+2+_height*1, clip);
		}
	} else {
		_moveFastNeg->Render(screen, x+2, y+2+_height*1, clip);
	}

	if(_canFire) {
		if(cursorY >= (y+2+_height*2) && cursorY < (y+2+_height+_height*2) && cursorX >= (x+2) && cursorX < (x+2+_width)) {
			_fireLight->Render(screen, x+2, y+2+_height*2, clip);
		} else {
			_fireDark->Render(screen, x+2, y+2+_height*2, clip);
		}
	} else {
		_fireNeg->Render(screen, x+2, y+2+_height*2, clip);
	}

	if(_canSneak) {
		if(cursorY >= (y+2+_height*3) && cursorY < (y+2+_height+_height*3) && cursorX >= (x+2) && cursorX < (x+2+_width)) {
			_sneakLight->Render(screen, x+2, y+2+_height*3, clip);
		} else {
			_sneakDark->Render(screen, x+2, y+2+_height*3, clip);
		}
	} else {
		_sneakNeg->Render(screen, x+2, y+2+_height*3, clip);
	}

	if(_canSmoke) {
		if(cursorY >= (y+2+_height*4) && cursorY < (y+2+_height+_height*4) && cursorX >= (x+2) && cursorX < (x+2+_width)) {
			_smokeLight->Render(screen, x+2, y+2+_height*4, clip);
		} else {
			_smokeDark->Render(screen, x+2, y+2+_height*4, clip);
		}
	} else {
		_smokeNeg->Render(screen, x+2, y+2+_height*4, clip);
	}

	if(_canDefend) {
		if(cursorY >= (y+2+_height*5) && cursorY < (y+2+_height+_height*5) && cursorX >= (x+2) && cursorX < (x+2+_width)) {
			_defendLight->Render(screen, x+2, y+2+_height*5, clip);
		} else {
			_defendDark->Render(screen, x+2, y+2+_height*5, clip);
		}
	} else {
		_defendNeg->Render(screen, x+2, y+2+_height*5, clip);
	}

	if(_canAmbush) {
		if(cursorY >= (y+2+_height*6) && cursorY < (y+2+_height+_height*6) && cursorX >= (x+2) && cursorX < (x+2+_width)) {
			_ambushLight->Render(screen, x+2, y+2+_height*6, clip);
		} else {
			_ambushDark->Render(screen, x+2, y+2+_height*6, clip);
		}
	} else {
		_ambushNeg->Render(screen, x+2, y+2+_height*6, clip);
	}
}

CombatContextMenu::ContextMenuChoice
CombatContextMenu::Choose(int x, int y)
{
	if(y >= (_y+2+_height*0) && y < (_y+2+_height+_height*0) && x >= (_x+2) && x < (_x+2+_width)) {
		if(_canMove) {
			return Move;
		}
	}

	if(y >= (_y+2+_height*1) && y < (_y+2+_height+_height*1) && x >= (_x+2) && x < (_x+2+_width)) {
		if(_canMoveFast) {
			return MoveFast;
		}
	}

	if(y >= (_y+2+_height*2) && y < (_y+2+_height+_height*2) && x >= (_x+2) && x < (_x+2+_width)) {
		if(_canFire) {
			return Fire;
		}
	}

	if(y >= (_y+2+_height*3) && y < (_y+2+_height+_height*3) && x >= (_x+2) && x < (_x+2+_width)) {
		if(_canSneak) {
			return Sneak;
		}
	}
	
	if(y >= (_y+2+_height*4) && y < (_y+2+_height+_height*4) && x >= (_x+2) && x < (_x+2+_width)) {
		if(_canSmoke) {
			return Smoke;
		}
	}

	if(y >= (_y+2+_height*5) && y < (_y+2+_height+_height*5) && x >= (_x+2) && x < (_x+2+_width)) {
		if(_canDefend) {
			return Defend;
		}
	}

	if(y >= (_y+2+_height*6) && y < (_y+2+_height+_height*6) && x >= (_x+2) && x < (_x+2+_width)) {
		if(_canAmbush) {
			return Ambush;
		}
	}

	return None;
}