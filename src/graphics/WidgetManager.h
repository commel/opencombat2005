#pragma once

#include <Misc\Array.h>

class TGA;
class Widget;

class WidgetManager
{
public:
	WidgetManager(void);
	virtual ~WidgetManager(void);

	// Loads a group of widgets into this widget manager
	void LoadWidgets(char *fileName);

	// Retrieves a widget from this manager
	Widget *GetWidget(char *widgetName);

	// Retrieves a widget by index
	Widget *GetWidget(int index);
	Widget *GetWidget(int index, bool clone);

protected:
	// The array of widgets we are managing
	Array<Widget> _widgets;

	// The array of source images for these widgets
	Array<TGA> _sourceImages;
};
