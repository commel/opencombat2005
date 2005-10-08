#include ".\mark.h"
#include <misc\Color.h>
#include <graphics\Screen.h>
#include <graphics\Widget.h>
#include <graphics\WidgetManager.h>

Mark::Mark(void)
{
}

Mark::~Mark(void)
{
}

// Loads the marks
void 
Mark::LoadMarks(WidgetManager *manager)
{
	_marks[Blue] = manager->GetWidget("Mark Blue");
	_marks[Purple] = manager->GetWidget("Mark Purple");
	_marks[Orange] = manager->GetWidget("Mark Orange");
	_marks[Yellow] = manager->GetWidget("Mark Yellow");
	_marks[Red] = manager->GetWidget("Mark Red");
	_marks[Brown] = manager->GetWidget("Mark Brown");
	_marks[Green] = manager->GetWidget("Mark Green");
}

// Renders a mark
void 
Mark::Render(Screen *screen, Mark::Color color, int x, int y)
{
	::Color white(255,255,255);
	_marks[color]->Render(screen, x, y, &white);
}
