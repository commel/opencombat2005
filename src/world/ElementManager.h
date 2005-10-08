#pragma once

#include <Misc\Array.h>

class TGA;
class Element;

class ElementManager
{
public:
	ElementManager(void);
	virtual ~ElementManager(void);

	// Loads a group of widgets into this widget manager
	void LoadElements(char *fileName);

	// Retrieves a widget by index
	Element *GetElement(int index);

protected:
	// The array of widgets we are managing
	Array<Element> _elements;
};
