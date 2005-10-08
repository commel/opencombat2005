#pragma once

class Screen;
class Widget;
class WidgetManager;

// This class describes the marks used for the ranger, firing locations, etc
class Mark
{
public:
	Mark(void);
	virtual ~Mark(void);

	enum Color {
		Blue=0,
		Purple,
		Red,
		Yellow,
		Orange,
		Green,
		Brown,
		NumColors // Must be last
	};

	// Loads the marks
	void LoadMarks(WidgetManager *manager);

	// Renders a mark
	void Render(Screen *screen, Mark::Color color, int x, int y);

private:
	Widget *_marks[Mark::NumColors];
};
