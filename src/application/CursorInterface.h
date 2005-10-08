#pragma once

class CursorInterface
{
public:
	enum CursorType {
		MarkBlue=0,
		MarkPurple,
		MarkRed,
		MarkYellow,
		MarkOrange,
		MarkBrown,
		MarkGreen,
		Regular,
		NumCursorTypes
	};

	virtual void ShowCursor(bool bShow, CursorType type) = 0;
};
